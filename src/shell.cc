#define SPDLOG_FMT_EXTERNAL
#include <arpa/inet.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <np/shell/builtin.h>
#include <np/shell/types.h>
#include <np/utils.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

namespace np {
namespace shell {

Shell::Shell(sockaddr_in* client_info, int sockfd, int uid,
             ShellConsole& console)
    : is_alive_(true), console_(console), env(uid) {
  inet_ntop(AF_INET, &(client_info->sin_addr), this->addr_, INET_ADDRSTRLEN);
  fmt::format_to_n(this->port_, sizeof(this->port_), "{}",
                   client_info->sin_port);
  this->sockfd_ = sockfd;
  builtin::setenv({"setenv", "PATH bin:."}, *this);
  builtin::setenv({"setenv", "LANG en_US.UTF-8"}, *this);
}

optional<weak_ptr<Pipe>> Shell::GetPipe2User_(int uid) {
  return this->console_.GetPipe2User(*this, uid);
}
bool Shell::SetPipe2User_(int uid, shared_ptr<Pipe> p) {
  return this->console_.SetPipe2User(*this, uid, p);
}
bool Shell::MoveChildProcesses2User_(int uid, vector<pid_t>& pids) {
  return this->console_.MoveChildProcesses2User(*this, uid, pids);
}

ssize_t Shell::Send(const string& s) const {
  return this->console_.Send2Fd(this->sockfd_, s);
}

int Shell::WaitDelayedChildProcesses_(int line) {
  int status;
  for (auto& pid : this->env.GetDelayedChildProcesses(line)) {
    waitpid(pid, &status, 0);
  }
  this->env.GetDelayedChildProcesses(line).clear();
  return status;
}

int Shell::WaitUserChildProcesses_(int uid) {
  int status;
  for (auto& pid : this->env.GetUserChildProcesses(uid)) {
    waitpid(pid, &status, 0);
  }
  this->env.GetUserChildProcesses(uid).clear();
  return status;
}

void Shell::Execute(string input) {
  ::clearenv();
  for (auto& vp : this->env.GetVariables()) {
    ::setenv(vp.first.c_str(), vp.second.c_str(), 1);
  }

  input = utils::trim(input);
  Command command(input);
  if (command.Tasks().empty()) {
    return;
  }

  auto first_task = command.Tasks().begin();
  auto last_task = command.Tasks().rbegin();
  if (first_task->GetStdin().type == IO::kUserPipe) {
    int uid_from = first_task->GetStdin().i_data;
    this->env.SetDelayedPipe(0, Pipe::Create());
    if (!this->console_.IsUserExists(uid_from)) {
      this->Send(fmt::format("*** Error: user #{} does not exist yet. ***\n",
                             uid_from));
      this->WaitDelayedChildProcesses_();
      return;
    } else if (auto p = this->env.GetUserPipe(uid_from).lock()) {
      this->console_.Broadcast(
          fmt::format("*** {} (#{}) just received from {} (#{}) by '{}' ***\n",
                      this->env.GetName(), this->env.GetUid(),
                      *this->console_.GetUserName(uid_from), uid_from, input));
      first_task->SetInPipe(p);
      this->env.CloseUserPipe(uid_from);
    } else {
      this->Send(
          fmt::format("*** Error: the pipe #{}->#{} does not exist yet. ***\n",
                      uid_from, this->env.GetUid()));
      this->WaitDelayedChildProcesses_();
      return;
    }
  }

  if (last_task->GetStdout().type == IO::kUserPipe ||
      last_task->GetStderr().type == IO::kUserPipe) {
    int uid_to = last_task->GetStdout().i_data;
    auto pipe_opt = this->GetPipe2User_(uid_to);
    if (!pipe_opt) {
      this->Send(
          fmt::format("*** Error: user #{} does not exist yet. ***\n", uid_to));
      if (first_task->GetStdin().type == IO::kUserPipe) {
        this->WaitUserChildProcesses_(first_task->GetStdin().i_data);
      }
      this->WaitDelayedChildProcesses_();
      return;
    } else if (auto p = pipe_opt->lock()) {
      this->Send(
          fmt::format("*** Error: the pipe #{}->#{} already exists. ***\n",
                      this->env.GetUid(), uid_to));
      if (first_task->GetStdin().type == IO::kUserPipe) {
        this->WaitUserChildProcesses_(first_task->GetStdin().i_data);
      }
      this->WaitDelayedChildProcesses_();
      return;
    } else {
      this->console_.Broadcast(
          fmt::format("*** {} (#{}) just piped '{}' to {} (#{}) ***\n",
                      this->env.GetName(), this->env.GetUid(), input,
                      *this->console_.GetUserName(uid_to), uid_to));
      this->SetPipe2User_(uid_to, Pipe::Create());
    }
  }

  for (auto task : command.Tasks()) {
    int status;

    while ((status = task.Execute(*this)) == ExecError::kForkFailed) {
      usleep(1000);
    }

    usleep(500);
    switch (status) {
      case ExecError::kFileNotFound:
        fmt::print("Unknown command: [{}].\n", task.GetFile());
        break;
      case ExecError::kSuccess:
        break;
      default:
        throw runtime_error("Unexpected error happened");
    }
  }

  if (last_task->GetStdout().type == IO::kDelayedPipe) {
    this->env.Move2DelayedChildProcesses(last_task->GetStdout().i_data,
                                         this->env.GetDelayedChildProcesses());
  } else if (last_task->GetStdout().type == IO::kUserPipe) {
    this->MoveChildProcesses2User_(last_task->GetStdout().i_data,
                                   this->env.GetDelayedChildProcesses());
  }

  if (first_task->GetStdin().type == IO::kUserPipe) {
    this->WaitUserChildProcesses_(first_task->GetStdin().i_data);
  }
  this->WaitDelayedChildProcesses_();
  this->env.GotoNextLine();
}

Shell::~Shell() {
  for (int i = 0; i < kMaxDelayedPipes; i++) {
    for (auto& pid : this->env.GetDelayedChildProcesses(i)) {
      ::kill(pid, SIGKILL);
    }
  }
  for (auto& user : this->env.GetAllUserChildProcesses()) {
    for (auto& pid : user.second) {
      ::kill(pid, SIGKILL);
    }
  }

  ::close(this->sockfd_);
  spdlog::info("User {} disconnected", this->env.GetUid());
}

}  // namespace shell
}  // namespace np
