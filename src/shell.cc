#include <npshell/shell.h>
#include <npshell/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

namespace np {
Shell::Shell() { this->env_.SetParam("PATH", "bin:."); }

void Shell::Run() {
  signal(SIGCHLD, [](int signo) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
      ;
  });

  string input;
  while (true) {
    cout << "% ";
    getline(cin, input);

    Command command(input);
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
    if (!command.Parse().empty()) {
      auto last_task = *command.Parse().rbegin();
      if (last_task.GetStdout().type == IO::kPipe) {
        this->env_.AddChildProcesses(last_task.GetStdout().line,
                                     this->env_.GetChildProcess());
      }
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
