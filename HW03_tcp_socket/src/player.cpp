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
    close(fd_master);
    close(fd_neigh);
    close(new_fd);
  }

  void connectServer(const char *hostname, const char *port, int &socket_fd) {
    int status;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(struct addrinfo));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      cerr << "Error: cannot get address info for host" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      perror("[MYF]error exit -1\n");
      exit(EXIT_FAILURE);
    } // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      perror("[MYF]error exit -1\n");
      exit(EXIT_FAILURE);
    } // if

    cout << "Connecting to " << hostname << " on port " << port << "..."
         << endl;

    status =
        connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      cerr << "Error: cannot connect to socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
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
    int listeningPort = BASE_PORT + player_id;
    meta_info_t meta_info;
    meta_info.port = listeningPort;
    gethostname(meta_info.addr, 100);

    char _port[10];
    sprintf(_port, "%d", listeningPort);
    buildServer(_port);

    send(fd_master, &meta_info, sizeof(meta_info_t), 0);

    printf("Connected as player %d out of %d total players\n", player_id,
           num_players);
  }

  void connectNeigh() {
    // connect first, then accept
    meta_info_t meta_info;
    printf("Waiting for connect message from master\n");
    recv(fd_master, &meta_info, sizeof(meta_info_t), 0);
    char port_id[9];
    sprintf(port_id, "%d", meta_info.port);
    connectServer(meta_info.addr, port_id, fd_neigh);
    char *ip;
    accept_connection(&ip);
    printf("Accepted connection from %s, which should be player_id - 1\n", ip);
    free(ip);
  }

  void stayListening() {
    srand((unsigned int)time(NULL) + player_id);
    potato_t potato;
    fd_set rfds;
    int fd[] = {new_fd, fd_neigh, fd_master};
    int nfds = 1 + (new_fd > fd_neigh ? new_fd : fd_neigh);
    while (1) {
      puts("-----");
      FD_ZERO(&rfds);
      for (int i = 0; i < 3; i++)
        FD_SET(fd[i], &rfds);
      int ret = select(nfds, &rfds, NULL, NULL, NULL);
      // assert(ret == 1);
      int fd_temp = -1;
      for (int i = 0; i < 3; i++) {
        if (FD_ISSET(fd[i], &rfds)) {
          fd_temp = fd[i];
          printf("Received potato from %s\n",
                 i == 0 ? "left" : (i == 1 ? "right" : "master"));
          break;
        }
      }
      recv(fd_temp, &potato, sizeof(potato_t), 0);
      if (potato.terminate) return;

      potato.hops--;
      potato.path[potato.tot++] = player_id;
      printf("This is the %dth hop\n", potato.tot);
      if (potato.hops == 0) {
        send(fd_master, &potato, sizeof(potato_t), 0);
        printf("Sending to master\n");
        printf("I'm it\n");
        continue;
      }

      int random = rand() % 2;
      printf("Sending potato to %d\n",
             random == 0 ? ((player_id - 1 + num_players) % num_players)
                         : (player_id + 1) % num_players);
      send(fd[random], &potato, sizeof(potato_t), 0);
      printf("Sending to %s\n", random == 0 ? "left" : "right");
    }
  }

  void run() {
    connectNeigh();
    printf("fd_master: %d, fd_neigh: %d, new_fd: %d\n", fd_master, fd_neigh,
           new_fd);
    stayListening();
  }
};

int main(int argc, char **argv) {
  Player *player = new Player(argv);
  player->run();
  delete player;
  return EXIT_SUCCESS;
}