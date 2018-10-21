#include <npshell/builtin.h>
#include <npshell/types.h>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>
using namespace std;

const vector<string> kBuiltinCommands({"exit", "printenv", "setenv"});
const set<string> kBuiltinCommandsSet(kBuiltinCommands.begin(),
                                      kBuiltinCommands.end());

namespace np {
namespace builtin {

const vector<string>& Commands() { return kBuiltinCommands; }

bool Resolve(const string& cmd) {
    return kBuiltinCommandsSet.find(cmd) != kBuiltinCommandsSet.end();
}

int Exec(vector<string>& argv, Environment& env) {
    if (argv[0] == "exit")
        return exit(env);
    if (argv[0] == "printenv")
        return printenv(argv[1], env);
    if (argv[0] == "setenv")
        return setenv(argv[1], argv[2], env);
    return np::ExecError::kFileNotFound;
}

np::ExecError exit(Environment& env) {
    ::exit(0);
    return np::ExecError::kSuccess;
}

np::ExecError printenv(const string& name, Environment& env) {
    cout << env.GetParam(name) << endl;
    return np::ExecError::kSuccess;
}

np::ExecError setenv(const string& name, const string& value,
                     Environment& env) {
    env.SetParam(name, value);
    return np::ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace np
