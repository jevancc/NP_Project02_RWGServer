#ifndef _NP_SHELL_SHELL_H_
#define _NP_SHELL_SHELL_H_

#include <np/shell/types.h>

namespace np {
namespace shell {
class Shell {
 private:
  Environment env_;

 public:
  Shell();
  void Run();
};
}  // namespace shell
}  // namespace np
#endif
