#include <arpa/inet.h>
#include <netinet/in.h>
#include <np/shell/shell.h>
#include <np/shell/shell_console.h>
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

namespace np {
namespace shell {

ShellConsole::ShellConsole(int sockfd, int maxfd)
    : sockfd_(sockfd), maxfd_(maxfd) {
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
    throw runtime_error("failed to create new user: uid or ufd is occupied");
  }
  this->id2user_map_[uid] = shell_ptr;
  this->fd2user_map_[ufd] = shell_ptr;
  return shell_ptr;
}

ssize_t ShellConsole::Send2Uid(int uid, const string& msg) const {
  shared_ptr<Shell> shell;
  if ((shell = this->id2user_map_[uid]) == nullptr) {
    throw runtime_error("failed to send message: invalid uid");
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

void ShellConsole::Run() {
  fd_set readfds;
  char msg_buffer[4096];

  while (true) {
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
          shell->SendWelcomeMessage_();
          this->Broadcast(
              // NOTE: fix <ip>/<port> in login message for demo in class
              "*** User '(no name)' entered from CGILAB/511. ***\n");
          shell->SendPrompt_();
        } else {
          auto shell = this->fd2user_map_[fd];
          memset(msg_buffer, 0, sizeof(msg_buffer));
          recv(fd, msg_buffer, sizeof(msg_buffer), 0);

          string command(msg_buffer);
          shell->Execute(command);
          shell->SendPrompt_();
        }
      }
    }
  }
}

}  // namespace shell
}  // namespace np
