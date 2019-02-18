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

    memset(&host_info, 0, sizeof(host_info));
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
    int listeningPort = buildServer();
    send(fd_master, &listeningPort, sizeof(listeningPort), 0);

    printf("Connected as player %d out of %d total players\n", player_id,
           num_players);
  }

  void connectNeigh() {
    // connect first, then accept
    meta_info_t meta_info;
    printf("Waiting for connect message from master\n");
    recv(fd_master, &meta_info, sizeof(meta_info), 0);
    char port_id[9];
    sprintf(port_id, "%d", meta_info.port);
    connectServer(meta_info.addr, port_id, fd_neigh);
    char *ip;
    accept_connection(&ip);
    printf("Accepted connection from %s, which should be player_id - 1\n", ip);
    free(ip);
  }

  void stayListening() {}

  void run() {
    connectNeigh();
    printf("fd_master: %d, fd_neigh: %d, new_fd: %d\n", fd_master, fd_neigh,
           new_fd);
    // stayListening();
  }
};

int main(int argc, char **argv) {
  Player *player = new Player(argv);
  player->run();
  delete player;
  return EXIT_SUCCESS;
}