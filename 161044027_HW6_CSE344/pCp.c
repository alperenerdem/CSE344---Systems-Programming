#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/file.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h> 

/*
**Ödev tamamlanmadı, customer threadiyle doğru inputlarım alamadığım için sildim, producer threadi her şeyi kendi yapıyor.
**Sadece main+producer threadi var.
*/
typedef struct forBuffer { 
    int rd;
    int wd;
    int flag;
    char fileName[512]; 
} forBuf;

int stringToInt(char* temp);
mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
forBuf* Buffer;
forBuf tempBuf;
pthread_t* customers;
int FullFlag=0,emptyFlag=0;
int file=0,byte=0,links=0,fifos=0,direc=0;


struct timespec start, end;
pthread_mutex_t lockPro,lockCus; 

int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int isLink(char *path);
int isFifo(char *path);
int searchAndCopyInDirectory(char *path,char *dest);/*ağaç şeklinde klasörlerde dosya dosya gezen fonksiyon */


void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT){
    printf("You pressed Ctrl+c");
    exit(1);
   } 
   else if (sinyal == SIGTERM){
    printf("Its SIGTERM signal");
    exit(1);
   } 
}
void *producer(void* argv2){
	DIR *dirp;
	char **direct=(char **)argv2;
	if ((dirp = opendir(direct[1])) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			mkdir(direct[1],0777);
	}
	while ((closedir(dirp) == -1) && (errno == EINTR)) ;
	searchAndCopyInDirectory(direct[0],direct[1]);
	
}
void *consumer(void* argv2){
	/*forBuf *direct=(forBuf *)argv2;
	int size=sizepathfun(temp);
	int count=0,c=0;
	char a=' ',b[1]="";
	while(count<size+1){
		read(forBuf[c].rd, &a, 1);
		write(forBuf[c].wd,&a,sizeof(a));
		count++;
	};	*/
}
int main(int argc, char *argv[]) {	
	char cwd[511]="";
	getcwd(cwd,sizeof(cwd));
	strcat(cwd,"/");
	strcat(cwd,argv[4]);
	char * argv2[2]={NULL,NULL};
	signal(SIGINT, signalCatcher);
	signal(SIGTERM, signalCatcher);
	if(argc == 5 ){
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);

		argv2[0]=argv[3];
		argv2[1]=cwd;
		pthread_t thread_id; 
		Buffer=malloc (stringToInt(argv[1])*sizeof(forBuf));
		/*pthread_t consumer_id;
    	pthread_create(&consumer_id, NULL, consumer,(void *) NULL);   */
   		pthread_create(&thread_id, NULL, producer,(void *) argv2); 
   		pthread_join(thread_id, NULL);
   		/*pthread_join(consumer_id, NULL);
   		pthread_exit(NULL);*/
   		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		int dif=(int)( (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000);
		dif=dif/1000;	
		printf("Copied %d file(%d byte) %d fifos %d directories and %d links in %d milisec\n",file,byte,fifos,direc,links,dif);	
    	free(Buffer);
    	
	}
	else {
		fprintf(stderr, "Usage: %s NumberOfConsumers BufferSize SourcePath DestinationPath", argv[0]);
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
int isFifo(char *path) {/*regularFile olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISFIFO(statbuf.st_mode);
} 
int isLink(char *path) {/*regularFile olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISLNK(statbuf.st_mode);
}  

int searchAndCopyInDirectory(char *path,char *dest){
		char temp[1024],temp2[1024],forCreate[1024];/*klasör/dosya isimleri için değişken */
		int size=0;
		struct dirent *direntp;
		DIR *dirp;/*dosya açmak ve gezmek için değişkenler */		
		if ((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			perror ("Failed to open directory");
			return -1;
		}
		while ((direntp = readdir(dirp)) != NULL){/*bilgi alınabilecek dosya varken */	
			char workingDir[1024]="";	
			temp[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp,path);
			strcat(temp,"/");
			strcat(temp,direntp->d_name);
			//sprintf(workingDir,"%s%s",dir,direntp->d_name);
			temp2[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp2,dest);
			strcat(temp2,"/");
			strcat(temp2,direntp->d_name);
			
			/*bulunduğum klasör ve içindeki dosya arasına '/' karakteri koyuyorum */
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
				mkdir(temp2,0777);											
				searchAndCopyInDirectory(temp,temp2);
				direc++;						
			} 
		
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0 &&!(isRegular(temp))&&!isDirectory(temp))/*link ise ekrana specialFile olduğun basıyor */			
			{	
				if(isLink(temp)){
					char cwd[511]="";
					getcwd(cwd,sizeof(cwd));
					strcat(cwd,"/");
					strcat(cwd,temp);
					int a=symlink(cwd,temp2);
					if(a!=-1){
						printf("%s Copied in Success\n",direntp->d_name);
						links++;
					}
					else
						printf("%s failed while copying(probably link exists already)\n",direntp->d_name);
					
				}
				else	{										
					int a=mkfifo(temp2,0666);
					if(a!=-1){
						fifos++;
						printf("%s Copied in Success\n",direntp->d_name);
					}
					else
						printf("%s failed while copying(probably fifo exists already)\n",direntp->d_name);
				}
					
			}
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")){
				
				int fd2= open(temp, 0666);	
				if(fd2!=-1){
					int fd= open(temp2,O_WRONLY | O_TRUNC | O_CREAT, mode);
					if(fd!=-1){	
						tempBuf.rd=fd2;
						tempBuf.wd=fd;
						strcpy(tempBuf.fileName,temp);
						emptyFlag=1;
						int size=sizepathfun(temp);
						int count=0;
						file++;
						char a=' ',b[1]="";
						while(count<size+1){
							read(tempBuf.rd, &a, 1);
							byte++;
							write(tempBuf.wd,&a,sizeof(a));
							count++;
						}
						printf("%s Copied in Success\n",direntp->d_name);
						close(fd);
						close(fd2);
					}
					else{
						printf("Unable to create copy %s\n",direntp->d_name);
						close(fd2);
					}	
				}
				else
					printf("Failed to open file for copying %s\n",direntp->d_name);
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 		

		return 1;
}
int stringToInt(char* temp){
	int a=0,result=0;
	while(temp[a+1]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
