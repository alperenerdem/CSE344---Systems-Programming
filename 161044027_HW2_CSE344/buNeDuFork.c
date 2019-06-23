#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include <sys/file.h>

int zFlag=0,directorySize=0,fd,rfd;/* -z komutu geldiğinde kullandığım global değişkenler */
char file[20]="161044027sizes.txt"; 
int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int returnTheSize(char *path, int pathfun (char *path1));/*ağaç şeklinde klasörlerde dosya dosya gezen fonksiyon */
int forker(char *path);/*klasör gördüğünde forklama yapan fonksiyo */
int printer();/*txt'yi dosyaya basıyor */
int printerWithSub();/*txt üzerinden hesaplar yaparak subdirectoryleri katarak size basıyor */
int findSubSizesInFile(char* parentFile);/*klasörün alt klasör sizelarını kendine ekliyor */
int checkIsItSubdir(char* checkFile,char* parentFile);/*2 klasörü karşılaştıırıp alt klasörü olup olmadığını söylüyor */
int stringToInt(char* temp);
void signalCatcher(int sinyal);

void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT)
    printf("You pressed Ctrl+c,Dead id=%d\n",getpid());
    exit(1);
}


int main(int argc, char *argv[]) {
	int pid=0,status,childCount;
	mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;/*write/read permissions */	
	fd= open(file,O_WRONLY | O_TRUNC | O_CREAT, mode);/*open file for read, if not exist create, and if exists delete data */
	signal(SIGINT, signalCatcher);
	pid=fork();/*main process sadece okuma yapcağı için yaptığım fork */
	waitpid(pid, &status, 0);/*çocuğu bekliyorum */
	 
	if(argc ==3&& strcmp(argv[1],"-z")==0 )/* -z 1.argüman ise */
			zFlag=1;
	else if(argc ==3&& strcmp(argv[2],"-z")==0 )/* -z 2.argüman ise */
			zFlag=1;
	
	if(pid<0)
		printf("error");
	else if(pid==0){
		if (argc >= 4) {/*4ve 4ten fazla argümen varsa */
			fprintf(stderr, "Usage: %s directory_name or Usage: %s -z directory_name \n", argv[0], argv[0]);
			return -1;	
		}
		else if(argc ==3&& strcmp(argv[1],"-z")==0 ){/* -z 1.argüman ise */
			forker(argv[2]);		
		}
		else if(argc ==3&& strcmp(argv[2],"-z")==0 ){/* -z 2.argüman ise */
			forker(argv[1]);	
		}
		else if(argc ==2){
			forker(argv[1]);	
		}
		else if(argc ==1){/*1 argüman varsa */
			fprintf(stderr, "Usage: %s directory_name or Usage: %s -z directory_name \n", argv[0], argv[0]);
			return -1;				
		}
	}
	else{
		close(fd);
		if(zFlag==0)
			childCount=printer();
		else
			childCount=printerWithSub();	
							
		printf("%d Child process created for directories,Main process id %d\n",childCount,(int)getpid());	
	}
	return 0;
}
int checkIsItSubdir(char* checkFile,char* parentFile){
 int x=0,y=0;
 while(parentFile[x]!='\0'){

 	if(checkFile[x]!=parentFile[x]){	
 		   return 0;
 		}
 	x++;
 }
 if(checkFile[x]=='/')
 	return 1;
 else
 	return 0;	
}
int findSubSizesInFile(char* parentFile){
	char temp;
	int tabFlag=0,dirFlag=0;
	int i=0,size=0,totalSize=0;
	char tempDir[255];
	rfd= open(file,O_RDONLY);
		while (read(rfd, &temp, 1) == 1){
			if(temp=='\t'){
				i=0;
				if(tabFlag==0){
					dirFlag=0;
					tabFlag=1;
				}	
				else{
					size=stringToInt(tempDir);
					dirFlag=1;
					tabFlag=0;
				}	
			}
			else if(temp=='\n'){
				if(strcmp(parentFile,tempDir)==0)
					return size+totalSize;
				else{
					if(checkIsItSubdir(tempDir,parentFile))
						totalSize=size+totalSize;		
				}	
				tabFlag=0;
				dirFlag=0;
				i=0;	
			}
			else if(tabFlag==1){
				tempDir[i]=temp;
				tempDir[i+1]='\0';
				i++;
			}
			else if(dirFlag==1){
				tempDir[i]=temp;
				tempDir[i+1]='\0';
				i++;
			}
			if(temp=='\0')
				break;
		}
	close(rfd);	
		return 1;		
}
int printerWithSub(){
	char temp;
	int tabFlag=0,dirFlag=0,processCounter=0;
	int i=0;
	char tempDir[255]="",tempID[20]="",compareID[20]="a";
	/*printf("%d \n",getpid());*//*main process ID */
	fd= open(file,O_RDONLY);
		while (read(fd, &temp, 1) == 1){
			if(temp=='\t'){
				i=0;
				if(tabFlag==0){
					dirFlag=0;
					tabFlag=1;
				}	
				else{
					dirFlag=1;
					tabFlag=0;
				}	
			}
			else if(temp=='\n'){
				if(processCounter==0){/*ilk process için karşılaştırmaya gerek yok */
					processCounter++;
				}
				if(compareID[0]!='a'&&strcmp(compareID,tempID)!=0){
					processCounter++;
				}
				strcpy(compareID,tempID);
				
				printf("\t%d\t",findSubSizesInFile(tempDir));
				printf("%s\n",tempDir);
				tabFlag=0;
				dirFlag=0;
				i=0;	
			}
			else if(tabFlag==1){
				tempDir[i]=temp;
				tempDir[i+1]='\0';
				i++;
			}
			else if(dirFlag==1){
				tempDir[i]=temp;
				tempDir[i+1]='\0';
				i++;
			}
			else{
				if(temp!=' '){
					tempID[i]=temp;
					tempID[i+1]=='\0';	
					i++;
				}
				printf("%c",temp);						
				}
			if(temp=='\0')
				break;
			
		}
	close(fd);
	return processCounter;		
} 
int printer(){
	char temp;
	char tempID[20]="",compareID[20]="a";
	int tabFlag=0,dirFlag=0;
	int processCounter=0,x=0;
	/*printf("%d \n",getpid());*//*main process ID */
	
	fd= open(file,O_RDONLY);
		while (read(fd, &temp, 1) == 1){
			printf("%c",temp);
			if(temp=='\0')
				break;
			if(temp=='\t'){
				x=0;
				if(tabFlag==0){
					dirFlag=0;
					tabFlag=1;
				}	
				else{
					dirFlag=1;
					tabFlag=0;
				}	
			
			}
			else if(temp=='\n'){
				if(processCounter==0){/*ilk process için karşılaştırmaya gerek yok */
					processCounter++;
				}
				if(compareID[0]!='a'&&strcmp(compareID,tempID)!=0){
					processCounter++;
				}
				strcpy(compareID,tempID);
				tabFlag=0;
				dirFlag=0;
				x=0;
			}
			else if(dirFlag!=1&&tabFlag!=1&& temp!=' '){
				tempID[x]=temp;
				tempID[x+1]=='\0';
				x++;	
			}	
		}
	close(fd);	
	return processCounter;	

}
int forker(char *path){
	int pid=0,sizeString;
	struct flock lock;
	int status;
	char temp[255],temp2[511];
	struct dirent *direntp;
	
	lock.l_type=F_WRLCK;
	DIR *dirp;/*dosya açmak ve gezmek için değişkenler */
	pid=fork();/*ilk childi olşuturyor */
	waitpid(pid, &status, 0);/*parent childi bekliyor */
	if(pid<0)
		printf("error");
	else if(pid==0){	
		if ((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			perror ("Failed to open directory");
			return -1;
		}
		while ((direntp = readdir(dirp)) != NULL){/*bilgi alınabilecek dosya varken */		
			temp[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp,path);
			strcat(temp,"/");
			strcat(temp,direntp->d_name);
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
				if(pid==0){
					strcpy(temp2,path);
					strcpy(path,temp);
				
					pid=fork();/*alt klasör için child ve bekleme */
					waitpid(pid, &status, 0);
					if(pid==0){
						strcpy(path,temp2);/*parent ise eski dosyadan devam etmesi için sakladığım klasör ismini path'e geri atıyorum
											child ise karşılaşılan yeni klasör ismi path'e atıldığı else'de onu açıp 
											döngüye ordan devam ediyor. */
					}
					else {
						pid=fork();/*alt klasörler dallanıyorsa diye child ve bekleme */
						waitpid(pid, &status, 0);	
						if (pid==0){
							while ((closedir(dirp) == -1) && (errno == EINTR));					
							if((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
								perror ("Failed to open directory");
								return -1;
							}
						}
					}
				}
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 
		if(pid!=0){		
			temp2[0]='\0';
			sizeString=sprintf(temp2,"%d \t%d \t%s\n",(int)getpid(),returnTheSize(path,sizepathfun)/1024,path);
			flock(fd, LOCK_EX);
			write(fd,temp2,sizeString);
			flock(fd, LOCK_UN);		
		}
	}
	else{	
		temp2[0]='\0';
		sizeString=sprintf(temp2,"%d \t%d \t%s\n",(int)getpid(),returnTheSize(path,sizepathfun)/1024,path);
		flock(fd, LOCK_EX);
		write(fd,temp2,sizeString);
		flock(fd, LOCK_UN);
	}
}
int sizepathfun (char *path){
	struct stat statbuf;
	if (lstat(path, &statbuf) == -1) {/*dosya bilgisi alınamıyorsa */
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
int returnTheSize(char *path,int pathfun (char *path1)){
		char temp[255],temp2[511];/*klasör/dosya isimleri için değişken */
		int size=0,returnCounter=0,sizeString;
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
				if(pathfun(temp)>0){
					returnCounter++;
					if(isRegular(temp)&&!(isDirectory(temp))){
						size=size+pathfun(temp);
					}
				}
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0 &&!(isRegular(temp))&&!isDirectory(temp))/*link ise ekrana specialFile olduğun basıyor */			
			{	
				sizeString=sprintf(temp2,"%d \t0 \tSpecial file %s\n",(int)getpid(),direntp->d_name);
				flock(fd, LOCK_EX);
				write(fd,temp2,sizeString);
				flock(fd, LOCK_UN);											
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 
		return size;		
}
int stringToInt(char* temp){
	int a=0,result=0;
	while(temp[a+1]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
