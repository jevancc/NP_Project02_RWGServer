#ifndef _NP_SHELL_BUILTIN_H_
#define _NP_SHELL_BUILTIN_H_

#include <np/shell/types.h>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
namespace builtin {
ExecError Exec(const vector<string>& argv, Environment& env);
ExecError exit(const vector<string>& argv, Environment& env);
ExecError printenv(const vector<string>& argv, Environment& env);
ExecError setenv(const vector<string>& argv, Environment& env);
}  // namespace builtin
}  // namespace shell
}  // namespace np

#endif
