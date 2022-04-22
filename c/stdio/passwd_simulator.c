#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#define PWDLEN 20

void getpwd(char *pwd, int pwdlen);

int main() {
	char pwd[PWDLEN+1];
	printf("Input password: ");
	getpwd(pwd, PWDLEN);
	printf("The password is %s\n", pwd);
	return 0;
}

/**
 * 获取用户输入的密码
 * @param pwd    char* 保存密码的内存首地址
 * @param pwdlen int   密码的最大长度
 */
void getpwd(char *pwd, int pwdlen) {
	char ch = 0;
	int i = 0;
	while (i < pwdlen) {
		ch = getch();
		
		if (ch == '\r') { // 回车
			printf("\n");
			break;
		}
		
		if (ch == '\b' && i > 0) { // 删除
			i--;
			printf("\b \b");
		} else if (isprint(ch)) { // 为可打印字符
			pwd[i] = ch;
			printf("*");
			i++;
		}
	}
	pwd[i] = '\0';
}