#include <npshell/builtin.h>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>
using namespace std;

const vector<string> kBuiltinCommands({"exit", "printenv", "setenv"});
const set<string> kBuiltinCommandsSet(builtin_commands.begin(),
                                      builtin_commands.end());

namespace builtin {

// constvector<string>& commands() { return builtin_commands; }
//
// bool resolve(const string& cmd) {
//     return builtin_commands_set.find(cmd) != builtin_commands_set.end();
// }
//
// int exec(string cmd, std::vector<string> argv, Environment env) {}
//
// int exit(Environment& env);
// int printenv(const string& name, Environment& env) {
//     string value = env.params.find(name);
//     if (
// }
//
// int setenv(const string& name, const string& value, Environment& env) {
//     env.params.insert(name, value);
// }
}  // namespace builtin
