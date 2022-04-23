#include <stdio.h>

int main() {
	char *p1[5] = {
		"让世界充满爱",
		"Just do it",
		"一切皆有可能",
		"为中华之崛起而读书",
		"bzzb"
	};
	int i;
	for (i = 0; i < 5; i++) {
		printf("%s\n", p1[i]);
	}
	return 0;
}