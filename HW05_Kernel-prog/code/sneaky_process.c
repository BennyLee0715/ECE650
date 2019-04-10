#include <sys/types.h>
#include <unistd.h>

int main() {
  printf(“sneaky_process pid = % d\n”, getpid());
}