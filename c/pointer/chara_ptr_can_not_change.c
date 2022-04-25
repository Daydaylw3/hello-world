#include <stdio.h>

int main() {
	char *str = "Hello World";
	str = "Debug World";
	printf("%s\n", str);
	
	str[3] = 'B'; // 可以更改指针变量本身的指向; 不能修改字符串中的字符。
	
	printf("%s\n", str);
	
	return 0;
}