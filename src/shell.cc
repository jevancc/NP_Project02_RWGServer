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

ssize_t Shell::Send(const string& s) const {
  return this->console_.Send2Fd(this->sockfd_, s);
}

void Shell::Execute(string input) {
  // cout << "% ";
  // if (!getline(cin, input)) {
  //   builtin::exit({"exit"}, this->env);
  // }

  ::clearenv();
  for (auto& vp : this->env.GetVariables()) {
    ::setenv(vp.first.c_str(), vp.second.c_str(), 1);
  }

  Command command(input);
  if (command.Parse().empty()) {
    return;
  }
  for (auto task : command.Parse()) {
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

  auto last_task = *command.Parse().rbegin();
  if (last_task.GetStdout().type == IO::kDelayedPipe) {
    this->env.AddDelayedChildProcesses(last_task.GetStdout().i_data,
                                       this->env.GetDelayedChildProcesses());
  }

  for (auto child_pid : this->env.GetDelayedChildProcesses()) {
    int status;
    waitpid(child_pid, &status, 0);
  }

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
