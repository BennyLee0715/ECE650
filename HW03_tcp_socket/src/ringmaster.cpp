#include "potato.h"
#include <iostream>

class Ringmaster : public Server {
public:
  int num_players;
  int num_hops;
  int *fd;
  char **ip;
  int *port;

  Ringmaster(char **arg) {
    num_players = atoi(arg[2]);
    num_hops = atoi(arg[3]);
    ip = (char **)malloc(num_players * sizeof(*ip));
    fd = (int *)malloc(num_players * sizeof(*fd));
    port = (int *)malloc(num_players * sizeof(*port));

    this->buildServer(arg[1]);
  }

  virtual ~Ringmaster() {
    for (int i = 0; i < num_players; i++) {
      close(fd[i]);
      free(ip[i]);
    }
    free(ip);
    free(fd);
    free(port);
  }

  void print_init() {
    std::cout << "Potato Ringmaster\n";
    std::cout << "Players = " << num_players << "\n";
    std::cout << "Hops = " << num_hops << "\n";
  }

  // build all connection between master and player
  // Server: master, Client: player
  void build_connections() {
    for (int i = 0; i < num_players; i++) {
      fd[i] = accept_connection(&ip[i]);

      // send player_id
      send(fd[i], &i, sizeof(i), 0);

      // send num_players
      send(fd[i], &num_players, sizeof(num_players), 0);

      // recv port id
      recv(fd[i], &port[i], sizeof(port[i]), 0);
      printf("[Debug] Player %d listen at %s:%d\n", i, ip[i], port[i]);
      std::cout << "Player " << i << " is ready to play\n";
    }
  }

  // build connections between players
  void build_circle() {
    for (int i = 0; i < num_players; i++) {
      meta_info_t meta_info;
      memset(&meta_info, 0, sizeof(meta_info_t));
      int next_id = (i + 1) % num_players;
      meta_info.port = port[next_id];
      strcpy(meta_info.addr, ip[next_id]);
      send(fd[i], &meta_info, sizeof(meta_info_t), 0);
    }
  }

  void printPotato(potato_t &potato) {
    printf("Trace of potato:\n");
    for (int i = 0; i < potato.tot; i++) {
      printf("%d%c", potato.path[i], i == potato.tot - 1 ? '\n' : ',');
    }
  }

  void sendPotato() {
    potato_t potato;
    memset(&potato, 0, sizeof(potato_t));
    potato.hops = num_hops;
    if (num_hops == 0) { // end game
      for (int i = 0; i < num_players; i++) {
        send(fd[i], &potato, sizeof(potato_t), 0);
      }
      return;
    }

    // send potato out
    int random = rand() % num_players;
    printf("Ready to start the game, sending potato to player %d\n", random);
    send(fd[random], &potato, sizeof(potato_t), 0);

    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < num_players; i++) {
      FD_SET(fd[i], &rfds);
    }
    int nfds = new_fd + 1;
    int ret = select(nfds, &rfds, NULL, NULL, NULL);
    if (ret == -1) {
      perror("select()");
    }
    assert(ret == 1);
    for (int i = 0; i < num_players; i++) {
      if (FD_ISSET(fd[i], &rfds)) {
        recv(fd[i], &potato, sizeof(potato_t), 0);
        potato.terminate = 1;
        for (int j = 0; j < num_players; j++) {
          send(fd[j], &potato, sizeof(potato_t), 0);
        }
        break;
      }
    }
    printPotato(potato);
    sleep(1);
  }

  void run() {
    print_init();
    build_connections();
    puts("Start build circle");
    build_circle();
    puts("Circle built successfully");
    sendPotato();
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  delete ringmaster;
  return EXIT_SUCCESS;
}