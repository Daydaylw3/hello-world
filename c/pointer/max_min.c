#include <stdio.h>

#define N 10

void max_min(int [], int, int *, int *);

int main() {
	printf("请输入%d个数字: ", N);
	int b[N], i, big, small;
	
	for (i = 0; i < N; i++) {
		scanf("%d", &b[i]);
	}
	
	max_min(b, N, &big, &small);
	
	printf("最大的数为: %d, 最小的数为: %d\n", big, small);
	
	return 0;
}

void max_min(int a[], int n, int *max, int *min) {
	int i;
	*max = a[0];
	*min = a[0];
	for (i = 1; i < n; i++) {
		if (a[i] > *max) {
			*max = a[i];
		}
		if (a[i] < *min) {
			*min = a[i];
		}
	}
}