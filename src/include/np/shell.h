#ifndef _NP_SHELL_H_
#define _NP_SHELL_H_

#include <np/types.h>

namespace np {
class Shell {
 private:
  Environment env_;

 public:
  Shell();
  void Run();
};
}  // namespace np

#endif
