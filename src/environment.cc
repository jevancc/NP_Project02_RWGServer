#include <np/constants.h>
#include <np/types.h>
#include <sys/stat.h>
#include <apathy/path.hpp>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
using namespace std;

namespace np {
char** Environment::C_Params() {
  char** params = new char*[this->params_.size() + 1];
  int i = 0;
  for (auto ent = this->params_.begin(); ent != this->params_.end();
       ent++, i++) {
    string s = ent->first + "=" + ent->second;
    params[i] = strdup(s.c_str());
  }
  params[this->params_.size()] = NULL;
  return params;
}

void Environment::CloseAllPipes() {
  for (int i = 0; i < kMaxDelayedPipe; i++) {
    this->pipes_[i].Close();
  }
}

nonstd::optional<string> Environment::ResolvePath(const string& path) {
  if (path.find('/') != string::npos) {
    return path;
  }

  string p_path = this->GetParam("PATH");
  vector<string> paths;
  for (size_t p = 0; (p = p_path.find(':')) != string::npos;) {
    paths.push_back(p_path.substr(0, p));
    p_path.erase(p_path.begin(), p_path.begin() + p + 1);
  }
  if (!p_path.empty()) {
    paths.push_back(p_path);
  }

  struct stat buffer;
  for (int i = int(paths.size()) - 1; i >= 0; i--) {
    string absolute_path =
        apathy::Path(paths[i]).relative(path).absolute().sanitize().string();
    if (stat(absolute_path.c_str(), &buffer) == 0) {
      return absolute_path;
    }
  }
  return nonstd::nullopt;
}

void Environment::GotoNextLine() {
  this->pipes_[this->current_line_].Close();
  this->child_processes_[this->current_line_].clear();
  this->current_line_ = (this->current_line_ + 1) % kMaxDelayedPipe;
}

}  // namespace np
