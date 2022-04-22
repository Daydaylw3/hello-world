#include <stdio.h>

#define M (n*n+3*n)

int main() {
	int sum, n;
	printf("请输入一个数字: ");
	scanf("%d", &n);
	sum = 3*M+4*M+5*M; // 12*(n*n+3*n)
	printf("sum=%d\n", sum);
	return 0;
}

