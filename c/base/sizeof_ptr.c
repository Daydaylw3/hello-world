#include <stdio.h>

int main() {
	char *a = "12345";
	char b[] = "12345";
	printf("%d\n", sizeof(a));
	printf("%d\n", sizeof(b));
	return 0;
}