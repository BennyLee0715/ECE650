#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 100

class Potato {
public:
  int hops;
  int cnt;
  int path[512];
  Potato() : hops(0), cnt(0) {
    memset(path, 0, sizeof(path));
  }

  void print() {
    printf("Trace of potato:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d%c", path[i], i == cnt - 1 ? '\n' : ',');
    }
  }
};

class MetaInfo {
public:
  char addr[100];
  int port;
  MetaInfo() : port(-1) {
    memset(addr, 0, sizeof(addr));
  }
};

class Server {
public:
  struct addrinfo host_info, *host_info_list;
  int sockfd; // fd for socket
  int new_fd; // fd after accept
  int status;

  void init(const char *_port); //  load up address structs with getaddrinfo():
  void set_sin_port(); // only apply to build server with system assigned port
  void create_socket();
  int get_port(); // only apply to build server with system assigned port

  // return value: system assigened port id
  int buildServer();             // player
  void buildServer(char *_port); // master

  // return value: new_fd
  int accept_connection(std::string &ip);

  virtual ~Server() {
    close(sockfd);
  }
};

void Server::init(const char *_port) {
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, _port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Server::set_sin_port() {
  // ask os to assign a port
  struct sockaddr_in *addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
  addr_in->sin_port = 0;
}

void Server::create_socket() {
  sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol);
  if (sockfd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    exit(EXIT_FAILURE);
  } // if

  int yes = 1;
  status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    exit(EXIT_FAILURE);
  } // if

  status = listen(sockfd, BACKLOG);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    exit(EXIT_FAILURE);
  } // if

  freeaddrinfo(host_info_list);
}

int Server::get_port() {
  // get OS assigned port
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname error");
  //  std::cout << "Waiting for connection on port " << ntohs(sin.sin_port)
  //<< std::endl;
  return ntohs(sin.sin_port);
}

int Server::buildServer() { // no port specified
  init("");                 // system assigened port
  set_sin_port();
  create_socket();
  int _port = get_port();
  return _port;
}

void Server::buildServer(char *port) {
  init(port);
  create_socket();
}

int Server::accept_connection(std::string &ip) {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  new_fd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (new_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    exit(EXIT_FAILURE);
  } // if
  struct sockaddr_in *temp = (struct sockaddr_in *)&socket_addr;
  ip = inet_ntoa(temp->sin_addr);
  return new_fd;
}
