#include <fcntl.h>
#include <np/shell/builtin.h>
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

pid_t Task::Exec(Environment& env) {
  if (this->argv_.empty()) {
    return ExecError::kSuccess;
  }
  ExecError status;
  if ((status = builtin::Exec(this->argv_, env)) != ExecError::kFileNotFound) {
    return status;
  }

  if (!this->in_pipe_) {
    this->in_pipe_ = env.GetPipe(0);
    env.SetPipe(0, Pipe::Create());
  }
  if (this->stdout_.type == IO::kPipe) {
    env.CreatePipe(this->stdout_.line);
  }
  if (this->stderr_.type == IO::kPipe) {
    env.CreatePipe(this->stderr_.line);
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
    env.AddChildProcess(0, pid);
    return ExecError::kSuccess;
  } else {
    // child process
    switch (this->stdin_.type) {
      case IO::kInherit:
        if (this->in_pipe_ && this->in_pipe_->IsEnable()) {
          this->in_pipe_->DupIn2(STDIN_FILENO);
        }
        break;
      case IO::kPipe:
        // unreachable
        throw runtime_error("unreachable");
        break;
      case IO::kFile:  // NOTE: Not in spec
        break;
    }

    switch (this->stdout_.type) {
      case IO::kInherit:
        break;
      case IO::kPipe:
        env.GetPipe(this->stdout_.line)->DupOut2(STDOUT_FILENO);
        break;
      case IO::kFile:
        int fd = open(this->stdout_.file.c_str(),
                      (O_WRONLY | O_CREAT | O_TRUNC), 0644);
        dup2(fd, STDOUT_FILENO);
        break;
    }

    switch (this->stderr_.type) {
      case IO::kInherit:
        break;
      case IO::kPipe:
        env.GetPipe(this->stderr_.line)->DupOut2(STDERR_FILENO);
        break;
      case IO::kFile:  // NOTE: Not in spec
        break;
    }

    if (this->in_pipe_) {
      this->in_pipe_->Close();
    }
    env.CloseAllPipes();

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
