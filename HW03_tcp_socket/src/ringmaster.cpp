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
      int fd_i = this->accept_connection();
      fd[i] = fd_i;
      char buffer[512];

      // Receive IP of each player
      size_t len;
      recv(fd_i, &len, sizeof(len), 0);
      recv(fd_i, buffer, len, 0);
      buffer[len] = 0; // null terminator
      client_info[i] = buffer;

      printf("Server received : %s\n", client_info[i].c_str());

      // Send Port no of each player
      size_t listening_port = BASE_PORT + i;
      send(fd_i, &listening_port, sizeof(size_t), 0);
    }
  }

  void build_circle() {
    for (size_t i = 0; i < num_players; i++) {
      size_t next_id = (i + 1) % num_players;
      size_t len = client_info[next_id].length();
      send(fd[i], &len, sizeof(len), 0);
      send(fd[i], (char *)client_info[next_id].c_str(), len, 0);
      size_t port = BASE_PORT + next_id;
      send(fd[i], &port, sizeof(port), 0);
    }
  }

  void run() {
    print_init();
    build_connections();
    std::cout << "Connections between player and server established\n";
    build_circle();
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  return EXIT_SUCCESS;
}
