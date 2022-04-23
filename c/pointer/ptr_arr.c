#include <stdio.h>

int main() {
	int a = 1;
	int b = 2;
	int c = 3;
	int d = 4;
	int e = 5;
	int *p[5] = {&a, &b, &c, &d, &e};

	int i;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *p[i]);
	}

	return 0;
}