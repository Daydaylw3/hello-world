#include <stdio.h>

#define GENERIC_MAX(type) 			\
type type##_max(type x, type y) 	\
{									\
	return (x) > (y) ? (x) : (y);	\
}									\
// 上面的宏定义被替换成下面:
// float float_max(float x, float y) { return (x) > (y) ? (x) : (y); }

GENERIC_MAX(float)

int main() {
	float i = 5.0, j = 3.0;
	printf("%f\n", float_max(i, j));

	return 0;
}