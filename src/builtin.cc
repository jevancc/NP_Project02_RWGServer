#include <fmt/core.h>
#include <fmt/ostream.h>
#include <np/shell/builtin.h>
#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace np {
namespace shell {
namespace builtin {
using np::shell::Shell;

const map<string, function<ExecError(const vector<string>&, Shell&)>>&
FunctionsMap() {
  static map<string, function<ExecError(const vector<string>&, Shell&)>>
      kFunctions{{"exit", exit}, {"printenv", printenv}, {"setenv", setenv},
                 {"name", name}, {"yell", yell},         {"tell", tell},
                 {"who", who}};
  return kFunctions;
}

ExecError Execute(const vector<string>& argv_, Shell& shell) {
  if (argv_.empty()) {
    return ExecError::kSuccess;
  }

  auto resolve = FunctionsMap().find(argv_[0]);
  if (resolve == FunctionsMap().end()) {
    return ExecError::kFileNotFound;
  } else {
    auto inherit_stdout = dup(STDOUT_FILENO);
    auto inherit_stderr = dup(STDERR_FILENO);
    dup2(shell.GetSockfd(), STDOUT_FILENO);
    dup2(shell.GetSockfd(), STDERR_FILENO);

    auto status = resolve->second(argv_, shell);

    dup2(inherit_stdout, STDOUT_FILENO);
    dup2(inherit_stderr, STDERR_FILENO);
    close(inherit_stdout);
    close(inherit_stderr);
    return status;
  }
}

#define PARSE_AND_CHECK_ARGV_(rs)           \
  smatch argv;                              \
  {                                         \
    static regex r(rs);                     \
    if (!regex_search(argv_[1], argv, r)) { \
      cerr << "Invalid arguments" << endl;  \
      return ExecError::kSuccess;           \
    }                                       \
  }

ExecError exit(const vector<string>& argv_, Shell& shell) {
  shell.console_.DeleteUser(shell);
  shell.is_alive_ = false;
  return ExecError::kSuccess;
}

ExecError printenv(const vector<string>& argv_, Shell& shell) {
  PARSE_AND_CHECK_ARGV_(R"(^\s*(\S+)$)");
  cout << shell.env.GetVariable(argv[1]) << endl;
  return ExecError::kSuccess;
}

ExecError setenv(const vector<string>& argv_, Shell& shell) {
  PARSE_AND_CHECK_ARGV_(R"(^\s*(\S+)\s+(.+)$)");
  shell.env.SetVariable(argv[1], argv[2]);
  ::setenv(argv[1].str().c_str(), argv[2].str().c_str(), 1);
  return ExecError::kSuccess;
}

ExecError name(const vector<string>& argv_, Shell& shell) {
  PARSE_AND_CHECK_ARGV_(R"(^\s*(.+)$)");
  const string& name = argv[1];
  if (shell.console_.user_names_.count(name) > 0 || name == "(no name)") {
    fmt::print(cout, "*** User '{}' already exists. ***\n", name);
  } else {
    // NOTE: fix <ip>/<port> in login message for demo in class
    shell.console_.Broadcast(fmt::format(
        "*** User from {} is named '{}'. ***\n", "CGILAB/511", name));
    shell.console_.user_names_.erase(shell.env.GetName());
    shell.console_.user_names_.insert(name);
    shell.env.SetName(name);
  }
  return ExecError::kSuccess;
}

ExecError yell(const vector<string>& argv_, Shell& shell) {
  PARSE_AND_CHECK_ARGV_(R"(^\s*(.+)$)");
  const string& message = argv[1];

  shell.console_.Broadcast(
      fmt::format("*** {} yelled ***: {}\n", shell.env.GetName(), message));
  return ExecError::kSuccess;
}

ExecError tell(const vector<string>& argv_, Shell& shell) {
  PARSE_AND_CHECK_ARGV_(R"(^\s*(\d+)\s+(.+)$)");
  const string& uid_s = argv[1];
  const string& message = argv[2];
  try {
    int uid = stoi(uid_s);
    auto user_to = shell.console_.GetUserByUid_(uid).lock();
    if (user_to == nullptr) {
      throw invalid_argument("user not exists");
    } else {
      user_to->Send(fmt::format("*** {} told you ***: {}\n",
                                shell.env.GetName(), message));
    }
  } catch (const invalid_argument& ia) {
    fmt::print(cout, "*** Error: user #{} does not exist yet. ***\n", uid_s);
  }
  return ExecError::kSuccess;
}

ExecError who(const vector<string>& argv_, Shell& shell) {
  fmt::print(cout, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
  for (weak_ptr<Shell>& user_w : shell.console_.GetUsers_()) {
    auto user = user_w.lock();
    if (user) {
      // NOTE: fix <ip>/<port> in login message for demo in class
      fmt::print(cout, "{0}\t{1}\t{2}{3}\n", user->env.GetUid(),
                 user->env.GetName(), "CGILAB/511",
                 user->env.GetUid() == shell.env.GetUid() ? "\t<-me" : "");
    }
  }
  return ExecError::kSuccess;
}

}  // namespace builtin
}  // namespace shell
}  // namespace np
