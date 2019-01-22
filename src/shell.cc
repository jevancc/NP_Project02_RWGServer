#include <np/shell/builtin.h>
#include <np/shell/shell.h>
#include <np/shell/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

namespace np {
namespace shell {
Shell::Shell() { builtin::setenv({"setenv", "PATH", "bin:."}, this->env_); }

void Shell::Run() {
  signal(SIGCHLD, [](int signo) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
      ;
  });

  string input;
  while (true) {
    cout << "% ";
    if (!getline(cin, input)) {
      builtin::exit({"exit"}, this->env_);
    }

    Command command(input);

    if (command.Parse().empty()) {
      continue;
    }
    for (auto task : command.Parse()) {
      int status;

      while ((status = task.Exec(this->env_)) == ExecError::kForkFailed) {
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
    if (last_task.GetStdout().type == IO::kPipe) {
      this->env_.AddChildProcesses(last_task.GetStdout().line,
                                   this->env_.GetChildProcesses());
    }

    for (auto child_pid : this->env_.GetChildProcesses()) {
      int status;
      waitpid(child_pid, &status, 0);
    }
    this->env_.GotoNextLine();
  }
}
}  // namespace shell
}  // namespace np
