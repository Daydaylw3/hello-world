#include <stdio.h>

int main() {
	#if defined(WIN32)
	printf("Win32\n");
	#elif defined(MAC_OS)
	printf("MacOS\n");
	#elif defined(LINUX)
	printf("Linux\n");
	#endif
	
	return 0;
}