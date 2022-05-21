#include <stdio.h>

int main() {
	int a[2] = {0, 1};
	*(a+3) = 2;
	printf("%d\n", *(a+3));
	return 0;
}
