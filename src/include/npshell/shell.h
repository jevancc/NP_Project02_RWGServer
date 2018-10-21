#include <npshell/types.h>

namespace np {
class Shell {
   private:
    Environment env_;

   public:
    Shell();
    void Run();
};
}  // namespace np
