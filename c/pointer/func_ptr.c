#include <stdio.h>

int max(int a, int b) {
	if (a > b) {
		return a;
	}
	return b;
}

int main() {
	int x, y, maxval;
	int (*pmax)(int, int) = max;
	printf("请输入两个数字: ");
	scanf("%d %d", &x, &y);
	maxval = (*pmax)(x, y);
	printf("较大的数是: %d\n", maxval);
	
	return 0;
}