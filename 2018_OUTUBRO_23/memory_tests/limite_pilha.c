#include<stdio.h>
#define T_ADRESS 32
//Variáveis Globais
int nChamadas = 0;
float capacidadeUtilizada = 0;
void fRecursiva();
 
main(){
    fRecursiva();
}
 
void fRecursiva(){
    nChamadas++;
    capacidadeUtilizada = capacidadeUtilizada + T_ADRESS;
    printf("\n%da chamada:\t %f kB da pilha foram utilizados...", nChamadas, capacidadeUtilizada/1024);
    fRecursiva();
}
