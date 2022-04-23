#include <stdio.h>

#define M(y) y*y+3*y
#define MAX(a,b) (a>b)?a:b

int main() {
	int k = M(5);
	printf("k=%d\n", k);
	int x, y, max;
	printf("请输入两个数字: ");
	scanf("%d %d", &x, &y);
	max = MAX(x, y);
	printf("max=%d\n", max);
	return 0;
}