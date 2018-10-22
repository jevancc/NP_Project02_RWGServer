#include <np/builtin.h>
#include <np/constants.h>
#include <np/types.h>
#include <signal.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace builtin {

const map<string,
          function<np::ExecError(const vector<string>& argv, Environment& env)>>
    kCommands{
        {"exit", exit},
        {"printenv", printenv},
        {"setenv", setenv},
    };

np::ExecError Exec(const vector<string>& argv, Environment& env) {
  if (argv.empty()) {
    return ExecError::kSuccess;
  }

  auto resolve = kCommands.find(argv[0]);
  if (resolve == kCommands.end()) {
    return ExecError::kFileNotFound;
  } else {
    return resolve->second(argv, env);
  }
}

np::ExecError exit(const vector<string>& argv, Environment& env) {
  for (int i = 0; i < kMaxDelayedPipe; i++) {
    for (auto& pid : env.GetChildProcess(i)) {
      ::kill(pid, SIGKILL);
    }
  }
  ::exit(0);
  return np::ExecError::kSuccess;
}

np::ExecError printenv(const vector<string>& argv, Environment& env) {
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

np::ExecError setenv(const vector<string>& argv, Environment& env) {
  if (argv.size() < 3) {
    cerr << "Invalid arguments" << endl;
  } else {
    ::setenv(argv[1].c_str(), argv[2].c_str(), 1);
  }
  return np::ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace np
