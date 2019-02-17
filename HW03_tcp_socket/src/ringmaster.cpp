#include "potato.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

class Ringmaster : public Server {
public:
  size_t num_players;
  size_t num_hops;
  std::vector<int> fd;
  std::vector<std::string> client_info;

  Ringmaster(char **arg) {
    this->buildServer(arg[1]);
    num_players = atoi(arg[2]);
    num_hops = atoi(arg[3]);
    fd.resize(num_players);
    client_info.resize(num_players);
  }

  void print_init() {
    std::cout << "Potato Ringmaster\n";
    std::cout << "Players = " << num_players << "\n";
    std::cout << "Hops = " << num_hops << "\n";
  }

  // build all connection between master and player
  // Server: master, Client: player
  void build_connections() {
    for (size_t i = 0; i < num_players; i++) {
      int fd_i = this->accept_connection(client_info[i]);
      fd[i] = fd_i;

      // Send Port # of each player
      size_t listening_port = BASE_PORT + i;
      send(fd_i, &listening_port, sizeof(size_t), 0);
      send(fd_i, &num_players, sizeof(&num_players), 0);
      std::cout << "Player " << i << " is ready to play\n";
    }
  }

  void build_circle() {
    for (size_t i = 0; i < num_players; i++) {
      size_t next_id = (i + 1) % num_players;
      MetaInfo metaInfo;
      strcpy(metaInfo.addr, client_info[next_id].c_str());
      metaInfo.port = BASE_PORT + next_id;
      send(fd[i], &metaInfo, sizeof(metaInfo), 0);
    }
  }

  void sendPotato() {
    int random = rand() % num_players;
    Potato potato;
    potato.hops = num_hops;
    int op = 1; // potato

    printf("Ready to start the game, sending potato to player %d\n", random);
    // send potato out
    send(fd[random], &op, sizeof(op), 0);
    send(fd[random], &potato, sizeof(potato), 0);
  }

  void printPotato(Potato &potato) {
    printf("Trace of potato:\n");
    for (int i = 0; i < potato.tot; i++) {
      printf("%d%c", potato.path[i], i == potato.tot - 1 ? '\n' : ',');
    }
  }

  void receivePotato() {
    fd_set rfds;
    FD_ZERO(&rfds);
    for (size_t i = 0; i < num_players; i++) {
      FD_SET(fd[i], &rfds);
    }

    Potato potato;

    select(new_fd + 1, &rfds, NULL, NULL, NULL);
    for (size_t i = 0; i < num_players; i++) {
      if (FD_ISSET(fd[i], &rfds)) {
        recv(fd[i], &potato, sizeof(potato), 0);
        break;
      }
    }

    // ask all socket to close
    for (size_t i = 0; i < num_players; i++) {
      int op = 0; // close signal
      send(fd[i], &op, sizeof(op), 0);
    }

    // print path
    printPotato(potato);
  }

  void run() {
    printf("%lu\n", sizeof(Potato));
    print_init();
    build_connections();
    build_circle();
    sendPotato();
    receivePotato();
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  return EXIT_SUCCESS;
}