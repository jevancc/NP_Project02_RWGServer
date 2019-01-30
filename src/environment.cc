#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <sys/stat.h>
#include <memory>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {

weak_ptr<Pipe> Environment::GetDelayedPipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  if (!this->delayed_pipes_[line_idx] ||
      !this->delayed_pipes_[line_idx]->IsEnable()) {
    this->delayed_pipes_[line_idx].reset();
  }
  return this->delayed_pipes_[line_idx];
}
void Environment::SetDelayedPipe(int line, shared_ptr<Pipe> pipe) {
  this->delayed_pipes_[(line + current_line_) % kMaxDelayedPipes] = pipe;
}
void Environment::EnsureDelayedPipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  if (!this->delayed_pipes_[line_idx] ||
      !this->delayed_pipes_[line_idx]->IsEnable()) {
    this->delayed_pipes_[line_idx] = Pipe::Create();
  }
}
void Environment::CloseDelayedPipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  this->delayed_pipes_[line_idx].reset();
}
void Environment::CloseAllDelayedPipes() {
  for (int i = 0; i < kMaxDelayedPipes; i++) {
    this->delayed_pipes_[i].reset();
  }
}

weak_ptr<Pipe> Environment::GetUserPipe(int uid) {
  if (!this->user_pipes_[uid] || !this->user_pipes_[uid]->IsEnable()) {
    this->user_pipes_[uid].reset();
  }
  return this->user_pipes_[uid];
}
void Environment::SetUserPipe(int uid, shared_ptr<Pipe> pipe) {
  this->user_pipes_[uid].swap(pipe);
}
void Environment::CloseUserPipe(int uid) { this->user_pipes_[uid].reset(); }
void Environment::CloseAllUserPipes() { this->user_pipes_.clear(); }

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

vector<pid_t>& Environment::GetDelayedChildProcesses(int line) {
  return this
      ->delayed_child_processes_[(line + current_line_) % kMaxDelayedPipes];
}
void Environment::AddDelayedChildProcess(int line, pid_t pid) {
  int line_idx = (line + current_line_) % kMaxDelayedPipes;
  this->delayed_child_processes_[line_idx].push_back(pid);
}
void Environment::Move2DelayedChildProcesses(int line, vector<pid_t>& pids) {
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
void Environment::Move2UserChildProcesses(int uid, vector<pid_t>& pids) {
  this->user_child_processes_[uid].insert(
      this->user_child_processes_[uid].end(), pids.begin(), pids.end());
  pids.clear();
}

void Environment::GotoNextLine() {
  if (this->delayed_pipes_[this->current_line_]) {
    this->delayed_pipes_[this->current_line_].reset();
  }
  this->current_line_ = (this->current_line_ + 1) % kMaxDelayedPipes;
}
}  // namespace shell
}  // namespace np
