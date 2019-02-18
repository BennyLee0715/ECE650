#include "potato.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

class Ringmaster : public Server {
public:
  int num_players;
  int num_hops;
  std::vector<int> fd;
  std::vector<std::string> client_info; // ip
  std::vector<int> port;

  Ringmaster(char **arg) {
    num_players = atoi(arg[2]);
    num_hops = atoi(arg[3]);
    fd.resize(num_players);
    client_info.resize(num_players);
    port.resize(num_players);
    this->buildServer(arg[1]);
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
      fd[i] = this->accept_connection(client_info[i]);

      // send player_id
      send(fd[i], &i, sizeof(i), 0);

      // send num_players
      send(fd[i], &num_players, sizeof(num_players), 0);

      // recv port id
      recv(fd[i], &port[i], sizeof(port[i]), 0);

      std::cout << "Player " << i << " is ready to play\n";
    }
  }

  void build_circle() {
    for (int i = 0; i < num_players; i++) {
      std::cout << "asking player " << i << " to connect with " << (i + 1)
                << "\n";

      int next_id = (i + 1) % num_players;
      MetaInfo metaInfo;
      strcpy(metaInfo.addr, client_info[next_id].c_str());
      metaInfo.port = port[next_id];
      send(fd[i], &metaInfo, sizeof(metaInfo), 0);
    }
    // validate
    for (int i = 0; i < num_players; i++) {
      int temp;
      recv(fd[i], &temp, sizeof(temp), 0);
    }
  }

  void sendPotato() {
    int random = rand() % num_players;
    Potato potato;
    potato.hops = num_hops;

    printf("Ready to start the game, sending potato to player %d\n", random);
    // send potato out
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
    for (int i = 0; i < num_players; i++) {
      FD_SET(fd[i], &rfds);
    }

    Potato potato;

    select(new_fd + 1, &rfds, NULL, NULL, NULL);
    for (int i = 0; i < num_players; i++) {
      if (FD_ISSET(fd[i], &rfds)) {
        recv(fd[i], &potato, sizeof(potato), 0);

        // ask all socket to close
        for (int j = 0; j < num_players; j++) {
          if (j == i) continue;
          send(fd[j], &potato, sizeof(potato), 0);
        }
      }
    }

    printf("The game should have finished: %d\n", potato.hops);

    // print path
    printPotato(potato);
  }

  void run() {
    print_init();
    build_connections();
    puts("Start build circle");
    build_circle();
    puts("Circle built successfully");
    for (int i = 0; i < num_players; i++) {
      std::cout << "Player " << i << ": " << client_info[i] << " at " << port[i]
                << "\n";
    }
    sendPotato();
    receivePotato();
  }
  virtual ~Ringmaster() {
    for (int i = 0; i < fd.size(); i++) {
      close(fd[i]);
    }
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  return EXIT_SUCCESS;
}