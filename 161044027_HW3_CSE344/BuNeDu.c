#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int zFlag=0,directorySize=0;/* -z komutu geldiğinde kullandığım global değişkenler */
int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int postOrderApply(char *path, int pathfun (char *path1));/*ağaç şeklinde klasörlerde dosya dosya gezen fonksiyon */

int main(int argc, char *argv[]) {

	if (argc >= 4) {/*4ve 4ten fazla argümen varsa */
		fprintf(stderr, "Usage: %s directory_name or Usage: %s -z directory_name \n", argv[0], argv[0]);
		return -1;	
	}
	else if(argc ==3&& strcmp(argv[1],"-z")==0 ){/* -z 1.argüman ise */
		zFlag=1;
		postOrderApply(argv[2],sizepathfun);		
	}
	else if(argc ==3&& strcmp(argv[2],"-z")==0 ){/* -z 2.argüman ise */
		zFlag=1;
		postOrderApply(argv[1],sizepathfun);		
	}
	else if(argc ==2){
		postOrderApply(argv[1],sizepathfun);		
	}
	else if(argc ==1){/*1 argüman varsa */
		fprintf(stderr, "Usage: %s directory_name or Usage: %s -z directory_name \n", argv[0], argv[0]);
		return -1;				
	}
	return 0;
} 
int sizepathfun (char *path){
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
int postOrderApply(char *path,int pathfun (char *path1)){
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
			if(isRegular(temp)&&pathfun(temp)>0){
			/*pathfunun pozitif değerleri için return değerini arttırıp, aynı zamanda size arttırıyorum */
				returnCounter++;
				size=size+pathfun(temp);
			}	
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
			/*dosyaysa return counteri recursiveden dönen counterla toplayıp
			size'ı directorySize ile(flag ile uyarıldıysa değeri var yoksa 0)
			topluyorum. */													
				returnCounter=returnCounter+postOrderApply(temp,pathfun);
				size=directorySize+size;
				directorySize=0;						
			} 
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0 &&!(isRegular(temp)))/*link ise ekrana specialFile olduğun basıyor */			
			{													
				printf("Special file %s\n",direntp->d_name);	
			}
		}
		if(zFlag==1){/*-z komutu geldiyse(flag 1 olduysa) directory size'ı saklıyorum */
					directorySize=size;
		}
		printf("%d\t ",size/1024);/*istenildiği gibi kb cinsinde ekrana boyutu basıyorum */			
		printf("%s\n",path);/*ekrana klasör pathini basıyorum */
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 
		return returnCounter;		
}
