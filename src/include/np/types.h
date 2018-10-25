#ifndef _NP_TYPES_H_
#define _NP_TYPES_H_

#include <np/constants.h>
#include <map>
#include <nonstd/optional.hpp>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

namespace np {

class Pipe {
 private:
  int fd_[2];

 public:
  static Pipe Create();
  Pipe();

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

class Task;
class Command {
 private:
  string raw_;
  vector<Task> parsed_commands_;

 public:
  Command(string command);
  const vector<Task>& Parse() const { return this->parsed_commands_; }
};

class Environment;
class Task {
 private:
  vector<string> argv_;
  optional<Pipe> in_pipe_;
  IOOption stdin_;
  IOOption stdout_;
  IOOption stderr_;

 public:
  Task() : in_pipe_(nullopt){};
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
  Pipe pipes_[kMaxDelayedPipe];
  vector<pid_t> child_processes_[kMaxDelayedPipe];
  int current_line_;

 public:
  Environment() : current_line_(0) {}

  Pipe& GetPipe(int line = 0) {
    return this->pipes_[(line + current_line_) % kMaxDelayedPipe];
  }
  void SetPipe(int line, Pipe pipe) {
    this->pipes_[(line + current_line_) % kMaxDelayedPipe] = pipe;
  }
  void CreatePipe(int line) {
    int line_idx = (line + current_line_) % kMaxDelayedPipe;
    if (!this->pipes_[line_idx].IsEnable()) {
      this->pipes_[line_idx] = Pipe::Create();
    }
  }
  void ClosePipe(int line) {
    this->pipes_[(line + current_line_) % kMaxDelayedPipe].Close();
  }
  void CloseAllPipes();

  vector<pid_t>& GetChildProcess(int line = 0) {
    return this->child_processes_[(line + current_line_) % kMaxDelayedPipe];
  }
  void AddChildProcess(int line, pid_t pid) {
    int line_idx = (line + current_line_) % kMaxDelayedPipe;
    this->child_processes_[line_idx].push_back(pid);
  }
  void AddChildProcesses(int line, vector<pid_t>& pids) {
    int line_idx = (line + current_line_) % kMaxDelayedPipe;
    this->child_processes_[line_idx].insert(
        this->child_processes_[line_idx].end(), pids.begin(), pids.end());
    pids.clear();
  }

  void GotoNextLine();
};

}  // namespace np

#endif
