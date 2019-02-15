#include "potato.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

class Ringmaster : public Server {
public:
  size_t num_players;
  size_t num_hops;
  std::vector<int> fd;

  Ringmaster(char **arg) {
    this->buildServer(arg[1]);
    num_players = atoi(arg[2]);
    num_hops = atoi(arg[3]);
    fd.resize(num_players);
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
      int fd_i = this->accept_connection();
      fd[i] = fd_i;
      const char *message = "hi there, this is client!";
      char buffer[512];
      recv(fd_i, buffer, 9, 0);
      std::cout << "Server received: " << buffer << std::endl;
      send(fd_i, message, strlen(message), 0);
    }
  }
  void build_circle() {}
  void run() {
    print_init();
    build_connections();
    build_circle();
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  return EXIT_SUCCESS;
}