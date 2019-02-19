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
      
      // recv listening port of player i
      recv(fd[i], &port[i], sizeof(port[i]), MSG_WAITALL);
      std::cout << "Player " << i << " is ready to play\n";
    }
  }

  // build connections between players
  void build_circle() {
    for (int i = 0; i < num_players; i++) {
      MetaInfo meta_info;
      memset(&meta_info, 0, sizeof(meta_info));
      int next_id = (i + 1) % num_players;
      meta_info.port = port[next_id];
      strcpy(meta_info.addr, ip[next_id]);
      send(fd[i], &meta_info, sizeof(meta_info), 0);
    }
  }


  void playPotato() {
    Potato potato;
    potato.hops = num_hops;
    if (num_hops == 0) { // end game
      for (int i = 0; i < num_players; i++) {
        if (send(fd[i], &potato, sizeof(potato), 0) != sizeof(potato)) {
          perror("Send a broken potato\n");
        }
      }
      return;
    }

    // send potato out
    int random = rand() % num_players;
    printf("Ready to start the game, sending potato to player %d\n", random);
    if (send(fd[random], &potato, sizeof(potato), 0) != sizeof(potato)) {
      perror("Send a broken potato\n");
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < num_players; i++) {
      FD_SET(fd[i], &rfds);
    }
    int nfds = new_fd + 1;
    int ret = select(nfds, &rfds, NULL, NULL, NULL);
    assert(ret == 1);
    for (int i = 0; i < num_players; i++) {
      if (FD_ISSET(fd[i], &rfds)) {
        int len = 0;
        if ((len = recv(fd[i], &potato, sizeof(potato), MSG_WAITALL)) != sizeof(potato)) {
          printf("Received a wired potato whose length is %d\n", len);
          perror("Received an broken potato:\n");
        }

        for (int j = 0; j < num_players; j++) {
          if (send(fd[j], &potato, sizeof(potato), 0) != sizeof(potato)) {
            perror("broken");
          }
        }
        break;
      }
    }

    potato.print();
    sleep(1);
  }

  void test_block() {
    // test fds
    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < num_players; i++) {
      FD_SET(fd[i], &rfds);
    }
    int nfds = new_fd + 1;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int ret = select(nfds, &rfds, NULL, NULL, &tv);

    if (ret > 0) {
      for (int i = 0; i < num_players; i++) {
        if (FD_ISSET(fd[i], &rfds)) {
          printf("[ERROR]received data from player %d\n", i);
        }
      }
    }
    else if (ret == 0) {
      printf("[good]Blocked 5s successfully\n");
    }
  }
  
  void run() {
    print_init();
    puts("[Steo 1] SUCCESS: become a server");
    build_connections();
    // test_block();
    build_circle();
    // test_block();
    puts("[Step 2] SUCCESS: all players connected");
    playPotato();
    puts("[Step 3] Potato got back");
  }
};

int main(int argc, char **argv) {
  Ringmaster *ringmaster = new Ringmaster(argv);
  ringmaster->run();
  delete ringmaster;
  return EXIT_SUCCESS;
}