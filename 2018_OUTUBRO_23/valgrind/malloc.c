/*
Malloc do mal
*/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	if(argc<2) {
		printf("Digite ./malloc <INT> \n\n");
	}
	int num = atoi(argv[1]);
  	printf("Argv[1] = %d \n",num);

	float *y = malloc(sizeof(float) * num);
	
	y[num-1] = 10;
	printf("\n%f\t\t\n", y[num-1]);

	return 0;
}
