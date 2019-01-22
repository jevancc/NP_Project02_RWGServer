#ifndef _NP_SHELL_TYPES_H_
#define _NP_SHELL_TYPES_H_

#include <np/shell/constants.h>
#include <map>
#include <memory>
#include <nonstd/optional.hpp>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

namespace np {
namespace shell {
class Pipe;
class IOOption;
class Command;
class Task;
class Environment;

class Pipe {
 private:
  int fd_[2];

 public:
  static shared_ptr<Pipe> Create();
  Pipe();
  ~Pipe();

  optional<int> In();
  optional<int> Out();
  void DupIn2(int fd);
  void DupOut2(int fd);
  int Close();
  inline bool IsEnable() { return this->fd_[0] != -1 || this->fd_[1] != -1; }

  inline optional<int> I() { return this->In(); }
  inline optional<int> O() { return this->Out(); }
};

enum IO {
  kInherit = 0,
  kPipe,
  kFile,
};

class IOOption {
 public:
  IO type;
  string file;
  int line;
  IOOption() : type(IO::kInherit) {}

  string ToString() const {
    char s[32];
    switch (this->type) {
      case IO::kInherit:
        sprintf(s, "Inherit");
        break;
      case IO::kPipe:
        sprintf(s, "Pipe %d", this->line);
        break;
      case IO::kFile:
        sprintf(s, "File \"%s\"", this->file.c_str());
        break;
      default:
        sprintf(s, "Unknown type[%d]", this->type);
        break;
    }
    return string(s);
  }
};

class Command {
 private:
  string raw_;
  vector<Task> parsed_commands_;

 public:
  Command(string command);
  const vector<Task>& Parse() const { return this->parsed_commands_; }
};

class Task {
 private:
  vector<string> argv_;
  shared_ptr<Pipe> in_pipe_;
  IOOption stdin_;
  IOOption stdout_;
  IOOption stderr_;

 public:
  Task(){};
  friend class Command;
  string ToString() const;
  const string& GetFile() const { return this->argv_[0]; }
  const IOOption& GetStdin() const { return this->stdin_; }
  const IOOption& GetStdout() const { return this->stdout_; }
  const IOOption& GetStderr() const { return this->stderr_; }
  pid_t Exec(Environment& env);
  char** C_Args() const;
};

enum ExecError {
  kSuccess = 0,
  kUnknown = -1,
  kFileNotFound = -2,
  kForkFailed = -3,
};

class Environment {
 private:
  shared_ptr<Pipe> pipes_[kMaxDelayedPipe];
  vector<pid_t> child_processes_[kMaxDelayedPipe];
  int current_line_;

 public:
  Environment() : current_line_(0) {}

  shared_ptr<Pipe> GetPipe(int line = 0);
  void SetPipe(int line, shared_ptr<Pipe> pipe);
  void CreatePipe(int line);
  void ClosePipe(int line);
  void CloseAllPipes();

  vector<pid_t>& GetChildProcesses(int line = 0);
  void AddChildProcess(int line, pid_t pid);
  void AddChildProcesses(int line, vector<pid_t>& pids);

  void GotoNextLine();
};
}  // namespace shell
}  // namespace np

#endif
