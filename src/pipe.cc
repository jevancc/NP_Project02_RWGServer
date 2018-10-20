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

optional<Pipe> Pipe::Create() {
    Pipe p;
    if (pipe(p.fd_) < 0) {
        return p;
    } else {
        return nullopt;
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

optional<int> Pipe::GetIn() {
    if (this->fd_[PIPE_OUT] >= 0) {
        if (::close(this->fd_[PIPE_OUT]) < 0) {
            return nullopt;
        }
        this->fd_[PIPE_OUT] = -1;
    }
    return this->In();
}

optional<int> Pipe::GetOut() {
    if (this->fd_[PIPE_IN] >= 0) {
        if (::close(this->fd_[PIPE_IN]) < 0) {
            return nullopt;
        }
        this->fd_[PIPE_IN] = -1;
    }
    return this->Out();
}

int Pipe::Close() {
    return (this->fd_[PIPE_IN] >= 0 ? ::close(this->fd_[PIPE_IN]) : 0) |
           (this->fd_[PIPE_OUT] >= 0 ? ::close(this->fd_[PIPE_OUT]) : 0);
}
}  // namespace np
