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

      printf("Client %zo %d: %s\n", i, fd_i, client_info[i].c_str());
    }
  }

  void build_circle() {
    /*
    for (size_t i = 0; i < num_players; i++) {
      size_t next_id = (i + 1) % num_players;
      size_t len = client_info[next_id].length();
      send(fd[i], &len, sizeof(len), 0);
      send(fd[i], (char *)client_info[next_id].c_str(), len, 0);
      size_t port = BASE_PORT + next_id;
      send(fd[i], &port, sizeof(port), 0);
      printf("Sending to client %zo: %s:%lu\n", i,
    client_info[next_id].c_str(),port);
    }
    */

    for (size_t i = 1; i < num_players; i++) {
      int op = 0;
      send(fd[i], &op, sizeof(op), 0);
    }

    for (size_t i = 0; i < num_players - 1; i++) {
      int op = 1;
      send(fd[i], &op, sizeof(op), 0);
      size_t next_id = (i + 1) % num_players;
      size_t len = client_info[next_id].length();
      send(fd[i], &len, sizeof(len), 0);
      send(fd[i], (char *)client_info[next_id].c_str(), len, 0);
      size_t port = BASE_PORT + next_id;
      send(fd[i], &port, sizeof(port), 0);
      // sleep(1);
    }

    // enable player 0 as a server
    int op = 0;
    send(fd[0], &op, sizeof(op), 0);

    op = 1;
    send(fd[num_players - 1], &op, sizeof(op), 0);
    size_t next_id = 0;
    size_t len = client_info[next_id].length();
    send(fd[num_players - 1], &len, sizeof(len), 0);
    send(fd[num_players - 1], (char *)client_info[next_id].c_str(), len, 0);
    size_t port = BASE_PORT + next_id;
    send(fd[num_players - 1], &port, sizeof(port), 0);
  }

  void sendPotato() {
    srand((unsigned int)time(NULL) + num_players);
    int random = rand() % num_players;
    Potato potato;
    potato.hops = num_hops;
    int op = 1; // potato

    // send potato out
    send(fd[random], &op, sizeof(op), 0);
    send(fd[random], &potato, sizeof(potato), 0);
  }

  void receivePotato() {
    fd_set rfds;
    struct timeval tv;
    int retval;

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
    printf("Trace of potato:\n%s\n", potato.path);
  }

  void run() {
    print_init();
    build_connections();
    std::cout << "Connections between player and server established\n";
    build_circle();
    std::cout << "Connections among players established\n";

    // sendPotato();
    // receivePotato();
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  return EXIT_SUCCESS;
}