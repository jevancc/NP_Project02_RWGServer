#include <npshell/types.h>
#include <string.h>
#include <unistd.h>
#include <nonstd/optional.hpp>
#include <vector>
using namespace std;
using nonstd::nullopt;
using nonstd::optional;

#define PIPE_IN 0
#define PIPE_OUT 1

namespace np {

Pipe::Pipe() { this->fd_[0] = this->fd_[1] = -1; }

Pipe Pipe::Create() {
    Pipe p;
    if (pipe(p.fd_) < 0) {
        throw runtime_error("fail to create pipe");
    } else {
        return p;
    }
}

optional<int> Pipe::In() {
    return this->fd_[PIPE_IN] >= 0 ? optional<int>(this->fd_[PIPE_IN])
                                   : nullopt;
}

optional<int> Pipe::Out() {
    return this->fd_[PIPE_OUT] >= 0 ? optional<int>(this->fd_[PIPE_OUT])
                                    : nullopt;
}

void Pipe::DupIn2(int fd) { dup2(*this->In(), fd); }

void Pipe::DupOut2(int fd) { dup2(*this->Out(), fd); }

int Pipe::Close() {
    int status = 0;
    if (this->fd_[PIPE_IN] >= 0) {
        status |= ::close(this->fd_[PIPE_IN]);
        this->fd_[PIPE_IN] = -1;
    }
    if (this->fd_[PIPE_OUT] >= 0) {
        status |= ::close(this->fd_[PIPE_OUT]);
        this->fd_[PIPE_OUT] = -1;
    }
    return status;
}
}  // namespace np
