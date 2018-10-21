// #include <npshell/environment.h>
#include <npshell/types.h>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace builtin {
const vector<string>& Commands();
bool Resolve(const string& cmd);
int Exec(const vector<string>& argv, Environment& env);
np::ExecError exit(Environment& env);
np::ExecError printenv(const vector<string>& argv, Environment& env);
np::ExecError setenv(const vector<string>& argv, Environment& env);
}  // namespace builtin
}  // namespace np
