#ifndef _NP_BUILTIN_H_
#define _NP_BUILTIN_H_

#include <np/types.h>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace builtin {
np::ExecError Exec(const vector<string>& argv, Environment& env);
np::ExecError exit(const vector<string>& argv, Environment& env);
np::ExecError printenv(const vector<string>& argv, Environment& env);
np::ExecError setenv(const vector<string>& argv, Environment& env);
}  // namespace builtin
}  // namespace np

#endif
