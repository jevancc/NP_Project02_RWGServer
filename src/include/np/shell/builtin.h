#ifndef _NP_SHELL_BUILTIN_H_
#define _NP_SHELL_BUILTIN_H_

#include <np/shell/constants.h>
#include <map>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
class Shell;
namespace builtin {
using np::shell::Shell;
ExecError Execute(const vector<string>& argv, Shell& shell);
ExecError exit(const vector<string>& argv, Shell& shell);
ExecError printenv(const vector<string>& argv, Shell& shell);
ExecError setenv(const vector<string>& argv, Shell& shell);

ExecError name(const vector<string>& argv, Shell& shell);
ExecError yell(const vector<string>& argv, Shell& shell);
ExecError tell(const vector<string>& argv, Shell& shell);
ExecError who(const vector<string>& argv, Shell& shell);

}  // namespace builtin
}  // namespace shell
}  // namespace np

#include <np/shell/types.h>

#endif
