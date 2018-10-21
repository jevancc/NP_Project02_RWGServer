#include <fcntl.h>
#include <npshell/builtin.h>
#include <npshell/types.h>
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
string Task::ToString() {
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

char** Task::C_Args() {
    char** args = new char*[this->argv_.size() + 1];
    for (size_t i = 0; i < this->argv_.size(); i++) {
        args[i] = strdup(this->argv_[i].c_str());
    }
    args[this->argv_.size()] = NULL;
    return args;
}

pid_t Task::Exec(Environment& env) {
    if (this->argv_.empty()){
        return ExecError::kSuccess;
    }
    if (builtin::Resolve(this->argv_[0])) {
        return builtin::Exec(this->argv_, env);
    }

    auto resolved_path = env.ResolvePath(this->argv_[0]);
    if (!resolved_path) {
        return ExecError::kFileNotFound;
    } else {
        this->argv_[0] = *resolved_path;
    }

    Pipe last_pipe = env.GetPipe();
    env.SetPipe(0, Pipe::Create());
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
        last_pipe.Close();
        env.AddChildProcess(0, pid);
        return ExecError::kSuccess;
    } else {
        // child process
        switch (this->stdin_.type) {
            case IO::kInherit:
                if (last_pipe.IsEnable()) {
                    last_pipe.DupIn2(STDIN_FILENO);
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
                env.GetPipe(this->stdout_.line).DupOut2(STDOUT_FILENO);
                break;
            case IO::kFile:
                int fd =
                    open(this->stdout_.file.c_str(), (O_RDWR | O_CREAT), 0644);
                dup2(fd, STDOUT_FILENO);
                break;
        }

        switch (this->stderr_.type) {
            case IO::kInherit:
                break;
            case IO::kPipe:
                env.GetPipe(this->stderr_.line).DupOut2(STDOUT_FILENO);
                break;
            case IO::kFile:  // NOTE: Not in spec
                break;
        }

        last_pipe.Close();
        env.CloseAllPipes();

        const char* file = this->argv_[0].c_str();
        char** arg = this->C_Args();
        char** envp = env.C_Params();
        return execvpe(file, arg, envp);
    }
}
}  // namespace np
