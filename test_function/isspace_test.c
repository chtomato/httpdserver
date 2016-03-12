#include <ctype.h>
int main(){
	char str[] = "123c @# FD\tsP[e?\n";
	int i;
	for(i = 0; str[i] != 0; i++)
		if(isspace(str[i]))
			 printf("str[%d] is a white-space character:%d\n", i, str[i]);
	return 0;
}
