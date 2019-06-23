#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
int pathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int returnFileStats(char *path);
int pathfun (char *path){
	struct stat statbuf;
	if (stat(path, &statbuf) == -1) {/*dosya bilgisi alınamıyorsa */
		perror("Failed to get file status\n");
		return -1;
	}	
	if(isDirectory(path))/*klasör ise boyutunu alma(4096byte) */
		return 0;	 
	else 
		return statbuf.st_size;	/*değilse boyutunu döndür */
}
int isDirectory(char *path) {/*klasör olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISDIR(statbuf.st_mode);
}
int isRegular(char *path) {/*regularFile olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISREG(statbuf.st_mode);
} 

int main(int argc, char *argv[]) {
		char path[3]=".";
		char temp[511];/*klasör/dosya isimleri için değişken */
		int size=0,returnCounter=0;

		/*size boyut için,returnCounter pathfunun pozitif değerlerinin tutulduğu değişken*/
		struct dirent *direntp;
		DIR *dirp;/*dosya açmak ve gezmek için değişkenler */		
		if ((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			perror ("Failed to open directory");
			return -1;
		}
		while ((direntp = readdir(dirp)) != NULL){/*bilgi alınabilecek dosya varken */		
			temp[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp,path);
			strcat(temp,"/");
			strcat(temp,direntp->d_name);
			/*bulunduğum klasör ve içindeki dosya arasına '/' karakteri koyuyorum */
			if(!(isDirectory(temp))&&isRegular(temp)){
				printf("R\t");
				returnFileStats(temp);
				printf("%d\t %s \n",pathfun(temp),direntp->d_name);
				size=pathfun(temp);
			}	
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&!(isDirectory(temp)) &&!(isRegular(temp)))		
			{	
				printf("S\t");
				returnFileStats(temp);												
				printf("0\t %s \n",direntp->d_name);
			}
		}
		closedir(dirp);
}

int returnFileStats(char* path){
	struct stat statbuf;
	lstat(path, &statbuf);
	
	if(S_IRUSR & statbuf.st_mode )
		printf("r");
	else
		printf("-");
	if(S_IWUSR& statbuf.st_mode)
		printf("w");
	else
		printf("-");
	if(S_IXUSR& statbuf.st_mode)
		printf("x");
	else
		printf("-");
	
	if(S_IRGRP& statbuf.st_mode)
		printf("r");
	else
		printf("-");
	if(S_IWGRP& statbuf.st_mode)
		printf("w");
	else
		printf("-");
	if(S_IXGRP& statbuf.st_mode)
		printf("x");
	else
		printf("-");
	
	if(S_IROTH& statbuf.st_mode)
		printf("r");
	else
		printf("-");
	if(S_IWOTH& statbuf.st_mode)
		printf("w");
	else
		printf("-");
	if(S_IXOTH& statbuf.st_mode)
		printf("x");
	else
		printf("-");


    
    printf("\t");
}


