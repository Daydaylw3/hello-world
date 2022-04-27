#include <stdio.h>

// 这里 n 的声明一定要在 a[n] 前
int sum_array(int n, int a[n]) {
	int i, sum = 0;
	for (i = 0; i < n; i++) {
		sum += a[i];
	}
	return sum;
}

int main() {
	int i, n = 0;
	printf("请输入数组的长度: ");
	scanf("%d", &n);
	if (n < 0) {
		return -1;
	}
	int a[n];
	for (i = 0; i < n; i++) {
		a[i] = i+1;
	}
	printf("sum_array = %d\n", sum_array(n, a));
	return 0;
}