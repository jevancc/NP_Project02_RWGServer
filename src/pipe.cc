#include <np/shell/types.h>
#include <spdlog/spdlog.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include <nonstd/optional.hpp>
#include <vector>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

const int kPipeIn = 0;
const int kPipeOut = 1;

namespace np {
namespace shell {

vector<weak_ptr<Pipe>> Pipe::PipePool_;

Pipe::Pipe() { this->fd_[0] = this->fd_[1] = -1; }
Pipe::~Pipe() { this->Close(); }

void Pipe::CloseAllPipes_() {
  for (weak_ptr<Pipe> p_w : Pipe::PipePool_) {
    if (auto p = p_w.lock()) {
      p->Close();
    }
  }
  Pipe::PipePool_.clear();
}

shared_ptr<Pipe> Pipe::Create() {
  auto p = make_shared<Pipe>();
  if (pipe(p->fd_) < 0) {
    auto static msg = "failed to create pipe";
    spdlog::error(msg);
    throw runtime_error(msg);
  } else {
    Pipe::PipePool_.push_back(p);
    return p;
  }
}

optional<int> Pipe::In() {
  return this->fd_[kPipeIn] >= 0 ? optional<int>(this->fd_[kPipeIn]) : nullopt;
}

optional<int> Pipe::Out() {
  return this->fd_[kPipeOut] >= 0 ? optional<int>(this->fd_[kPipeOut])
                                  : nullopt;
}

void Pipe::DupIn2(int fd) {
  if (this->In()) {
    dup2(*this->In(), fd);
  } else {
    static const char* msg = "failed to dup pipe fd: pipe is closed";
    spdlog::error(msg);
    throw runtime_error(msg);
  }
}

void Pipe::DupOut2(int fd) {
  if (this->Out()) {
    dup2(*this->Out(), fd);
  } else {
    static const char* msg = "failed to dup pipe fd: pipe is closed";
    spdlog::error(msg);
    throw runtime_error(msg);
  }
}

int Pipe::Close() {
  int status = 0;
  if (this->fd_[kPipeIn] >= 0) {
    status |= ::close(this->fd_[kPipeIn]);
    this->fd_[kPipeIn] = -1;
  }
  if (this->fd_[kPipeOut] >= 0) {
    status |= ::close(this->fd_[kPipeOut]);
    this->fd_[kPipeOut] = -1;
  }
  return status;
}
}  // namespace shell
}  // namespace np
