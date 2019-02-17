#include "potato.h"
#include <algorithm>
#include <cstdlib>
class Player : public Server {
public:
  int player_id;
  size_t num_players;

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
    recv(fd_master, &num_players, sizeof(num_players), 0);

    // start as a server
    buildServer((char *)_port.c_str());
    printf("Connected as player %d out of %lu total players\n", player_id,
           num_players);
  }

  void connectNeigh() {
    MetaInfo metaInfo;
    recv(fd_master, &metaInfo, sizeof(metaInfo), 0);

    char port_id[9];
    sprintf(port_id, "%lu", metaInfo.port);

    // connect to neighbor
    connectServer(metaInfo.addr, port_id, fd_neigh);
    std::string host_ip;
    accept_connection(host_ip);
  }

  void stayListening() {
    srand((unsigned int)time(NULL) + player_id);
    while (1) {
      fd_set rfds;
      FD_ZERO(&rfds);
      int fd[] = {fd_master, fd_neigh, new_fd}; // master, right, left
      int mx_fd = 0;
      for (int i = 0; i < 3; i++) {
        FD_SET(fd[i], &rfds);
        mx_fd = std::max(mx_fd, fd[i]);
      }
      select(mx_fd + 1, &rfds, NULL, NULL, NULL);
      Potato potato;
      if (FD_ISSET(fd_master, &rfds)) {
        recv(fd_master, &potato, sizeof(potato), 0);
        if (potato.hops == 0) return;
      }
      else if (FD_ISSET(fd_neigh, &rfds)) {
        recv(fd_neigh, &potato, sizeof(potato), 0);
      }
      else if (FD_ISSET(new_fd, &rfds)) {
        recv(new_fd, &potato, sizeof(potato), 0);
      }
      printf("I’m it\n");

      potato.hops--;
      potato.path[potato.tot++] = player_id;
      printf("potato.tot = %d\n", potato.tot);

      // reach # of hops
      if (potato.hops == 0) {
        send(fd_master, &potato, sizeof(potato), 0);
        continue;
      }

      int dir = rand() % 2;
      if (dir == 0) {
        printf("Sending potato to %lu\n",
               (player_id - 1 + num_players) % num_players);
        send(new_fd, &potato, sizeof(potato), 0);
      }
      else {
        printf("Sending potato to %lu\n", (player_id + 1) % num_players);
        send(fd_neigh, &potato, sizeof(potato), 0);
      }
    }
  }

  void run() {
    connectNeigh();
    printf("fd_master: %d, fd_neigh: %d, new_fd: %d\n", fd_master, fd_neigh,
           new_fd);
    stayListening();
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