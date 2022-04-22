#include <stdio.h>

#define SQ(y) ((y)*(y))

int main() {
	int i = 1;
	while (i <= 5) {
		printf("%d^2 = %d\n", i, SQ(i++));
	}
	// 宏调用只是简单的字符串替换，SQ(i++) 会被替换为 ((i++)*(i++))，这样每循环一次 i 的值增加 2，所以最终只循环 3 次
	return 0;
}