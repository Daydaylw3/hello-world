#include <stdio.h>

#define PRINT_INT(n) printf(#n " = %d\n", n)

int main() {
	int i = 5, j = 3;
	PRINT_INT(i/j); // 被替换成 printf("i/j" " = %d\n", i/j)

	return 0;
}