#include <stdio.h>
#include <stdlib.h>

int main() {
	// 分配可以保存30个字符的内存，并把返回的指针转换为 char *
	char *str = (char *)malloc(sizeof(char)*30);
	scanf("%s", str);
	printf("%s\n", str);
	return 0;
}