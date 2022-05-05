#include <stdio.h>
#include <stdlib.h>
#include "readline.h"

#define NAME_LEN 25

struct part {
	int number;
	char name[NAME_LEN+1];
	int on_hand;
	struct part *next;
};

struct part *inventory = NULL;

struct part *find_part(int number);
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
 *            list. Returns a pointer to the node         *
 *            containing the part number; if the part     *
 *            number is not found, returns NULL.          *
 **********************************************************/
struct part *find_part(int number) {
	struct part *p;
	
	for (p = inventory;
			 p != NULL && number > p->number;
			 p = p->next)
			 ;
	if (p != NULL && number == p->number) {
		return p;
	}
	return NULL;
}

/**********************************************************
 * insert: Prompts the user for information about a new   *
 *         part and then inserts the part into the        *
 *         inventory list; the list remains sorted by     *
 *         part number. Prints an error message and       *
 *         returns prematurely if the part already exists *
 *         or space could not be allocated for the part   *
 **********************************************************/
void insert(void) {
	struct part *cur, *prev, *new_node;
	
	new_node = malloc(sizeof(struct part));
	if (new_node == NULL) {
		printf("仓库已满， 无法增加零件\n");
		return ;
	}
	
	printf("请输入零件编号: ");
	scanf("%d", &new_node->number);
	
	for (cur = inventory, prev = NULL;
			 cur != NULL && new_node->number > cur->number;
			 prev = cur, cur = cur->next)
			 ;
	if (cur != NULL && new_node->number == cur->number) {
		printf("该零件已存在\n");
		free(new_node);
		return ;
	}
	
	printf("请输入零件名称: ");
	read_line(new_node->name, NAME_LEN);
	printf("请输入存货数量: ");
	scanf("%d", &new_node->on_hand);
	
	new_node->next = cur;
	if (prev == NULL) {
		inventory = new_node;
	} else {
		prev->next = new_node;
	}
}

/**********************************************************
 * search: Prompts the user to enter a part number, then  *
 *         looks up the part in the database. If the part *
 *         exists, prints the name and quantity on hand;  *
 *         if not, prints an error message.               *
 **********************************************************/
void search(void) {
	int number;
	struct part *p;
	
	printf("请输入零件编号: ");
	scanf("%d", &number);
	p = find_part(number);
	if (p != NULL) {
		printf("零件名称: %s\n", p->name);
		printf("零件存货量: %d\n", p->on_hand);
	} else {
		printf("零件未找到\n");
	}
}

/**********************************************************
 * search: Prompts the user to enter a part number.       *
 *         Prints an error message if the part doesn't    *
 *         exist; otherwise, prompts the user to enter    *
 *         change in quantity on hand and updates the     *
 *         database.                                      *
 **********************************************************/
void update(void) {
	int number, change;
	struct part *p;
	
	printf("请输入零件编号: ");
	scanf("%d", &number);
	p = find_part(number);
	if (p != NULL) {
		printf("请输入变更数量: ");
		scanf("%d", &change);
		p->on_hand += change;
	} else {
		printf("零件未找到\n");
	}
}

/**********************************************************
 * print: Prints a listing of all parts in the database,  *
 *        showing the part number, part name, and         *
 *        quantity on hand. Part numbers will appear in   *
 *        ascending order.                                *
 **********************************************************/
void print(void) {
	struct part *p;
	printf("零件编号	零件名称		零件存货量\n");
	for (p = inventory; p != NULL; p = p->next) {
		printf("%7d		%-25s%11d\n", p->number, p->name, p->on_hand);
	}	
}