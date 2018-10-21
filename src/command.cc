#include <npshell/types.h>
#include <npshell/utils.h>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
using namespace std;

namespace np {
Command::Command(string command) {
  static regex pipe_regex(R"([^|!]+((\|\d+|!\d+)\s*$|\||$))");
  static regex argv_regex(R"((\||!)(\d+)?|(>)?\s*(\S+))");
  this->raw_ = utils::trim(command);

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
      if (sm[1].length() > 0) {
        // if x == "|\d+" or "!\d+" or "|"
        if (sm[2].length() > 0) {
          // x == "|\d+" or "!\d
          int line_number = stoi(sm[2]);
          task.stdout_.type = IO::kPipe;
          task.stdout_.line = line_number;
          if (sm[1] == '!') {
            task.stderr_.type = IO::kPipe;
            task.stderr_.line = line_number;
          }
        } else {
          // x == "|"
          task.stdout_.type = IO::kPipe;
          task.stdout_.line = 0;
        }
      } else if (sm[3].length() > 0) {
        // if x == "> FILE"
        task.stdout_.type = IO::kFile;
        task.stdout_.file = sm[4];
      } else {
        // others
        task.argv_.push_back(sm[4]);
      }

      s = sm.suffix().str();
      if (utils::trim(s).size() == 0) {
        break;
      }
    }
    this->parsed_commands_.push_back(task);
  }
}

}  // namespace np
