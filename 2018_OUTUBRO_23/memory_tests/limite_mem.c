#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
 
int main (int argc, char *argv[])
{
  struct rlimit limit;
 
  if (getrlimit(RLIMIT_AS, &limit) != 0) {
    printf("getrlimit() failed with errno=%d\n", errno);
    exit(1);
  }
 
  printf("Limite default para tamanho de processo:\n");
  printf("O soft limit eh %lu MB\n", limit.rlim_cur/(1024*1024));
  printf("O hard limit eh %lu MB\n", limit.rlim_max/(1024*1024));
 
  if (getrlimit(RLIMIT_STACK, &limit) != 0) {
    printf("getrlimit() failed with errno=%d\n", errno);
    exit(1);
  }
 
  printf("\nLimite default para tamanho da area de pilha de um processo:\n");
  printf("O soft limit eh %lu MB\n", limit.rlim_cur/(1024*1024));
  printf("O hard limit eh %lu MB\n", limit.rlim_max/(1024*1024));
  printf("sizeof(int) = %d\n", sizeof(int));
  printf("sizeof(float) = %d\n", sizeof(float));
  exit(0);
}

