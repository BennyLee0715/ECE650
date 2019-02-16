#include <cstdio>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define BACKLOG 100
#define BASE_PORT 30000

class Potato {
public:
  int hops;
  std::vector<int> path;

  Potato(int n = 0) : hops(n) {}
};

class Server {
public:
  std::string _port;
  int status;

  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd; // fd for socket
  int new_fd; // fd after accept

  int accept_connection() {
    // now accept an incoming connection:
    addr_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    return new_fd;
  }

  void buildServer(char *_port) {
    this->_port = _port;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    // load info for server
    getaddrinfo(NULL, _port, &hints, &res);

    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    freeaddrinfo(res); /* No longer needed */
  }

  virtual ~Server() {
    close(sockfd);
  }
};