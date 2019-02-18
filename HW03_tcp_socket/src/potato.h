#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BASE_PORT 30000
using namespace std;

struct potato_tag {
  int hops;
  int path[512];
  int tot;
  int terminate;
};
typedef struct potato_tag potato_t;

struct meta_info_tag {
  char addr[100];
  int port;
};
typedef struct meta_info_tag meta_info_t;

class Server {
public:
  struct addrinfo host_info, *host_info_list;
  int sockfd; // fd for socket
  int new_fd; // fd after accept
  int status;

  // return value: port id
  int buildServer();

  void buildServer(char *_port);

  // return value: new_fd
  int accept_connection(char **ip);

  virtual ~Server() {
    close(sockfd);
  }
};

int Server::buildServer() { // no port specified
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, "", &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    exit(EXIT_FAILURE);
  } // if

  sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol);
  if (sockfd == -1) {
    cerr << "Error: cannot create socket" << endl;
    exit(EXIT_FAILURE);
  } // if

  int yes = 1;
  status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  // ask os to assign a port
  struct sockaddr_in *addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
  addr_in->sin_port = 0;

  status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    exit(EXIT_FAILURE);
  } // if

  status = listen(sockfd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    exit(EXIT_FAILURE);
  } // if

  freeaddrinfo(host_info_list);

  // get OS assigned port
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname error");
  cout << "Waiting for connection on port " << ntohs(sin.sin_port) << endl;
  return ntohs(sin.sin_port);
}

void Server::buildServer(char *port) {
  memset(&host_info, 0, sizeof(host_info));

  char *hostname = NULL;

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } // if

  sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol);
  if (sockfd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } // if

  int yes = 1;
  status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } // if

  status = listen(sockfd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } // if

  cout << "Waiting for connection on port " << port << endl;
  freeaddrinfo(host_info_list);
}

int Server::accept_connection(char **ip) {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  new_fd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (new_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  } // if
  struct sockaddr_in *temp = (struct sockaddr_in *)&socket_addr;
  *ip = strdup(inet_ntoa(temp->sin_addr));

  return new_fd;
}
