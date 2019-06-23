#define _GNU_SOURCE
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
#include <fcntl.h>


int zFlag=0,directorySize=0,fd,rfd;/* -z komutu geldiğinde kullandığım global değişkenler */
char myfifo[20] = "161044027sizes"; 
int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int returnTheSize(char *path, int pathfun (char *path1));/*ağaç şeklinde klasörlerde dosya dosya gezen fonksiyon */
int forker(char *path);/*klasör gördüğünde forklama yapan fonksiyo */
int printer();/*txt'yi dosyaya basıyor */
int stringToInt(char* temp);
void signalCatcher(int sinyal);

void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT)
    printf("You pressed Ctrl+c,Dead id=%d\n",(int)getpid());
    exit(1);
}


int main(int argc, char *argv[]) {
	int pid=0,status,childCount;
  
    // Creating the named file(FIFO) 
    // mkfifo(<pathname>, <permission>) 

	mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;/*write/read permissions */	
	mkfifo(myfifo, 0666);
	fd= open(myfifo,0666,mode);/*open file for read, if not exist create, and if exists delete data */
	fcntl(fd, F_SETPIPE_SZ, 1024 * 1024*sizeof(char));

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
			close(fd);		
		}
		else if(argc ==3&& strcmp(argv[2],"-z")==0 ){/* -z 2.argüman ise */
			forker(argv[1]);	
			close(fd);
		}
		else if(argc ==2){
			forker(argv[1]);
			close(fd);	
		}
		else if(argc ==1){/*1 argüman varsa */
			fprintf(stderr, "Usage: %s directory_name or Usage: %s -z directory_name \n", argv[0], argv[0]);
			return -1;				
		}
	}
	else{
		write(fd,"",1);
		childCount=printer();
		close(fd);	
							
		printf("%d Child process created for directories,Main process id %d\n",childCount,(int)getpid());	
		unlink(myfifo);
		remove(myfifo);
	}
	return 0;
}

int printer(){
	char temp=' ';
	char tempID[20]="",compareID[20]="a",spec[13]="a";
	int processCounter=0,tabFlag=0,dirFlag=0,specFlag=0;
	int x=0;
	/*printf("%d \n",getpid());*//*main process ID */
	
	fd= open(myfifo,0666);
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
				if(specFlag==1&&strcmp(spec,"Special file")==0){
						
						specFlag=0;
						x=0;
						tabFlag=0;
						dirFlag=0;
						spec[0]='\0';
				}
				else{
					spec[0]='\0';
					specFlag=0;
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
			}
			else if(specFlag!=1&&dirFlag==1){
				spec[x]=temp;
				spec[x+1]=='\0';
				if(x==11){
					specFlag=1;
				}
				x++;	
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
	int size=0;
	int getsize=0;
	int pipeFd[2];
 	pipe(pipeFd);
	struct flock lock;
	int status;
	char temp[255],temp2[511];
	struct dirent *direntp;
	
	lock.l_type=F_WRLCK;
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
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
					strcpy(temp2,path);
					strcpy(path,temp);				
					pid=fork();/*alt klasör için child*/
					//waitpid(pid, &status, 0);					
					if(pid==0){			
						while ((closedir(dirp) == -1) && (errno == EINTR));					
							if((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
								perror ("Failed to open directory");
								return -1;
						}
						/*parent ise eski dosyadan devam etmesi için sakladığım klasör ismini path'e geri atıyorum
											child ise karşılaşılan yeni klasör ismi path'e atıldığı else'de onu açıp 
											döngüye ordan devam ediyor. */
					}
					else {
						strcpy(path,temp2);
					}
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 	
			temp2[0]='\0';
			if(zFlag!=1){
				size=returnTheSize(path,sizepathfun);
				sizeString=sprintf(temp2,"%d \t%d \t%s\n",(int)getpid(),size/1024,path);
				while(wait(NULL)>0);	
				write(fd,temp2,sizeString);
				close(pipeFd[1]);
				close(pipeFd[0]);
			}
			else{
				size=returnTheSize(path,sizepathfun);			
				if(pid!=0){ /*En dip çocuk değilse girsin yazılanları okusun,kendi çocuklarını toplasın kendine */
					while(wait(NULL)>0){ 
					read(pipeFd[0],&getsize, sizeof(int)); /*alt çocuklarının size'ını okusun */
					/*printf("%d + %d \t%s\n",getsize/1024,size/1024,path);*/
					/*If you want to understand how i operate, just look the print above */
					size=size+getsize;
					}		
				}	
				write(pipeFd[1],&size, sizeof(int)); /*size'ını yazsın pipe'a */
				close(pipeFd[1]);
				close(pipeFd[0]);
				sizeString=sprintf(temp2,"%d \t%d \t%s\n",(int)getpid(),size/1024,path);	
				write(fd,temp2,sizeString);			
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
				if(zFlag!=1){
					while(wait(NULL)>0);
				}
				/*I dont know how to show special files, if you take this line out of comment line it will be ordered, or showing at the start	*/
				write(fd,temp2,sizeString);										
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
