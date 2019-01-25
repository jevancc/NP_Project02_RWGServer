#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <sys/stat.h>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {

shared_ptr<Pipe> Environment::GetDelayedPipe(int line) {
  return this->delayed_pipes_[(line + current_line_) % kMaxDelayedPipes];
}
void Environment::SetDelayedPipe(int line, shared_ptr<Pipe> pipe) {
  this->delayed_pipes_[(line + current_line_) % kMaxDelayedPipes] = pipe;
}
void Environment::CreateDelayedPipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  if (!this->delayed_pipes_[line_idx] ||
      !this->delayed_pipes_[line_idx]->IsEnable()) {
    this->delayed_pipes_[line_idx] = Pipe::Create();
  }
}
void Environment::CloseDelayedPipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  if (this->delayed_pipes_[line_idx]) {
    this->delayed_pipes_[line_idx]->Close();
  }
}
void Environment::CloseAllDelayedPipes() {
  for (int i = 0; i < kMaxDelayedPipes; i++) {
    if (this->delayed_pipes_[i]) {
      this->delayed_pipes_[i]->Close();
    }
  }
}

shared_ptr<Pipe> Environment::GetUserPipe(int uid) {
  return this->user_pipes_[uid];
}
void Environment::SetUserPipe(int uid, shared_ptr<Pipe> pipe) {
  this->user_pipes_[uid] = pipe;
}
void Environment::CreateUserPipe(int uid) {
  if (!this->user_pipes_[uid] || !this->user_pipes_[uid]->IsEnable()) {
    this->user_pipes_[uid] = Pipe::Create();
  }
}
void Environment::CloseUserPipe(int uid) {
  if (this->user_pipes_[uid]) {
    this->user_pipes_[uid]->Close();
  }
}
void Environment::CloseAllUserPipes() {
  for (int i = 0; i < kMaxShellUsers; i++) {
    if (this->user_pipes_[i]) {
      this->user_pipes_[i]->Close();
    }
  }
}

void Environment::SetVariable(const string& var, const string& val) {
  this->variables_[var] = val;
}
const string& Environment::GetVariable(const string& var) const {
  map<string, string>::const_iterator it;
  if ((it = this->variables_.find(var)) != this->variables_.end()) {
    return it->second;
  } else {
    static const string empty_str;
    return empty_str;
  }
}
const map<string, string>& Environment::GetVariables() const {
  return this->variables_;
}

vector<pid_t>& Environment::GetDelayedChildProcesses(int line) {
  return this
      ->delayed_child_processes_[(line + current_line_) % kMaxDelayedPipes];
}
void Environment::AddDelayedChildProcess(int line, pid_t pid) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  this->delayed_child_processes_[line_idx].push_back(pid);
}
void Environment::AddDelayedChildProcesses(int line, vector<pid_t>& pids) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  this->delayed_child_processes_[line_idx].insert(
      this->delayed_child_processes_[line_idx].end(), pids.begin(), pids.end());
  pids.clear();
}

vector<pid_t>& Environment::GetUserChildProcesses(int uid) {
  return this->user_child_processes_[uid];
}
void Environment::AddUserChildProcess(int uid, pid_t pid) {
  this->user_child_processes_[uid].push_back(pid);
}
void Environment::AddUserChildProcesses(int uid, vector<pid_t>& pids) {
  this->user_child_processes_[uid].insert(
      this->user_child_processes_[uid].end(), pids.begin(), pids.end());
  pids.clear();
}

void Environment::GotoNextLine() {
  if (this->delayed_pipes_[this->current_line_]) {
    this->delayed_pipes_[this->current_line_]->Close();
  }
  this->delayed_child_processes_[this->current_line_].clear();
  this->current_line_ = (this->current_line_ + 1) % kMaxDelayedPipes;
}
}  // namespace shell
}  // namespace np
