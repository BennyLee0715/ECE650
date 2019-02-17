#include "assert.h"
#include "potato.h"
#include <algorithm>
#include <cstdlib>
class Player : public Server {
public:
  int player_id;
  int num_players;

  // As a client of master
  int fd_master;

  // int new_fd; // As a server (included in base class)

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

    if (getaddrinfo(hostname, port, &host_info, &host_info_list)) {
      perror("getaddrinfo: ");
    }
    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
    }
    int s =
        connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (s) {
      std::cerr << "Error: cannot connect to socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      perror("Perror Msg: ");
    }
    // while (
    //     connect(socket_fd, host_info_list->ai_addr,
    //     host_info_list->ai_addrlen))

    // {
    //   perror("Error: cannot connect to socket ");
    //   std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // }
    printf("Connected to %s at %s\n", hostname, port);

    freeaddrinfo(host_info_list);
  }

  void connectMaster(const char *hostname, const char *port) {
    connectServer(hostname, port, fd_master);

    // receive player_id
    recv(fd_master, &player_id, sizeof(player_id), 0);

    // receive num_players
    recv(fd_master, &num_players, sizeof(num_players), 0);

    // start as a server
    int listeningPort = buildServer();
    send(fd_master, &listeningPort, sizeof(listeningPort), 0);
    printf("[Debug] Listening at %d\n", listeningPort);

    printf("Connected as player %d out of %d total players\n", player_id,
           num_players);
  }

  void connectNeigh() {
    MetaInfo metaInfo;
    for (int i = 0; i < 2; i++) {
      printf("Waiting for connect message from master\n");
      recv(fd_master, &metaInfo, sizeof(metaInfo), 0);
      printf("Recv msg from master : %s\n",
             metaInfo.op == 0 ? "connect" : "accept");
      if (metaInfo.op == 0) { // connect
        char port_id[9];
        sprintf(port_id, "%d", metaInfo.port);
        connectServer(metaInfo.addr, port_id, fd_neigh);
      }
      else { // accept
        std::string host_ip;
        accept_connection(host_ip);
        int sig = 1;
        send(fd_master, &sig, sizeof(sig), 0);
      }
    }
  }

  void stayListening() {
    srand((unsigned int)time(NULL) + player_id);
    while (1) {
      puts("----");
      fd_set rfds;
      FD_ZERO(&rfds);
      int _fd[] = {fd_master, fd_neigh, new_fd}; // master, right, left
      int mx_fd = 0;
      for (int i = 0; i < 3; i++) {
        FD_SET(_fd[i], &rfds);
        mx_fd = std::max(mx_fd, _fd[i]);
      }
      int ret = select(mx_fd + 1, &rfds, NULL, NULL, NULL);
      Potato potato;
      for (int i = 0; i < 3; i++) {
        if (FD_ISSET(_fd[i], &rfds)) {
          recv(_fd[i], &potato, sizeof(potato), 0);
          if (potato.hops == 0) return;
          printf("recv from %d\n", i);
          break;
        }
      }

      printf("ret = %d\n", ret);
      printf("I’m it\n");

      potato.hops--;
      potato.path[potato.tot++] = player_id;
      printf("This is the %dst hop, the rest hops: %d\n", potato.tot,
             potato.hops);

      // reach # of hops
      if (potato.hops == 0) {
        send(fd_master, &potato, sizeof(potato), 0);
        return;
      }

      int dir = rand() % 2;
      if (dir == 0) {
        printf("Sending potato to %d\n",
               (player_id - 1 + num_players) % num_players);
        send(new_fd, &potato, sizeof(potato), 0);
      }
      else {
        printf("Sending potato to %d\n", (player_id + 1) % num_players);
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