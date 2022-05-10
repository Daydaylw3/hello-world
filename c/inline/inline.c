#include <stdio.h>
#include "average.h"

int main(void) {
	double a, b;

	scanf("%lf %lf", &a, &b);
	printf("%lf\n", average(a, b));

	return 0;
}