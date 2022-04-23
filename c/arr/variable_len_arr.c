#include <stdio.h>

int main() {
	int n;
	printf("请输入数组长度: ");
	scanf("%d", &n);
	scanf("%*[^\n]"); scanf("%*c"); // 清空缓冲区
	char str[n];
	printf("请输入字符串: ");
	gets(str);
	puts(str);
	
	return 0;
}