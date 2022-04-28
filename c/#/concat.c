#include <stdio.h>

// 替换列表中依赖##的宏通常不能嵌套
#define CONCAT(x,y) x##y

int main() {
	int x, y, z;
	printf("%s\n", CONCAT(x, CONCAT(y, z)));
	// 会变成如下
	// printf("%s\n", xCONCAT(y, z));
	
	return 0;
}