#include <stdio.h>

int main() {
	// 这段程序没有语法错误，能够通过编译和链接，但当用户输入完字符串并按下回车键时就会发生错误，
	// 在 Linux 下表现为段错误（Segment Fault），在 Windows 下程序直接崩溃。如果你足够幸运，或者
	// 输入的字符串少，也可能不报错，这都是未知的。
	// char *str;
	
	// 运行程序后发现，还未等用户输入任何字符，printf() 就直接输出了(null)。我们有理由据此推断，
	// gets() 和 printf() 都对空指针做了特殊处理
	char *str = NULL;
	gets(str);
	printf("%s\n", str);
	return 0;
}