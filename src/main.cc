#include <arpa/inet.h>
#include <netinet/in.h>
#include <np/shell/shell.h>
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
  if (argc < 2) {
    throw runtime_error("invalid argument: no port specified");
  }
  int sockfd = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    throw runtime_error("failed to create socket");
  }

  // socket的連線
  struct sockaddr_in server_info, client_info;

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

  while (true) {
    int addrlen = sizeof(client_info);

    int clientfd =
        accept(sockfd, (struct sockaddr*)&client_info, (socklen_t*)&addrlen);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_info.sin_addr), ip, INET_ADDRSTRLEN);
    cout << "Connection from " << ip << " " << client_info.sin_port << endl;
    fflush(stdout);

    pid_t proc = fork();
    if (proc == 0) {
      dup2(clientfd, STDOUT_FILENO);
      dup2(clientfd, STDERR_FILENO);
      dup2(clientfd, STDIN_FILENO);

      close(clientfd);
      np::shell::Shell shell;
      shell.Run();
    } else {
      int status;
      close(clientfd);
      waitpid(proc, &status, 0);
      cout << "Connection closed" << endl;
    }
  }
  return 0;
}
