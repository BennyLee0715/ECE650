#include "assert.h"
#include "potato.h"
#include <algorithm>

class Player : public Server {
public:
  int player_id;
  int num_players;

  int fd_master; // master
  // int new_fd; // player_id - 1
  int fd_neigh; // player_id + 1

  Player(char **argv) {
    connectMaster(argv[1], argv[2]);
  }

  virtual ~Player() {
    close(new_fd);
    close(fd_master);
    close(fd_neigh);
  }

  void connectServer(const char *hostname, const char *port, int &socket_fd) {

    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(struct addrinfo));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      std::cerr << "Error: cannot get address info for host" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      perror("[MYF]error exit -1\n");
      exit(EXIT_FAILURE);
    } // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      exit(EXIT_FAILURE);
    } // if

    status =
        connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot connect to socket" << std::endl;
      std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
      perror("[MYF]error exit -1\n");
      exit(EXIT_FAILURE);
    } // if

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
    std::cout << "Connected as player " << player_id << " out of "
              << num_players << " total players\n";
  }

  // connect first, then accept
  void connectNeigh() {
    MetaInfo meta_info;
    recv(fd_master, &meta_info, sizeof(meta_info), MSG_WAITALL);

    char port_id[9];
    sprintf(port_id, "%d", meta_info.port);
    connectServer(meta_info.addr, port_id, fd_neigh);
    accept_connection(NULL);
  }

  void stayListening() {
    srand((unsigned int)time(NULL) + player_id);
    Potato potato;
    fd_set rfds;
    int fd[] = {new_fd, fd_neigh, fd_master};
    int nfds = 1 + (new_fd > fd_neigh ? new_fd : fd_neigh);
    while (1) {
      memset(&potato, 0, sizeof(potato));
      FD_ZERO(&rfds);
      for (int i = 0; i < 3; i++)
        FD_SET(fd[i], &rfds);
      int ret = select(nfds, &rfds, NULL, NULL, NULL);
      assert(ret == 1);
      for (int i = 0; i < 3; i++) {
        if (FD_ISSET(fd[i], &rfds)) {
          if ((status = recv(fd[i], &potato, sizeof(potato), MSG_WAITALL)) !=
              sizeof(potato)) {
            printf("Received a broken potato whose length is %d\n", status);
            perror("");
          }
          break;
        }
      }

      if (potato.hops == 0) return;

      potato.hops--;
      potato.path[potato.cnt++] = player_id;
      if (potato.hops == 0) {
        if (send(fd_master, &potato, sizeof(potato), 0) != sizeof(potato)) {
          std::cerr << "Send error\n";
        }
        std::cout << "I'm it\n";
        continue;
      }

      int random = rand() % 2;
      printf("Sending potato to %d\n",
             random == 0 ? ((player_id - 1 + num_players) % num_players)
                         : (player_id + 1) % num_players);
      if (send(fd[random], &potato, sizeof(potato), 0) != sizeof(potato)) {
        printf("Send error\n");
      }
    }
  }

  void run() {
    connectNeigh();
    stayListening();
    sleep(1);
  }
};

int main(int argc, char **argv) {
  Player *player = new Player(argv);
  player->run();
  delete player;
  return EXIT_SUCCESS;
}