#include "potato.h"
#include <cstdlib>
class Player : public Server {
public:
  int player_id;

  // As a client of master
  int fd_master;

  // As a server (included in base class)

  // As a client of neightbor
  int fd_neigh;

  Player(char **argv) {
    connectMaster(argv[1], argv[2]);
  }

  void connectServer(const char *hostname, const char *port, int &socket_fd) {
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    getaddrinfo(hostname, port, &host_info, &host_info_list);
    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    freeaddrinfo(host_info_list);
    const char *message = "hi there, this is server!";
    char buffer[512];
    send(socket_fd, message, strlen(message), 0);
    recv(socket_fd, buffer, 9, 0);
    buffer[9] = 0;
    std::cout << "Client received: " << buffer << std::endl;
  }

  void connectMaster(const char *hostname, const char *port) {
    connectServer(hostname, port, fd_master);
  }

  void connectNeigh() {}

  void run() {}
  ~Player() {
    close(fd_master);
    close(fd_neigh);
  }
};

int main(int argc, char **argv) {
  Player *player = new Player(argv);
  player->run();
  return EXIT_SUCCESS;
}