#include <npshell/shell.h>
#include <npshell/types.h>
#include <sys/wait.h>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

namespace np {
Shell::Shell() { this->env_.SetParam("PATH", "bin:.:/bin"); }

void Shell::Run() {
    string input;
    while (true) {
        cout << "% ";
        getline(cin, input);

        Command command(input);
        for (auto task : command.Parse()) {
            // cout << task.ToString() << endl;
            switch (task.Exec(this->env_)) {
                case ExecError::kFileNotFound:
                    cout << "Unknown command: [" << task.GetFile() << "]."
                         << endl;
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
