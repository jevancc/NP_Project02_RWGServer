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

int Exec(const vector<string>& argv, Environment& env) {
  if (argv[0] == "exit")
    return exit(env);
  if (argv[0] == "printenv")
    return printenv(argv, env);
  if (argv[0] == "setenv")
    return setenv(argv, env);
  return np::ExecError::kFileNotFound;
}

np::ExecError exit(Environment& env) {
  ::exit(0);
  return np::ExecError::kSuccess;
}

np::ExecError printenv(const vector<string>& argv, Environment& env) {
  if (argv.size() < 2) {
    cerr << "Invalid arguments" << endl;
  } else {
    cout << env.GetParam(argv[1]) << endl;
  }
  return np::ExecError::kSuccess;
}

np::ExecError setenv(const vector<string>& argv, Environment& env) {
  if (argv.size() < 3) {
    cerr << "Invalid arguments" << endl;
  } else {
    env.SetParam(argv[1], argv[2]);
  }
  return np::ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace np
