#ifndef _NP_SHELL_TYPES_H_
#define _NP_SHELL_TYPES_H_

namespace np {
namespace shell {
class Pipe;
class IOOption;
class Command;
class Task;
class Environment;
class Shell;
class ShellConsole;
}  // namespace shell
}  // namespace np

#include <np/shell/builtin.h>
#include <np/shell/constants.h>
#include <map>
#include <memory>
#include <nonstd/optional.hpp>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;
using nonstd::optional;

namespace np {
namespace shell {

class Pipe {
 private:
  int fd_[2];
  static vector<weak_ptr<Pipe>> PipePool_;
  static void CloseAllPipes_();

 public:
  friend class Task;
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

class IOOption {
 public:
  IO type;
  int i_data;
  string s_data;
  IOOption() : type(IO::kInherit) {}

  void SetInherit() {
    this->type = kInherit;
    this->i_data = 0;
    string().swap(this->s_data);
  }
  void SetDelayedPipe(int line) {
    this->type = kDelayedPipe;
    this->i_data = line;
    string().swap(this->s_data);
  }
  void SetUserPipe(int uid) {
    this->type = kUserPipe;
    this->i_data = uid;
    string().swap(this->s_data);
  }
  void SetFile(const string& s) {
    this->type = kFile;
    this->i_data = -1;
    this->s_data = s;
  }

  string ToString() const;
};

class Command {
 private:
  string raw_;
  vector<Task> parsed_commands_;

 public:
  Command(string input);
  vector<Task>& Tasks() { return this->parsed_commands_; }
};

class Task {
 private:
  string original_input_;
  vector<string> argv_;
  shared_ptr<Pipe> in_pipe_;
  IOOption stdin_;
  IOOption stdout_;
  IOOption stderr_;

 public:
  Task(const string& input) : original_input_(input){};
  friend class Command;
  string ToString() const;
  const string& GetFile() const { return this->argv_[0]; }
  const IOOption& GetStdin() const { return this->stdin_; }
  const IOOption& GetStdout() const { return this->stdout_; }
  const IOOption& GetStderr() const { return this->stderr_; }
  void SetInPipe(shared_ptr<Pipe> p) { this->in_pipe_.swap(p); }
  pid_t Execute(Shell& shell);
  char** C_Args() const;
};

class Environment {
 private:
  int uid_;
  int current_line_;
  string name_;

  shared_ptr<Pipe> delayed_pipes_[kMaxDelayedPipes];
  map<int, shared_ptr<Pipe>> user_pipes_;
  vector<pid_t> delayed_child_processes_[kMaxDelayedPipes];
  map<int, vector<pid_t>> user_child_processes_;
  map<string, string> variables_;

 public:
  Environment(int uid) : uid_(uid), current_line_(0), name_("(no name)") {}
  int GetUid() const { return this->uid_; }
  void SetName(const string& s) { this->name_ = s; }
  const string& GetName() const { return this->name_; }

  weak_ptr<Pipe> GetDelayedPipe(int line = 0);
  void SetDelayedPipe(int line, shared_ptr<Pipe> pipe);
  void EnsureDelayedPipe(int line);
  void CloseDelayedPipe(int line);
  void CloseAllDelayedPipes();

  weak_ptr<Pipe> GetUserPipe(int uid);
  void SetUserPipe(int uid, shared_ptr<Pipe> pipe);
  void CloseUserPipe(int uid);
  void CloseAllUserPipes();

  void SetVariable(const string& var, const string& val);
  const string& GetVariable(const string& var) const;
  const map<string, string>& GetVariables() const { return this->variables_; }

  vector<pid_t>& GetDelayedChildProcesses(int line = 0);
  void AddDelayedChildProcess(int line, pid_t pid);
  void Move2DelayedChildProcesses(int line, vector<pid_t>& pids);

  vector<pid_t>& GetUserChildProcesses(int uid);
  map<int, vector<pid_t>>& GetAllUserChildProcesses() {
    return this->user_child_processes_;
  }
  void AddUserChildProcess(int uid, pid_t pid);
  void Move2UserChildProcesses(int uid, vector<pid_t>& pids);

  void GotoNextLine();
};
}  // namespace shell
}  // namespace np

#include <np/shell/shell.h>
#include <np/shell/shell_console.h>

#endif
