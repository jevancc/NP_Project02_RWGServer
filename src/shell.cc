#include <np/builtin.h>
#include <np/shell.h>
#include <np/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

namespace np {
Shell::Shell() { setenv("PATH", "bin:.", 1); }

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
      builtin::exit(this->env_);
    }

    Command command(input);
    if (command.Parse().empty()) {
      continue;
    }
    for (auto task : command.Parse()) {
      // cout << task.ToString() << endl;
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
                                   this->env_.GetChildProcess());
    }

    for (auto child_pid : this->env_.GetChildProcess()) {
      int status;
      // cout << "WAITING " << child_pid << endl;
      waitpid(child_pid, &status, 0);
    }
    // cout << " DONE " << endl;
    this->env_.GotoNextLine();
  }
}
}  // namespace np
