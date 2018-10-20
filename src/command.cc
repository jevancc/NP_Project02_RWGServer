#include <npshell/types.h>
#include <npshell/utils.h>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
using namespace std;

namespace np {
Command::Command(string command) {
    static regex pipe_regex(R"([^|!]+((\|\d+|!\d+)\s*$|\||$))");
    static regex args_regex(R"((\||!)(\d+)?|(>)?\s*(\S+))");
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

    for (auto s: commands) {
        string exec;
        vector<string> args;
        vector<IOOption> io_options(3);
        bool is_arg = false;

        utils::trim(s);
        while (regex_search(s, sm, args_regex)) {
            if (sm[1].length() > 0) {
                // if x == "|\d+" or "!\d+" or "|"
                if (sm[2].length() > 0) {
                    // x == "|\d+" or "!\d
                    int line_number = stoi(sm[2]);
                    io_options[1].type = IO::kPipe;
                    io_options[1].line = line_number;
                    if (sm[1] == '!') {
                        io_options[2].type = IO::kPipe;
                        io_options[2].line = line_number;
                    }
                } else {
                    // x == "|"
                    io_options[1].type = IO::kPipe;
                    io_options[1].line = 0;
                }
            } else if(sm[3].length() > 0) {
                // if x == "> FILE"
                io_options[1].type = IO::kFile;
                io_options[1].file = sm[4];
            } else {
                // others
                if (!is_arg) {
                    exec = sm[4];
                    is_arg = true;
                } else {
                    args.push_back(sm[4]);
                }
            }


            s = sm.suffix().str();
            if (utils::trim(s).size() == 0) {
                break;
            }
        }
        this->parsed_commands_.push_back(make_tuple(
            exec,
            args,
            io_options
        ));
    }
}

}  // namespace np
