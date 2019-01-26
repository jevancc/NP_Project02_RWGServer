#ifndef _NP_SHELL_SHELL_CONSOLE_H_
#define _NP_SHELL_SHELL_CONSOLE_H_

#include <netinet/in.h>
#include <np/shell/builtin.h>
#include <np/shell/constants.h>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>
using namespace std;

namespace np {
namespace shell {

class ShellConsole {
 private:
  int sockfd_;
  int maxfd_;
  fd_set sockfds_;
  vector<shared_ptr<Shell>> id2user_map_;
  map<int, shared_ptr<Shell>> fd2user_map_;
  set<string> user_names_;
  priority_queue<int, vector<int>, greater<int>> available_uids_;
  queue<shared_ptr<Shell>> garbage_shell_queue_;

  int CreateUid_();
  shared_ptr<Shell> CreateShell_(sockaddr_in* client_info, int ufd);
  void ClearGarbageShells_();

  ssize_t SendWelcomeMessage2Fd_(int fd) const;
  ssize_t SendPrompt2Fd_(int fd) const;

 public:
  friend ExecError builtin::name(const vector<string>&, Shell&);

  ShellConsole(int sockfd, int maxfd);
  ShellConsole(const ShellConsole&);
  ShellConsole& operator=(const ShellConsole&);

  vector<weak_ptr<Shell>> GetUsers() const {
    vector<weak_ptr<Shell>> users(id2user_map_.size());
    for (auto& user : this->id2user_map_) {
      users.push_back(weak_ptr<Shell>(user));
    }
    return users;
  }

  weak_ptr<Shell> GetUserByUid(int uid) const {
    return uid < int(this->id2user_map_.size()) ? this->id2user_map_[uid]
                                                : nullptr;
  }

  ssize_t Send2Uid(int uid, const string& msg) const;
  ssize_t Send2Fd(int fd, const string& msg) const;
  ssize_t Broadcast(const string& msg) const;

  void DeleteUser(int uid, int ufd);
  void Run();
};
}  // namespace shell
}  // namespace np

#include <np/shell/types.h>

#endif
