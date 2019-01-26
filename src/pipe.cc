#include <easylogging++.h>
#include <np/shell/types.h>
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

Pipe::Pipe() { this->fd_[0] = this->fd_[1] = -1; }
Pipe::~Pipe() { this->Close(); }

shared_ptr<Pipe> Pipe::Create() {
  auto p = make_shared<Pipe>();
  if (pipe(p->fd_) < 0) {
    auto static msg = "failed to create pipe";
    LOG(ERROR) << msg;
    throw runtime_error(msg);
  } else {
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

void Pipe::DupIn2(int fd) { dup2(*this->In(), fd); }

void Pipe::DupOut2(int fd) { dup2(*this->Out(), fd); }

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
