#include <stdio.h>
#include <string.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * This is a poorly implemented `cat` like program that for some reason also
 * ships dead code, the function zombie(), that is never called. Or is it?
 */

void zombie() {
	printf("I shouldn't live!\n");
}

void cat() {
	char text[32];
	int i;

	// puts a well defined pattern into the text buffer
	for (i = 0; i < sizeof(text); i++) {
		text[i] = i;
	}

	while (1) {
		scanf("%[^\n]", text);
		// eat the newline, act on possible EOF condition
		if (getchar() == EOF)
			break;
		printf("%s\n", text);
	}
}

int main() {
	cat();

	return 0;
}
