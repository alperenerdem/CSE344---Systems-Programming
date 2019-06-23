#include <stdlib.h>
#include <stdio.h>	
#include <sys/file.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
		char temp='c';
		int x=0;
		int y=0;
		int fd= open(argv[1],O_RDONLY);
		if(fd<0){
			while(argv[1][x]!='\0'){
			x++;
				if(argv[1][x]=='\n'){
				y++;
				}	
			}
			printf("%d\n",y);
			return 0;
		}
		while (read(fd, &temp, 1) == 1){
			if(temp=='\n')
				x++;
			if(temp=='\0')
				break;
		}
		close(fd);
		printf("%d\n",x);			
}
