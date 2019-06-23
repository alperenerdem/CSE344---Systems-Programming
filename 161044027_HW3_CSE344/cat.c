#include <stdlib.h>
#include <stdio.h>	
#include <sys/file.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int isDirectory(char *path) {/*klasör olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISDIR(statbuf.st_mode);
}

int main(int argc, char *argv[]) {
		char temp='c';

		int fd= open(argv[1],O_RDONLY);
		if(fd<0){
			printf("%s : is a directory or something cant be read\n",argv[1]);
			return 0;
		}
		while (read(fd, &temp, 1) == 1){
			printf("%c",temp);
			if(temp=='\0')
				break;
		}
		close(fd);
}			
