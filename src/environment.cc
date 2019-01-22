#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <sys/stat.h>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {

shared_ptr<Pipe> Environment::GetPipe(int line) {
  return this->pipes_[(line + current_line_) % kMaxDelayedPipe];
}
void Environment::SetPipe(int line, shared_ptr<Pipe> pipe) {
  this->pipes_[(line + current_line_) % kMaxDelayedPipe] = pipe;
}
void Environment::CreatePipe(int line) {
  int line_idx = (line + current_line_) % kMaxDelayedPipe;
  if (!this->pipes_ || !this->pipes_[line_idx]->IsEnable()) {
    this->pipes_[line_idx] = Pipe::Create();
  }
}
void Environment::ClosePipe(int line) {
  int index = (line + current_line_) % kMaxDelayedPipe;
  if (this->pipes_[index]) {
    this->pipes_[(line + current_line_) % kMaxDelayedPipe]->Close();
  }
}
void Environment::CloseAllPipes() {
  for (int i = 0; i < kMaxDelayedPipe; i++) {
    if (this->pipes_[i]) {
      this->pipes_[i]->Close();
    }
  }
}

vector<pid_t>& Environment::GetChildProcesses(int line) {
  return this->child_processes_[(line + current_line_) % kMaxDelayedPipe];
}
void Environment::AddChildProcess(int line, pid_t pid) {
  int line_idx = (line + current_line_) % kMaxDelayedPipe;
  this->child_processes_[line_idx].push_back(pid);
}
void Environment::AddChildProcesses(int line, vector<pid_t>& pids) {
  int line_idx = (line + current_line_) % kMaxDelayedPipe;
  this->child_processes_[line_idx].insert(
      this->child_processes_[line_idx].end(), pids.begin(), pids.end());
  pids.clear();
}

void Environment::GotoNextLine() {
  if (this->pipes_[this->current_line_]) {
    this->pipes_[this->current_line_]->Close();
  }
  this->child_processes_[this->current_line_].clear();
  this->current_line_ = (this->current_line_ + 1) % kMaxDelayedPipe;
}
}  // namespace shell
}  // namespace np
