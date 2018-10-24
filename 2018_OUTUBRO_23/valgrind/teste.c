/*

Uso de malloc, e detecção com uso de valgind 
destas operações.

*/

#include <stdlib.h>
#include <stdio.h>

int main()
{
	float *y = malloc(sizeof(float) * 15);
	y[13] = 10.4;
	printf("\n%f\t\t\n", y[13]);
	printf("\n%f\t\t\n", y[12]);

	int *z = malloc(sizeof(float) * 20);

	z[10] = 10.4;
	printf("\n%i\t\n", z[10]);

	int *x = malloc(sizeof(int) * 20);
	x[10] = 10.4;
	printf("\n%i\t\n", x[10]);

	free(y);
	free(x);
	free(z);
    return 0;
}
