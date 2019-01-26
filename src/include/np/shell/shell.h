#ifndef _NP_SHELL_SHELL_H_
#define _NP_SHELL_SHELL_H_

#include <netinet/in.h>
#include <np/shell/builtin.h>
#include <np/shell/constants.h>
#include <np/shell/types.h>
#include <stdio.h>
#include <sys/socket.h>

namespace np {
namespace shell {

class Shell : enable_shared_from_this<Shell> {
 private:
  int sockfd_;
  char addr_[INET_ADDRSTRLEN];
  char port_[8];
  ShellConsole& console_;

  optional<weak_ptr<Pipe>> GetPipe2User_(int uid);
  bool SetPipe2User_(int uid, shared_ptr<Pipe> p);
  bool MoveChildProcesses2User_(int uid, vector<pid_t>& pids);

  int WaitDelayedChildProcesses_(int line = 0);
  int WaitUserChildProcesses_(int uid);

 public:
  friend class Task;
  friend class ShellConsole;
  friend ExecError builtin::exit(const vector<string>&, Shell&);
  friend ExecError builtin::printenv(const vector<string>&, Shell&);
  friend ExecError builtin::setenv(const vector<string>&, Shell&);
  friend ExecError builtin::name(const vector<string>&, Shell&);
  friend ExecError builtin::yell(const vector<string>&, Shell&);
  friend ExecError builtin::tell(const vector<string>&, Shell&);
  friend ExecError builtin::who(const vector<string>&, Shell&);
  Environment env;

  Shell(sockaddr_in* client_info, int sockfd, int uid, ShellConsole& console);
  Shell(const Shell&);
  Shell& operator=(const Shell&);

  ssize_t Send(const string& s) const;
  void Execute(string input);
  int GetSockfd() const { return this->sockfd_; }
  void CloseSockfd() { close(this->sockfd_); }

  ~Shell();
};
}  // namespace shell
}  // namespace np
#endif
