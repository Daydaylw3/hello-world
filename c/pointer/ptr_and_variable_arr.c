void f(int n) {
	int a[n], *p;
	p = a;
}

// 如果数组是多维的, 指针的类型取决于除第一维外每一维的长度
// 下面是二维的情况
void f2(int m, int n) {
	int a[m][n], (*p)[n];
	p = a;
}

// 以下的代码可以通过编译, 但是如果m≠n, 后续对p的使用都将导致未定义的行为
void f3(int m, int n) {
	int a[m][n], (*p)[m];
	p = a;
}

// 这个只是为了方便编译
int main() {
	
	return 0;
}