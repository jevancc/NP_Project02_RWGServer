#ifndef _NP_SHELL_CONSTANTS_H_
#define _NP_SHELL_CONSTANTS_H_

#include <unistd.h>

const int kMaxDelayedPipes = 2048;

enum IO {
  kInherit = 0,
  kDelayedPipe,
  kUserPipe,
  kFile,
};

enum ExecError {
  kSuccess = 0,
  kUnknown = -1,
  kFileNotFound = -2,
  kForkFailed = -3,
};

#endif
