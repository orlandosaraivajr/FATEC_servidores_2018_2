#include <stdio.h>
#include <stdlib.h>
 
#define MAX_PROCESS 1000
 
int main(int argc, char** argv) {
 
    int i, new_pid;
 
    for( i = 1;  i <= MAX_PROCESS; i++ ) {
 
    new_pid = fork();
 
        if( new_pid != 0 ) {
            break;
 
            } else {
            printf("Processo FILHO [%d] gerado. (%d) Processos simultaneos\n\n", getpid(), i);
            }
 
    }
 
    // esperando eternamente
    for(;;);
 
    return (EXIT_SUCCESS);
}
