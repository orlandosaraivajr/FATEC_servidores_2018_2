#include <stdlib.h>
#include <stdio.h>

int foo(num) {
	if(num < 10) {
		printf("num is less than 10\n");
	}

	float *y = malloc(sizeof(float) * num);

	y[num-1] = rand();
	printf("\n%f\t\t", y[num-1]);
	//free(y);
}

int main(int argc, char *argv[])
{
	if(argc<2) {
		printf("Digite ./memoria <INT> \n\n");
	}
	int num = atoi(argv[1]);
	printf("Argv[1] = %d \n",num);
	while(1) {
		foo(num);
	}
	return 0;
}


