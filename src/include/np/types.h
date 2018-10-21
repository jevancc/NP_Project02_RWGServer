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

  string ToString() {
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
  const vector<Task>& Parse() { return this->parsed_commands_; }
};

class Environment;
class Task {
 private:
  vector<string> argv_;
  IOOption stdin_;
  IOOption stdout_;
  IOOption stderr_;

 public:
  friend class Command;
  string ToString();
  const string& GetFile() { return this->argv_[0]; }
  const IOOption& GetStdin() { return this->stdin_; }
  const IOOption& GetStdout() { return this->stdout_; }
  const IOOption& GetStderr() { return this->stderr_; }
  pid_t Exec(Environment& env);
  char** C_Args();
};

enum ExecError {
  kSuccess = 0,
  kUnknown = -1,
  kFileNotFound = -2,
  kForkFailed = -3,
};

class Environment {
 private:
  map<string, string> params_;
  Pipe pipes_[kMaxDelayedPipe];
  vector<pid_t> child_processes_[kMaxDelayedPipe];
  int current_line_;

 public:
  Environment() : current_line_(0) {}
  char** C_Params();
  string GetParam(const string& key) { return this->params_[key]; }
  string SetParam(const string& key, const string& value) {
    return this->params_[key] = value;
  }
  optional<string> ResolvePath(const string& s);

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
