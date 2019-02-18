#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define BACKLOG 100

class Potato {
public:
  int hops;
  int path[512];
  int tot;

  Potato() : hops(0), tot(0) {
    memset(path, 0, sizeof(path));
  }
};

class MetaInfo {
public:
  char addr[100];
  int port;
};

class Server {
public:
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd; // fd for socket
  int new_fd; // fd after accept

  int accept_connection(std::string &ip) {
    // now accept an incoming connection:
    addr_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    struct sockaddr_in *temp = (struct sockaddr_in *)&their_addr;
    ip = inet_ntoa(temp->sin_addr);
    // std::cout << "Accepting connection from " << ip << "\n";

    return new_fd;
  }

  int buildServer() { // for player
    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;

    // load info for server
    getaddrinfo("0.0.0.0", "", &hints, &res);

    // ask os to assign a port
    struct sockaddr_in *addr_in = (struct sockaddr_in *)(res->ai_addr);
    addr_in->sin_port = 0;

    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int yes = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    freeaddrinfo(res); /* No longer needed */

    // get OS assigned port
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
      perror("getsockname error");
    return ntohs(sin.sin_port);
  }

  void buildServer(char *_port) { // for master
    // this->_port = _port;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;

    // load info for server
    getaddrinfo("0.0.0.0", _port, &hints, &res);

    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int yes = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    freeaddrinfo(res); /* No longer needed */
  }

  virtual ~Server() {
    close(sockfd);
  }
};