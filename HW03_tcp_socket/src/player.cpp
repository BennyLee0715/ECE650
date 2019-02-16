#include "potato.h"
#include <algorithm>
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

    // receive my port to listen
    size_t temp;
    recv(fd_master, &temp, sizeof(temp), 0);
    char port_id[9];
    sprintf(port_id, "%zd", temp);
    _port = port_id;

    // Set id according to port
    player_id = temp - BASE_PORT;

    // start as a server
    buildServer((char *)_port.c_str());
    std::cout << "Player " << player_id << " is ready to play\n";
  }

  void connectNeigh() {
    int op;
    for (int i = 0; i < 2; i++) {
      recv(fd_master, &op, sizeof(op), 0);
      if (op == 0) { // be a server
        std::string host_ip;
        accept_connection(host_ip);
      }
      else { // connect to neighbor
        size_t len;
        char ip_buffer[512];
        recv(fd_master, &len, sizeof(len), 0);
        recv(fd_master, ip_buffer, len, 0);
        ip_buffer[len] = 0;
        size_t _port_id;
        recv(fd_master, &_port_id, sizeof(_port_id), 0);

        // recv port of neighbor
        char port_id[9];
        sprintf(port_id, "%zd", _port_id);

        // connect to neighbor
        connectServer(ip_buffer, port_id, fd_neigh);
        std::cout << "I should connect to" << ip_buffer << " at " << _port_id
                  << "\n";
      }
    }

    /*
    int op = 0;
    while(1) {
      select(10, &rfds, NULL, NULL, NULL); // fd # never exceeds 10 in client.
      if(FD_ISSET(fd_master, &rfds)){
        // recv IP of neighbor
        size_t len;
        char ip_buffer[512];
        recv(fd_master, &len, sizeof(len), 0);
        recv(fd_master, ip_buffer, len, 0);
        ip_buffer[len] = 0;
        size_t _port_id;
        recv(fd_master, &_port_id, sizeof(_port_id), 0);

        // recv port of neighbor
        char port_id[9];
        sprintf(port_id, "%zd", _port_id);

        // connect to neighbor
        connectServer(ip_buffer, port_id, fd_neigh);
        std::cout << "Connecting to " << ip_buffer << " at " << _port_id <<
    "\n"; if((op &= 2) == 3) break;
      }
      else if(FD_ISSET(sockfd, &rfds)){
        std::string host_ip;
        accept_connection(host_ip);
        if((op &= 1) == 3) break;
      }
    }
    */
  }

  void stayListening() {
    Potato potato;
    fd_set rfds;
    FD_ZERO(&rfds);
    int fd[] = {fd_master, fd_neigh, new_fd};
    for (int i = 0; i < 3; i++) {
      FD_SET(fd[i], &rfds);
    }
    while (1) {
      select(std::max(fd_master, new_fd) + 1, &rfds, NULL, NULL, NULL);
      if (FD_ISSET(fd_master, &rfds)) {
        int op;
        recv(fd_master, &op, sizeof(op), 0);
        if (op == 0) return; // termination signal
        recv(fd_master, &potato, sizeof(potato), 0);
      }
      else if (FD_ISSET(fd_neigh, &rfds)) {
        recv(fd_neigh, &potato, sizeof(potato), 0);
      }
      else if (FD_ISSET(new_fd, &rfds)) {
        recv(new_fd, &potato, sizeof(potato), 0);
      }

      potato.hops--;
      sprintf(potato.path, "%s%d%c", potato.path, player_id,
              potato.hops == 0 ? '\n' : ',');

      if (potato.hops == 0) {
        send(fd_master, &potato, sizeof(potato), 0);
      }

      int random = rand() % 2;
      if (random) {
        send(fd_neigh, &potato, sizeof(potato), 0);
      }
      else {
        send(new_fd, &potato, sizeof(potato), 0);
      }
    }
  }

  void run() {
    connectNeigh();
    printf("fd_master: %d, fd_neigh: %d, new_fd: %d\n", fd_master, fd_neigh,
           new_fd);
    // stayListening();
  }

  ~Player() {
    close(fd_master);
    close(fd_neigh);
    close(new_fd);
  }
};

int main(int argc, char **argv) {
  Player *player = new Player(argv);
  player->run();
  return EXIT_SUCCESS;
}