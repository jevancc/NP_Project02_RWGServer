// #include <npshell/environment.h>
#include <string>
#include <vector>

namespace builtin {
const std::vector<std::string>& commands();
bool resolve(const std::string& cmd);

// int exec(string cmd, std::vector<string> argv, Environment env);

// int exit(Environment env);
// int printenv(string name, Environment env);
// int setenv(string name, string value, Environment env);
}  // namespace builtin
