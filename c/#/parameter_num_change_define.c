#include <stdio.h>

// 参数个数可变的宏
#define TEST(condition, ...) ((condition)? \
	printf("Passed test: %s\n", #condition): \
	printf(__VA_ARGS__))
	
int main() {
	int vol = 3, max = 2;
	
	// 被替换成这个
	// ((vol <= max)? printf("Passed test: %s\n", "vol <= max"): printf("vol %d exceeds %d\n", vol, max));
	TEST(vol <= max, "vol %d exceeds %d\n", vol, max);
	
	return 0;
}