#include <string.h>
#include <stdio.h>
#include "line.h"

#define MAX_LINE_LEN 60

char line[MAX_LINE_LEN+1];
int line_len = 0;
int num_words = 0;

void clear_line(void) {
	line[0] = '\0';
	line_len = 0;
	num_words = 0;
}

void add_word(const char *word) {
	if (num_words > 0) {
		line[line_len] = ' ';
		line[line_len+1] = '\0';
		line_len++;
	}
	strcat(line, word);
	line_len += strlen(word);
	num_words++;
}

int space_remaining(void) {
	return MAX_LINE_LEN - line_len;
}

void write_line(void) {
	int extra_spaces, spaces_to_insert, i, j;
	
	// 额外空格的数量存储在变量 spaces_to_insert 中，这个变量的值由
	// extra_spaces / (num_words -1) 确定，其中 extra_spaces 初始是最大行长度和当前行
	// 长度的差。因为在打印每个单词之后 extra_spaces 和 num_words 都发生变化，所以
	// spcaes_to_insert 也将变化。如果 extra_spaces 初始为10，并且 num_words 初始为5，
	// 那么第1个单词之后将有两个额外的空格，第2个单词之后将有两个额外的空格，第3个单词之后将有
	// 3个额外的空格，第4个单词之后将有3个额外的空格。
	extra_spaces = MAX_LINE_LEN - line_len;
	for (i = 0; i < line_len; i++) {
		if (line[i] != ' ') {
			putchar(line[i]);
			continue;
		}
		spaces_to_insert = extra_spaces / (num_words - 1);
		for (j = 1; j <= spaces_to_insert+1; j++) {
			putchar(' ');
		}
		extra_spaces -= spaces_to_insert;
		num_words--;
	}
	putchar('\n');
}

void flush_line(void) {
	if (line_len > 0) {
		puts(line);
	}
}