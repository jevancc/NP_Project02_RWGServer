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

  string ToString() const {
    char s[32];
    switch (this->type) {
      case IO::kInherit:
        sprintf(s, "Inherit");
        break;
      case IO::kDelayedPipe:
        sprintf(s, "Delayed Pipe %d", this->i_data);
        break;
      case IO::kUserPipe:
        sprintf(s, "User Pipe %d", this->i_data);
        break;
      case IO::kFile:
        sprintf(s, "File \"%s\"", this->s_data.c_str());
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
  Command(string input);
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
  pid_t Execute(Shell& shell);
  char** C_Args() const;
};

class Environment {
 private:
  int uid_;
  int current_line_;
  string name_;

  shared_ptr<Pipe> delayed_pipes_[kMaxDelayedPipes];
  shared_ptr<Pipe> user_pipes_[kMaxShellUsers];
  vector<pid_t> delayed_child_processes_[kMaxDelayedPipes];
  vector<pid_t> user_child_processes_[kMaxShellUsers];
  map<string, string> variables_;

 public:
  Environment(int uid) : uid_(uid), current_line_(0), name_("(no name)") {}
  int GetUid() const { return this->uid_; }
  void SetName(const string& s) { this->name_ = s; }
  const string& GetName() const { return this->name_; }

  shared_ptr<Pipe> GetDelayedPipe(int line = 0);
  void SetDelayedPipe(int line, shared_ptr<Pipe> pipe);
  void CreateDelayedPipe(int line);
  void CloseDelayedPipe(int line);
  void CloseAllDelayedPipes();

  shared_ptr<Pipe> GetUserPipe(int uid = 0);
  void SetUserPipe(int uid, shared_ptr<Pipe> pipe);
  void CreateUserPipe(int uid);
  void CloseUserPipe(int uid);
  void CloseAllUserPipes();

  void SetVariable(const string& var, const string& val);
  const string& GetVariable(const string& var) const;
  const map<string, string>& GetVariables() const;

  vector<pid_t>& GetDelayedChildProcesses(int line = 0);
  void AddDelayedChildProcess(int line, pid_t pid);
  void AddDelayedChildProcesses(int line, vector<pid_t>& pids);

  vector<pid_t>& GetUserChildProcesses(int uid = 0);
  void AddUserChildProcess(int uid, pid_t pid);
  void AddUserChildProcesses(int uid, vector<pid_t>& pids);

  void GotoNextLine();
};
}  // namespace shell
}  // namespace np

#include <np/shell/shell.h>
#include <np/shell/shell_console.h>

#endif
