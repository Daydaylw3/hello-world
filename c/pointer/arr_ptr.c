#include <stdio.h>

int main() {
	// 这样初始化是错的
	int (*p)[5] = {1, 2, 3, 4, 5};
	int i;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *(p+i));
	}
	// 这样初始化也是错的
	int temp[5] = {1, 2, 3, 4, 5};
	int (*p2)[5] = temp;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *(p2+i));
	}
	// 这样才对, 其实关键是 printf 那儿由于 p3 指向的是
	// temp 首个元素的地址, 所以还要再次取值
	int (*p3)[5] = &temp;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *(*p3+i));
	}
	// 这样会有警告, 但是打印的是正确的
	int (*p4)[5] = temp;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *(*p4+i));
	}
	// p5 和 p3 对比一下 printf
	int (*p5)[5] = &temp;
	for (i = 0; i < 5; i++) {
		printf("%d\n", *(p5+i));
	}
	return 0;
}