#include <arpa/inet.h>
#include <netinet/in.h>
#include <np/shell/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <memory>
using namespace std;

int main(int argc, char** argv, char** envp) {
  signal(SIGCHLD, [](int signo) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
      ;
  });

  if (argc < 2) {
    throw runtime_error("invalid argument: no port specified");
  }
  int sockfd = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    throw runtime_error("failed to create socket");
  }

  // socket的連線
  struct sockaddr_in server_info;

  bzero(&server_info, sizeof(server_info));
  server_info.sin_family = PF_INET;
  server_info.sin_addr.s_addr = INADDR_ANY;
  server_info.sin_port = htons(atoi(argv[1]));

  int flag = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) {
    throw runtime_error("setsockopt(SO_REUSEADDR) failed");
  }

  bind(sockfd, (struct sockaddr*)&server_info, sizeof(server_info));
  listen(sockfd, 5);

  // fd_set readfds, sockfds;
  // FD_ZERO(readfds);
  // FD_ZERO(sockfds);
  // FD_SET(sockfd, &sockfds);
  np::shell::ShellConsole shell_console(sockfd, sockfd);
  shell_console.Run();
  return 0;
}
