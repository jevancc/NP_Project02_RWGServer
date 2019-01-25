#include <np/shell/builtin.h>
#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <signal.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
namespace builtin {
using np::shell::Shell;

const map<string, function<ExecError(const vector<string>& argv, Shell& shell)>>
    kCommands{{"exit", exit}, {"printenv", printenv}, {"setenv", setenv},
              {"name", name}, {"yell", yell},         {"tell", tell},
              {"who", who}};

ExecError Execute(const vector<string>& argv, Shell& shell) {
  if (argv.empty()) {
    return ExecError::kSuccess;
  }

  auto resolve = kCommands.find(argv[0]);
  if (resolve == kCommands.end()) {
    return ExecError::kFileNotFound;
  } else {
    auto inherit_stdout = dup(STDOUT_FILENO);
    auto inherit_stderr = dup(STDERR_FILENO);
    dup2(shell.GetSockfd(), STDOUT_FILENO);
    dup2(shell.GetSockfd(), STDERR_FILENO);

    auto status = resolve->second(argv, shell);

    dup2(inherit_stdout, STDOUT_FILENO);
    dup2(inherit_stderr, STDERR_FILENO);
    close(inherit_stdout);
    close(inherit_stderr);
    return status;
  }
}

#define _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(a, s) \
  {                                             \
    if (a.size() < s) {                         \
      cerr << "Invalid arguments" << endl;      \
      return ExecError::kSuccess;               \
    }                                           \
  }

ExecError exit(const vector<string>& argv, Shell& shell) {
  for (int i = 0; i < kMaxDelayedPipes; i++) {
    for (auto& pid : shell.env.GetDelayedChildProcesses(i)) {
      ::kill(pid, SIGKILL);
    }
  }
  ::exit(0);
  return ExecError::kSuccess;
}

ExecError printenv(const vector<string>& argv, Shell& shell) {
  _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(argv, 2);
  cout << shell.env.GetVariable(argv[1]) << endl;
  return ExecError::kSuccess;
}

ExecError setenv(const vector<string>& argv, Shell& shell) {
  _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(argv, 3);
  shell.env.SetVariable(argv[1], argv[2]);
  ::setenv(argv[1].c_str(), argv[2].c_str(), 1);
  return ExecError::kSuccess;
}

ExecError name(const vector<string>& argv, Shell& shell) {
  _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(argv, 2);
  const string& name = argv[1];
  if (shell.console_.user_names_.count(name) > 0 || name == "(no name)") {
    cout << "*** User '" << name << "' already exists. ***" << endl;
  } else {
    stringstream ss;
    // NOTE: fix <ip>/<port> in login message for demo in class
    ss << "*** User from CGILAB/511 is named '" << name << "'. ***" << endl;
    shell.console_.user_names_.erase(shell.env.GetName());
    shell.console_.user_names_.insert(name);
    shell.env.SetName(name);
    shell.console_.Broadcast(ss.str());
  }
  return ExecError::kSuccess;
}

ExecError yell(const vector<string>& argv, Shell& shell) {
  _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(argv, 2);
  const string& message = argv[1];
  stringstream ss;
  ss << "*** " << shell.env.GetName() << " yelled ***: " << message << endl;
  shell.console_.Broadcast(ss.str());
  return ExecError::kSuccess;
}

ExecError tell(const vector<string>& argv, Shell& shell) {
  _NP_SHELL_BUILTIN_CHECK_ARGV_SIZE(argv, 3);
  const string& uid_s = argv[1];
  const string& message = argv[2];
  try {
    int uid = stoi(uid_s);
    auto user_to = shell.console_.GetUserByUid(uid).lock();
    if (user_to == nullptr) {
      throw invalid_argument("user not exists");
    } else {
      stringstream ss;
      ss << "*** " << shell.env.GetName() << " told you ***: " << message
         << endl;
      user_to->Send(ss.str());
    }
  } catch (const invalid_argument& ia) {
    cout << "*** Error: user #" << uid_s << " does not exist yet. ***" << endl;
  }
  return ExecError::kSuccess;
}
ExecError who(const vector<string>& argv, Shell& shell) {
  cout << "<ID>\t<nickname>\t<IP/port>\t<indicate me>" << endl;
  for (weak_ptr<Shell>& user_w : shell.console_.GetUsers()) {
    auto user = user_w.lock();
    if (user) {
      cout << user->env.GetUid() << "\t" << user->env.GetName() << "\t"
           << "CGILAB/511"
           << "\t";
      if (user->env.GetUid() == shell.env.GetUid()) {
        cout << "<- me";
      }
      cout << endl;
    }
  }
  return ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace shell
}  // namespace np
