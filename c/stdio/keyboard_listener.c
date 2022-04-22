#include <stdio.h>
#include <conio.h>

int main() {
	char ch;
	int i = 0;
	
	while (ch = getch()) {
		if (ch == 27) { // ESCÔòÍË³ö
			break;
		} else {
			printf("Number: %d\n", ++i);
		}
	}
	return 0;
}