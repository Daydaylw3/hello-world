#include <stdio.h>
#include <conio.h>
#include <windows.h>

int main() {
	char ch;
	int i = 0;
	
	while (1) {
		if (kbhit()) {
			ch = getch();
			if (ch == 27) {
				break;
			}
		}
		printf("Number: %d\n", ++i);
		Sleep(1000);
	}
	return 0;
}
