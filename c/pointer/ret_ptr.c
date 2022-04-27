#include <stdio.h>
#include <string.h>

char *strlong(char *str1, char *str2) {
	if (strlen(str1) >= strlen(str2)) {
		return str1;
	}
	return str2;
}

// 用指针作为函数返回值时需要注意的一点是，函数运行结束后会销毁在它内部定义的所有局部数据，包括局部变量、局部数组和形式参数，
// 函数返回的指针请尽量不要指向这些数据，C语言没有任何机制来保证这些数据会一直有效，它们在后续使用过程中可能会引发运行时错误。
int *func() {
	int n = 100;
	return &n;
}

int main() {
	char str1[30], str2[30], *str;
	printf("请输入字符串: ");
	scanf("%s", str1);
	printf("清再输入字符串: ");
	scanf("%s", str2);
	str = strlong(str1, str2);
	printf("更长的字符串是: %s", str);
	
	int *p = func(), n;
	n = *p;
	printf("value = %d\n", n);
	
	return 0;
}