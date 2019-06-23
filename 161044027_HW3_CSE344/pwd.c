#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char cwd[511]="";
	getcwd(cwd,sizeof(cwd));
	printf("%s",cwd);
}
