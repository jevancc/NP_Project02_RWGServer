#ifndef _NP_SHELL_BUILTIN_H_
#define _NP_SHELL_BUILTIN_H_

#include <np/shell/constants.h>
#include <functional>
#include <map>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
class Shell;
namespace builtin {
using np::shell::Shell;
const map<string, function<ExecError(const vector<string>&, Shell&)>>&
FunctionsMap();

ExecError Execute(const vector<string>&, Shell&);
ExecError exit(const vector<string>&, Shell&);
ExecError printenv(const vector<string>&, Shell&);
ExecError setenv(const vector<string>&, Shell&);

ExecError name(const vector<string>&, Shell&);
ExecError yell(const vector<string>&, Shell&);
ExecError tell(const vector<string>&, Shell&);
ExecError who(const vector<string>&, Shell&);

}  // namespace builtin
}  // namespace shell
}  // namespace np

#include <np/shell/types.h>

#endif
