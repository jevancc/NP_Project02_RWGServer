#include <arpa/inet.h>
#include <easylogging++.h>
#include <netinet/in.h>
#include <np/shell/builtin.h>
#include <np/shell/types.h>
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
    : console_(console), env(uid) {
  inet_ntop(AF_INET, &(client_info->sin_addr), this->addr_, INET_ADDRSTRLEN);
  sprintf(this->port_, "%d", client_info->sin_port);
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
  for (auto pid : this->env.GetDelayedChildProcesses(line)) {
    waitpid(pid, &status, 0);
  }
  return status;
}
int Shell::WaitUserChildProcesses_(int uid) {
  int status;
  for (auto pid : this->env.GetUserChildProcesses(uid)) {
    waitpid(pid, &status, 0);
  }
  return status;
}

void Shell::Execute(string input) {
  ::clearenv();
  for (auto& vp : this->env.GetVariables()) {
    ::setenv(vp.first.c_str(), vp.second.c_str(), 1);
  }

  Command command(input);
  if (command.Tasks().empty()) {
    return;
  }

  auto first_task = command.Tasks().begin();
  auto last_task = command.Tasks().rbegin();
  int uid_from = first_task->GetStdin().i_data;
  int uid_to = last_task->GetStdout().i_data;

  if (first_task->GetStdin().type == IO::kUserPipe) {
    char msg[kMaxMessageSize] = {};
    if (auto p = this->env.GetDelayedPipe(0).lock()) {
      p->Close();
    }
    this->env.SetDelayedPipe(0, Pipe::Create());
    if (!this->console_.IsUserExists(uid_from)) {
      sprintf(msg, "*** Error: user #%d does not exist yet. ***\n", uid_from);
      this->Send(msg);
      this->WaitDelayedChildProcesses_();
      return;
    } else if (auto p = this->env.GetUserPipe(uid_from).lock()) {
      sprintf(msg, "*** %s (#%d) just received from %s (#%d) by '%s' ***\n",
              this->env.GetName().c_str(), this->env.GetUid(),
              this->console_.GetUserName(uid_from)->c_str(), uid_from,
              input.c_str());
      this->console_.Broadcast(msg);
      first_task->SetInPipe(p);
      this->env.CloseUserPipe(uid_from);
    } else {
      sprintf(msg, "*** Error: the pipe #%d->#%d does not exist yet. ***\n",
              uid_from, this->env.GetUid());
      this->Send(msg);
      this->WaitDelayedChildProcesses_();
      return;
    }
  }

  if (last_task->GetStdout().type == IO::kUserPipe ||
      last_task->GetStderr().type == IO::kUserPipe) {
    auto pipe_opt = this->GetPipe2User_(uid_to);
    char msg[kMaxMessageSize] = {};
    if (!pipe_opt) {
      sprintf(msg, "*** Error: user #%d does not exist yet. ***\n", uid_to);
      this->Send(msg);
      this->WaitUserChildProcesses_(uid_from);
      this->WaitDelayedChildProcesses_();
      return;
    } else if (auto p = pipe_opt->lock()) {
      sprintf(msg, "*** Error: the pipe #%d->#%d already exists. ***\n",
              this->env.GetUid(), uid_to);
      this->Send(msg);
      this->WaitUserChildProcesses_(uid_from);
      this->WaitDelayedChildProcesses_();
      return;
    } else {
      sprintf(msg, "*** %s (#%d) just piped '%s' to %s (#%d) ***\n",
              this->env.GetName().c_str(), this->env.GetUid(), input.c_str(),
              this->console_.GetUserName(uid_to)->c_str(), uid_to);
      this->console_.Broadcast(msg);
      this->SetPipe2User_(uid_to, Pipe::Create());
    }
  }

  for (auto task : command.Tasks()) {
    int status;

    while ((status = task.Execute(*this)) == ExecError::kForkFailed) {
      usleep(1000);
    }
    switch (status) {
      case ExecError::kFileNotFound:
        cout << "Unknown command: [" << task.GetFile() << "]." << endl;
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

  this->WaitUserChildProcesses_(first_task->GetStdout().i_data);
  this->WaitDelayedChildProcesses_();
  this->env.GotoNextLine();
}

Shell::~Shell() {
  for (int i = 0; i < kMaxDelayedPipes; i++) {
    for (auto& pid : this->env.GetDelayedChildProcesses(i)) {
      ::kill(pid, SIGKILL);
    }
  }
  for (int i = 0; i < kMaxShellUsers; i++) {
    for (auto& pid : this->env.GetUserChildProcesses(i)) {
      ::kill(pid, SIGKILL);
    }
  }
  ::close(this->sockfd_);
  LOG(INFO) << "User " << this->env.GetUid() << " disconnected";
}

}  // namespace shell
}  // namespace np
