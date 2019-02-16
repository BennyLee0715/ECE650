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
  }

  void connectMaster(const char *hostname, const char *port) {
    connectServer(hostname, port, fd_master);

    // send my IP to master
    char local_hostname[256];
    gethostname(local_hostname, 256);                    // 127.0.0.1
    struct hostent *ret = gethostbyname(local_hostname); // vcm-8126.vm.duke.edu
    size_t len = strlen(ret->h_name);
    send(fd_master, &len, sizeof(len), 0);
    send(fd_master, ret->h_name, len, 0);

    // receive my port to listen
    size_t temp;
    recv(fd_master, &temp, sizeof(temp), 0);
    char port_id[9];
    sprintf(port_id, "%zd", temp);
    _port = port_id;

    // Set id according to port
    player_id = temp - BASE_PORT + 1;

    // start as a server
    buildServer((char *)_port.c_str());
    std::cout << "Player " << player_id << " is ready to play\n";
  }

  void connectNeigh() {
    size_t len;
    char buffer[512];
    recv(fd_master, &len, sizeof(len), 0);
    recv(fd_master, buffer, len, 0);
    buffer[len] = 0;

    size_t _port_id;
    recv(fd_master, &_port_id, sizeof(_port_id), 0);

    char port_id[9];
    sprintf(port_id, "%zd", _port_id);

    std::cout << "Connecting to " << buffer << " at " << _port_id << "\n";
    connectServer(buffer, port_id, fd_neigh);
  }

  void run() {
    connectNeigh();
  }

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
