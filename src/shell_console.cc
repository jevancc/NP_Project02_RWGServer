#define SPDLOG_FMT_EXTERNAL
#include <arpa/inet.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <np/shell/shell.h>
#include <np/shell/shell_console.h>
#include <np/shell/types.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

namespace np {
namespace shell {

ShellConsole::ShellConsole(int sockfd, int maxfd)
    : sockfd_(sockfd), maxfd_(maxfd), id2user_map_({nullptr}) {
  FD_ZERO(&this->sockfds_);
  FD_SET(sockfd, &this->sockfds_);
}

int ShellConsole::CreateUid_() {
  int uid = -1;
  if (this->available_uids_.empty()) {
    uid = this->id2user_map_.size();
    this->id2user_map_.push_back(nullptr);
  } else {
    uid = this->available_uids_.top();
    this->available_uids_.pop();
  }
  return uid;
}

shared_ptr<Shell> ShellConsole::CreateShell_(sockaddr_in* client_info,
                                             int ufd) {
  int uid = this->CreateUid_();
  shared_ptr<Shell> shell_ptr(new Shell(client_info, ufd, uid, *this));
  if (this->id2user_map_[uid] != nullptr ||
      this->fd2user_map_[ufd] != nullptr) {
    static auto msg = "failed to create user: uid or ufd is occupied";
    spdlog::error(msg);
    throw runtime_error(msg);
  }
  this->id2user_map_[uid] = shell_ptr;
  this->fd2user_map_[ufd] = shell_ptr;
  return shell_ptr;
}

void ShellConsole::ClearGarbageShells_() {
  while (!this->garbage_shell_queue_.empty()) {
    auto shell = this->garbage_shell_queue_.front();
    this->garbage_shell_queue_.pop();
    if (shell.use_count() > 3) {
      static auto msg =
          "failed to delete user: shell's shared_ptr is owned by others";
      spdlog::error(msg);
      throw runtime_error(msg);
    } else {
      this->Broadcast(
          fmt::format("*** User '{}' left. ***", shell->env.GetName()));

      for (auto u_w : this->GetUsers_()) {
        if (auto u = u_w.lock()) {
          for (auto& pid : u->env.GetUserChildProcesses(shell->env.GetUid())) {
            ::kill(pid, SIGKILL);
          }
          u->env.GetUserChildProcesses(shell->env.GetUid()).clear();
          u->env.GetUserPipe(shell->env.GetUid()).reset();
        }
      }
      this->id2user_map_[shell->env.GetUid()].reset();
      this->fd2user_map_[shell->sockfd_].reset();
      FD_CLR(shell->sockfd_, &this->sockfds_);
      this->user_names_.erase(shell->env.GetName());
      this->available_uids_.push(shell->env.GetUid());
      if (this->maxfd_ == shell->sockfd_) {
        this->maxfd_--;
      }
    }
  }
}

ssize_t ShellConsole::Send2Uid(int uid, const string& msg) const {
  shared_ptr<Shell> shell;
  if ((shell = this->id2user_map_[uid]) == nullptr) {
    static auto msg = "failed to send message: invalid uid";
    spdlog::error(msg);
    throw runtime_error(msg);
  }
  return send(shell->sockfd_, msg.c_str(), msg.size(), 0);
}

ssize_t ShellConsole::Send2Fd(int fd, const string& msg) const {
  return send(fd, msg.c_str(), msg.size(), 0);
}

ssize_t ShellConsole::Broadcast(const string& msg) const {
  ssize_t ret = 0;
  for (auto& fd_user : this->fd2user_map_) {
    int ufd = fd_user.first;
    ret |= send(ufd, msg.c_str(), msg.size(), 0);
  }
  return ret;
}

ssize_t ShellConsole::SendWelcomeMessage2Fd_(int fd) const {
  static const string welcome_message(
      "****************************************\n"
      "** Welcome to the information server. **\n"
      "****************************************\n");
  return this->Send2Fd(fd, welcome_message);
}

ssize_t ShellConsole::SendPrompt2Fd_(int fd) const {
  static const string prompt("% ");
  return this->Send2Fd(fd, prompt);
}

optional<string> ShellConsole::GetUserName(int uid) const {
  if (this->IsUserExists(uid)) {
    return string(this->id2user_map_[uid]->env.GetName());
  } else {
    return nullopt;
  }
}

void ShellConsole::DeleteUser(int uid, int ufd) {
  if (this->id2user_map_[uid] == this->fd2user_map_[ufd]) {
    auto shell = this->id2user_map_[uid];
    if (shell != nullptr) {
      this->garbage_shell_queue_.push(shell);
    } else {
      static auto msg = "failed to add user to garbage queue: user not exists";
      spdlog::error(msg);
      throw invalid_argument(msg);
    }
  } else {
    static auto msg =
        "failed to add user to garbage queue: uid and ufd not correspond to "
        "same user";
    spdlog::error(msg);
    throw invalid_argument(msg);
  }
}

optional<weak_ptr<Pipe>> ShellConsole::GetPipe2User(Shell& user_from,
                                                    int uid_to) {
  if (this->IsUserExists(uid_to)) {
    return this->id2user_map_[uid_to]->env.GetUserPipe(user_from.env.GetUid());
  } else {
    return nullopt;
  }
}
bool ShellConsole::SetPipe2User(Shell& user_from, int uid_to,
                                shared_ptr<Pipe> p) {
  if (this->IsUserExists(uid_to)) {
    this->id2user_map_[uid_to]->env.SetUserPipe(user_from.env.GetUid(), p);
    return true;
  } else {
    return false;
  }
}
bool ShellConsole::MoveChildProcesses2User(Shell& user_from, int uid_to,
                                           vector<pid_t>& pids) {
  if (this->IsUserExists(uid_to)) {
    this->id2user_map_[uid_to]->env.Move2UserChildProcesses(
        user_from.env.GetUid(), pids);
    return true;
  } else {
    return false;
  }
}

void ShellConsole::Run() {
  fd_set readfds;
  char input_buffer[4096];

  while (true) {
    this->ClearGarbageShells_();

    readfds = this->sockfds_;
    if (select(this->maxfd_ + 1, &readfds, NULL, NULL, NULL) == -1) {
      continue;
    }

    struct sockaddr_in client_info;
    socklen_t addrlen = sizeof(client_info);
    for (int fd = 0; fd <= this->maxfd_; fd++) {
      if (FD_ISSET(fd, &readfds)) {
        if (fd == this->sockfd_) {
          int ufd =
              accept(this->sockfd_, (struct sockaddr*)&client_info, &addrlen);
          auto shell = this->CreateShell_(&client_info, ufd);
          FD_SET(ufd, &this->sockfds_);
          if (ufd > this->maxfd_) {
            this->maxfd_ = ufd;
          }
          this->SendWelcomeMessage2Fd_(ufd);
          this->Broadcast(
              // NOTE: fix <ip>/<port> in login message for demo in class
              "*** User '(no name)' entered from CGILAB/511. ***\n");
          this->SendPrompt2Fd_(ufd);
          spdlog::info("User connected from {}:{} with uid:{}", shell->addr_,
                       shell->port_, shell->env.GetUid());
        } else {
          auto shell = this->fd2user_map_[fd];
          if (shell == nullptr) {
            throw runtime_error("shell with given sockfd no exists");
          } else if (!shell->IsAlive()) {
            continue;
          }

          memset(input_buffer, 0, sizeof(input_buffer));
          recv(fd, input_buffer, sizeof(input_buffer), 0);

          shell->Execute(string(input_buffer));
          if (shell->IsAlive()) {
            this->SendPrompt2Fd_(fd);
          }
        }
      }
    }
  }
}

}  // namespace shell
}  // namespace np
