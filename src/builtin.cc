#include <np/builtin.h>
#include <np/constants.h>
#include <np/types.h>
#include <signal.h>
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
    return printenv(argv);
  if (argv[0] == "setenv")
    return setenv(argv);
  return np::ExecError::kFileNotFound;
}

np::ExecError exit(Environment& env) {
  for (int i = 0; i < kMaxDelayedPipe; i++) {
    for (auto& pid : env.GetChildProcess(i)) {
      kill(pid, SIGKILL);
    }
  }
  ::exit(0);
  return np::ExecError::kSuccess;
}

np::ExecError printenv(const vector<string>& argv) {
  if (argv.size() < 2) {
    cerr << "Invalid arguments" << endl;
  } else {
    const char* envp = ::getenv(argv[1].c_str());
    if (envp) {
      cout << ::getenv(argv[1].c_str()) << endl;
    } else {
      cout << endl;
    }
  }
  return np::ExecError::kSuccess;
}

np::ExecError setenv(const vector<string>& argv) {
  if (argv.size() < 3) {
    cerr << "Invalid arguments" << endl;
  } else {
    ::setenv(argv[1].c_str(), argv[2].c_str(), 1);
  }
  return np::ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace np
