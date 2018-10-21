#include <np/constants.h>
#include <np/types.h>
#include <sys/stat.h>
#include <apathy/path.hpp>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
using namespace std;

namespace np {

void Environment::CloseAllPipes() {
  for (int i = 0; i < kMaxDelayedPipe; i++) {
    this->pipes_[i].Close();
  }
}

void Environment::GotoNextLine() {
  this->pipes_[this->current_line_].Close();
  this->child_processes_[this->current_line_].clear();
  this->current_line_ = (this->current_line_ + 1) % kMaxDelayedPipe;
}

}  // namespace np
