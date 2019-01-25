#include <np/shell/builtin.h>
#include <np/shell/types.h>
#include <np/utils.h>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
using namespace std;

namespace np {
namespace shell {
Command::Command(string command) {
  static regex pipe_regex(R"([^|!]+((\||!)\d+\s*$|\||$))");
  static regex argv_regex(R"((\||!|>|<)(\d+)+|(\|)|(>\s+)?(\S+))");
  this->raw_ = utils::trim(command);

  // for (auto& builtin: builtin::kCommands) {
  //   if (utils::is_prefix(builtin.first, command)) {
  //     Task task;
  //     Task.argv_.push_back(builtin.first,
  //     utils::trim(command.substr(builtin.first.size(), command.size())));
  //     this->parsed_commands_.push_back(task);
  //     return;
  //   }
  // }

  smatch sm;
  vector<string> commands;
  while (regex_search(command, sm, pipe_regex)) {
    commands.push_back(sm[0]);

    command = sm.suffix().str();
    if (utils::trim(command).size() == 0) {
      break;
    }
  }

  for (auto s : commands) {
    Task task;

    utils::trim(s);
    while (regex_search(s, sm, argv_regex)) {
      if (sm[2].length() > 0) {
        // if x == "|\d+" or "!\d+" or ">\d+" or "<\d+"
        if (sm[1] == '>') {
          int user_id = stoi(sm[2]);
          task.stdout_.SetUserPipe(user_id);
          task.stderr_.SetUserPipe(user_id);
        } else if (sm[1] == '<') {
          int user_id = stoi(sm[2]);
          task.stdin_.SetUserPipe(user_id);
        } else {
          // x == "|\d+" or "!\d+"
          int line_number = stoi(sm[2]);
          task.stdout_.SetDelayedPipe(line_number);
          if (sm[1] == '!') {
            task.stderr_.SetDelayedPipe(line_number);
          }
        }
      } else if (sm[3].length() > 0) {
        // if x == "|"
        task.stdout_.SetDelayedPipe(0);
      } else if (sm[4].length() > 0) {
        // if x == "> FILE"
        task.stdout_.SetFile(sm[5]);
      } else {
        // others
        task.argv_.push_back(sm[5]);
      }

      s = sm.suffix().str();
      if (utils::trim(s).size() == 0) {
        break;
      }
    }
    this->parsed_commands_.push_back(task);
  }
}
}  // namespace shell
}  // namespace np
