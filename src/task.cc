#include <fcntl.h>
#include <np/shell/builtin.h>
#include <np/shell/shell.h>
#include <np/shell/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <nonstd/optional.hpp>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
string Task::ToString() const {
  string s = "Task {\n\t";
  for (auto& a : this->argv_) {
    s += a + " ";
  }
  s += "\n";
  s += "\tStdin: " + this->stdin_.ToString() + "\n";
  s += "\tStdout: " + this->stdout_.ToString() + "\n";
  s += "\tStderr: " + this->stderr_.ToString() + "\n";
  s += "\n}";
  return s;
}

char** Task::C_Args() const {
  char** args = new char*[this->argv_.size() + 1];
  for (size_t i = 0; i < this->argv_.size(); i++) {
    args[i] = strdup(this->argv_[i].c_str());
  }
  args[this->argv_.size()] = NULL;
  return args;
}

pid_t Task::Execute(Shell& shell) {
  if (this->argv_.empty()) {
    return ExecError::kSuccess;
  }
  ExecError status;
  if ((status = builtin::Execute(this->argv_, shell)) !=
      ExecError::kFileNotFound) {
    return status;
  }

  if (!this->in_pipe_) {
    this->in_pipe_ = shell.env.GetDelayedPipe(0).lock();
    shell.env.SetDelayedPipe(0, Pipe::Create());
  }

  if (this->stdout_.type == IO::kDelayedPipe) {
    shell.env.EnsureDelayedPipe(this->stdout_.i_data);
  }
  if (this->stderr_.type == IO::kDelayedPipe) {
    shell.env.EnsureDelayedPipe(this->stderr_.i_data);
  }

  pid_t pid = fork();
  if (pid < 0) {
    return ExecError::kForkFailed;
  } else if (pid > 0) {
    // parent process
    if (this->in_pipe_) {
      this->in_pipe_->Close();
      this->in_pipe_ = nullptr;
    }
    shell.env.AddDelayedChildProcess(0, pid);
    return ExecError::kSuccess;
  } else {
    dup2(shell.GetSockfd(), STDIN_FILENO);
    dup2(shell.GetSockfd(), STDOUT_FILENO);
    dup2(shell.GetSockfd(), STDERR_FILENO);

    weak_ptr<Pipe> pipe_w;

    // child process
    switch (this->stdin_.type) {
      case IO::kUserPipe:  // NOTE:this->in_pipe_ is ensured in shell::Execute()
      case IO::kInherit:
        if (this->in_pipe_ && this->in_pipe_->IsEnable()) {
          this->in_pipe_->DupIn2(STDIN_FILENO);
        }
        break;
      case IO::kDelayedPipe:
        // unreachable
        throw runtime_error("unreachable");
        break;
      case IO::kFile:  // NOTE: Not in spec
        break;
    }

    switch (this->stdout_.type) {
      case IO::kInherit:
        break;
      case IO::kDelayedPipe:
        pipe_w = shell.env.GetDelayedPipe(this->stdout_.i_data);
        if (auto p = pipe_w.lock()) {
          p->DupOut2(STDOUT_FILENO);
        }
        break;
      case IO::kUserPipe:
        pipe_w = *shell.GetPipe2User_(this->stdout_.i_data);
        if (auto p = pipe_w.lock()) {
          p->DupOut2(STDOUT_FILENO);
        }
        break;
      case IO::kFile:
        int fd = open(this->stdout_.s_data.c_str(),
                      (O_WRONLY | O_CREAT | O_TRUNC), 0644);
        dup2(fd, STDOUT_FILENO);
        break;
    }

    switch (this->stderr_.type) {
      case IO::kInherit:
        break;
      case IO::kDelayedPipe:
        pipe_w = shell.env.GetDelayedPipe(this->stderr_.i_data);
        if (auto p = pipe_w.lock()) {
          p->DupOut2(STDERR_FILENO);
        }
        break;
      case IO::kUserPipe:
        pipe_w = *shell.GetPipe2User_(this->stderr_.i_data);
        if (auto p = pipe_w.lock()) {
          p->DupOut2(STDERR_FILENO);
        }
        break;
      case IO::kFile:  // NOTE: Not in spec
        break;
    }

    if (this->in_pipe_) {
      this->in_pipe_->Close();
    }
    for (auto& user_w : shell.console_.GetUsers_()) {
      if (auto user = user_w.lock()) {
        user->CloseSockfd();
      }
    }

    const char* file = this->argv_[0].c_str();
    char** arg = this->C_Args();
    if (execvp(file, arg) < 0) {
      cerr << "Unknown command: [" << this->argv_[0] << "]." << endl;
      exit(0);
    }
    return ExecError::kSuccess;
  }
}
}  // namespace shell
}  // namespace np
