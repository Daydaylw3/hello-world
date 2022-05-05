#include <stdio.h>
#include "readline.h"

#define NAME_LEN 25
#define MAX_PARTS 100

struct part {
	int number;
	char name[NAME_LEN+1];
	int on_hand;
} inventory[MAX_PARTS];

int num_parts = 0;

int find_part(int number);
void insert(void);
void search(void);
void update(void);
void print(void);

int main(void) {
	char code;
	
	for (;;) {
		printf("请输入操作码: ");
		scanf(" %c", &code);
		while (getchar() != '\n')	// skips to end of line
			;
		switch (code) {
			case 'i': insert(); break;
			case 's': search(); break;
			case 'u': update(); break;
			case 'p': print(); break;
			case 'q': return 0;
			default: printf("非法操作符\n");
		}
		printf("\n");
	}
}

/**********************************************************
 * find_part: Looks up a part number in the inventory     *
 *            array. Returns the array index if the part  *
 *            number is found; otherwise, returns -1.     *
 **********************************************************/
int find_part(int number) {
	int i;
	
	for (i = 0; i < num_parts; i++) {
		if (inventory[i].number == number) {
			return i;
		}
	}
	return -1;
}

/**********************************************************
 * insert: Prompts the user for information about a new   *
 *         part and then inserts the part into the        *
 *         database. Prints an error message and returns  *
 *         prematurely if the part already exists or the  *
 *         database is full.                              *
 **********************************************************/
void insert(void) {
	int part_number;
	
	if (num_parts == MAX_PARTS) {
		printf("仓库已满，无法增加零件\n");
		return ;
	}
	
	printf("请输入零件编号: ");
	scanf("%d", &part_number);
	
	if (find_part(part_number) >= 0) {
		printf("该零件已存在\n");
		return ;
	}
	
	inventory[num_parts].number = part_number;
	printf("请输入零件名称: ");
	read_line(inventory[num_parts].name, NAME_LEN);
	printf("请输入存货数量: ");
	scanf("%d", &inventory[num_parts].on_hand);
	num_parts++;
}

/**********************************************************
 * search: Prompts the user to enter a part number, then  *
 *         looks up the part in the database. If the part *
 *         exists, prints the name and quantity on hand;  *
 *         if not, prints an error message.               *
 **********************************************************/
void search(void) {
	int i, part_number;
	
	if (num_parts == 0) {
		printf("仓库为空\n");
		return ;
	}
	
	printf("请输入零件编号: ");
	scanf("%d", &part_number);
	i = find_part(part_number);
	if (i >= 0) {
		printf("零件名称: %s\n", inventory[i].name);
		printf("零件存货量: %d\n", inventory[i].on_hand);
	} else {
		printf("零件未找到\n");
	}
}

/**********************************************************
 * update: Prompts the user to enter a part number.       *
 *         Prints an error message if the part doesn't    *
 *         exist; otherwise, prompts the user to enter    *
 *         change in quantity on hand and updates the     *
 *         database.                                      *
 **********************************************************/
void update(void) {
	int i, part_number, change;
	
	if (num_parts == 0) {
		printf("仓库为空\n");
		return ;
	}
	
	printf("请输入零件编号: ");
	scanf("%d", &part_number);
	i = find_part(part_number);
	if (i >= 0) {
		printf("请输入变更数量: ");
		scanf("%d", &change);
		inventory[i].on_hand += change;
	} else {
		printf("零件未找到\n");
	}
}

/**********************************************************
 * print: Prints a listing of all parts in the database,  *
 *        showing the part number, part name, and         *
 *        quantity on hand. Parts are printed in the      *
 *        order in which they were entered into the       *
 *        database.                                       *
 **********************************************************/
void print(void) {
	int i;
	
	printf("零件编号	零件名称		零件存货量\n");
	for (i = 0; i < num_parts; i++) {
		printf("%7d		%-25s%11d\n", inventory[i].number, 
			inventory[i].name, inventory[i].on_hand);
	}
}