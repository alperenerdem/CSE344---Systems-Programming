// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <sys/file.h>
#include <errno.h>
#define PORT 8080
int stringToInt(const char* temp);
int *Threads;
int Size;
char path1[255]="";
char spec[10]="spec";
char endFile[255]="***endFile***";
char reg[10]="regu";
char dir[10]="dire";
char fifo[10]="fifo";
char Link[10]="link";
char end[10]="***end***";
char theLog[10]="LOG.txt";
void TheServer(const char* directory,int threadPoolSiz,int portnumber);
void syncronizeFiles(const char *path,int socket);
int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int isLink(char *path);
int fd;
int CreateTheThreads(int threadPoolSiz);
int server_fd, valread,addrlen,opt;
mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; 
 struct sockaddr_in address; 
void signalCatcher(int sinyal);
void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT){
  	free(Threads);
  	close(fd);
    printf("You pressed Ctrl+c");
    exit(1);
   } 
   else if (sinyal == SIGTERM){
   free(Threads);
   close(fd);
    printf("Its SIGTERM signal");
    exit(1);
   } 
}
void syncronizeFiles(const char *path2,int socket){
		
		char temp[255];/*klasör/dosya isimleri için değişken */
		int size=0,returnCounter=0;
		/*size boyut için,returnCounter pathfunun pozitif değerlerinin tutulduğu değişken*/
		struct dirent *direntp;
		DIR *dirp;/*dosya açmak ve gezmek için değişkenler */		
		if ((dirp = opendir(path2)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			perror ("Failessd to open directory");
		}
		while ((direntp = readdir(dirp)) != NULL){/*bilgi alınabilecek dosya varken */		
			temp[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp,path2);
			strcat(temp,"/");
			strcat(temp,direntp->d_name);
			//printf("%s\n",direntp->d_name);
			/*bulunduğum klasör ve içindeki dosya arasına '/' karakteri koyuyorum */
			if(sizepathfun(temp)>0){
					returnCounter++;
					if(isRegular(temp)&&!(isDirectory(temp))){
						struct stat forTime;
    					stat(temp, &forTime);
    					time_t t1=forTime.st_mtime;
    				
   						//printf("Last modified time: %s -%ld-", ctime(&forTime.st_mtime),t1);
						size=sizepathfun(temp);
						//printf("%d\t ",size);
						send(socket , reg, 10, 0 );
						send(socket , temp, 255, 0 );
						char Time[25]="";
						sprintf(Time,"%ld",(long int)t1);
						//printf("time %s\n",Time);
						send(socket , Time, 25, 0 );
						int fd2= open(temp, 0666);
						char sizeStr[15]="";
						int tempSize=size;
						sprintf(sizeStr,"%d",size);
						send(socket , sizeStr, 15, 0 );
						char sendData=' ';	
						while(tempSize>0){
							read(fd2, &sendData, 1);
							send(socket , &sendData, 1, 0 );
							tempSize=tempSize-1;
						}
						//printf("\n");
						/*istenildiği gibi kb cinsinde ekrana boyutu basıyorum */			
						char forLog[512]="";
						int strSize=sprintf(forLog,"%s is updating in client (%s byte)\n",temp,sizeStr);
						write(fd,forLog,strSize);
					}
				}	
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
			/*dosyaysa return counteri recursiveden dönen counterla toplayıp
			size'ı directorySize ile(flag ile uyarıldıysa değeri var yoksa 0)
			topluyorum. */
				
				send(socket , dir, 10, 0 );
				send(socket , temp, 255, 0 );
				char forLog[512]="";
				int strSize=sprintf(forLog,"Directory %s is updating in client\n",temp);
				write(fd,forLog,strSize);												
				syncronizeFiles(temp,socket);						
			} 
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0 &&!(isRegular(temp))&&!isDirectory(temp))/*link ise ekrana specialFile olduğun basıyor */			
			{													
				if(isLink(temp)){
					char* cwd="";
					char temp3[511];
					cwd=realpath(temp,temp3);
					send(socket , Link, 10, 0 );
					send(socket , temp, 255, 0 );
					send(socket , temp3, 511, 0 );
					char forLog[512]="";
					int strSize=sprintf(forLog,"Link %s is updating in client\n",temp);
					write(fd,forLog,strSize);
				}
				else{
					send(socket , fifo, 10, 0 );
					send(socket , temp, 255, 0 );
					char forLog[512]="";
					int strSize=sprintf(forLog,"Fifo %s is updating in client\n",temp);
					write(fd,forLog,strSize);
				}	
				//printf("Special file %s\n",temp);	
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 
		
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
int isLink(char *path) {/*regularFile olup olmadığını kontrol ediyor */
	struct stat statbuf;
	
	if (lstat(path, &statbuf) == -1)
		return 0;
	else 
		return S_ISLNK(statbuf.st_mode);
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
int stringToInt(const char* temp){
	int a=0,result=0;
	while(temp[a]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
int main(int argc, char const *argv[]) 
{ 
	
	signal(SIGINT, signalCatcher);
   if(argc==4){
   		char forLog[512]="";
		int strSizem=sprintf(forLog,"The Server is starting with %s thread",argv[2]);
		write(fd,forLog,strSizem);
   		TheServer(argv[1],stringToInt(argv[2]),stringToInt(argv[3])); 
   }
   else
   	printf("Usage: BibakBOXServer [directory] [threadPoolSize] [portnumber]\n");
}
void TheServer(const char* directory,int threadPoolSiz,int portnumber){

 	mkdir(directory,0777);
 	if (chdir(directory)) {/*verilen argümanla dosya açarken hata varsa */
		perror ("Failed to open directory");
	}
	fd= open(theLog,O_WRONLY | O_TRUNC | O_CREAT, mode);
 	sprintf(path1,"%s",directory);
 	DIR *dirp;	
 	Size=threadPoolSiz;
 	Threads=(int*)malloc(sizeof(int)*(threadPoolSiz+1));
	int count=0;
	while(count<threadPoolSiz+1){
		Threads[count]=0;
		count++;
	}
 	int valread; 
    opt = 1; 
    addrlen = sizeof(address); 


    // Creating socket file descriptor 
   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 

    address.sin_port = htons(portnumber); 
       

    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    CreateTheThreads(threadPoolSiz);

} 
void *myThreadFun(void *vargp) 
{ 
	int new_socket=0;
    int *x = (int *)vargp; 
    int y=*x;
	Threads[y]=1;
    //printf("%d Thread Number %d\n",y,*x);
    char temp[255]="";
    char mainDir[1024]="";
    char type[10]="";
    while((new_socket = accept(server_fd, (struct sockaddr *)&address,  (socklen_t*)&addrlen))>0)
    { 
    	
    	read( new_socket , temp, 255); 
    	int newClientFlag=0;
    	//printf("sa%s\n",temp );
    	sprintf(mainDir,"%s",temp);
    	char forLoge[512]="";
		int strSizemm=sprintf(forLoge,"Thread %d started his job with directory %s\n",y,mainDir);
		write(fd,forLoge,strSizemm);
    	int z=mkdir(mainDir,0777);
    	//printf("%d\n",z );
    	if(z!=-1){
    		char forLog[512]="";
			int strSize=sprintf(forLog,"Client with directory %s first time connected,Files are coming\n",mainDir);
			write(fd,forLog,strSize);	
    		newClientFlag=1;
    	}
    	else{
    		char forLog[512]="";
			int strSize=sprintf(forLog,"An old friend came with %s, we are going to updating files to him\n",mainDir);
			write(fd,forLog,strSize);	   	
    	}
    	if(newClientFlag==1){
    		char conMode[10]="";
			sprintf(conMode,"NEW");
			send(new_socket , conMode, 10, 0 );	
			while(1){
			
				read( new_socket , type, 10);
				if(strcmp(type,end)==0){
					break;
				} 
				read( new_socket , temp, 255);
				sprintf(mainDir,"%s",temp);
				if(strcmp(type,end)==0){
					break;
				}
				else if(strcmp(type,dir)==0){
					mkdir(mainDir,0777);
					char forLog[512]="";
					int strSize=sprintf(forLog,"Directory %s is going to upload to server\n",mainDir);
					write(fd,forLog,strSize);
				}
				else if(strcmp(type,reg)==0){
					struct stat forTime;
					stat(mainDir, &forTime);
					char Time[25]="";
					read(new_socket ,Time, 25 );
					time_t t1=forTime.st_mtime;
					long int IncomingTime=0;
					IncomingTime=strtol(Time,NULL,10);
					forTime.st_mtime=IncomingTime;
					//printf("%ld -- %ld \n",IncomingTime,t1);
					int fdc= open(mainDir,O_WRONLY | O_TRUNC | O_CREAT, mode);				
					char strSize[15]="";
					
					read( new_socket , strSize, 15);
					int sizeStr=stringToInt(strSize);
					char forLog[512]="";
					int strSizek=sprintf(forLog,"File %s is going to upload to server(%d bytes)\n",mainDir,sizeStr);
					write(fd,forLog,strSizek);
					
					//printf("%d %s\n",sizeStr,mainDir);
						while(sizeStr>0){
								char readData=' ';
								read( new_socket , &readData, 1);
								write(fdc,&readData,1);
								sizeStr=sizeStr-1;
						}
				}
				else if(strcmp(type,Link)==0){
					char link[511]="";
					char cwd[511]="";
					getcwd(cwd,sizeof(cwd));
					strcat(cwd,"/");
					strcat(cwd,temp);
					read( new_socket , link, 511);
					int fail=symlink(cwd,link);
					char forLog[512]="";
					if(fail==-1){
						int strSizem=sprintf(forLog,"%s symbolick Link couldnt copy\n",temp);
						write(fd,forLog,strSizem);
					}
					else{
						int strSizem=sprintf(forLog,"%s symbolick Link uploaded to server\n",temp);
						write(fd,forLog,strSizem);
					}
				}
				else if(strcmp(type,fifo)==0){
					mkfifo(temp,0666);
				}
		
				sprintf(mainDir,"%s",temp);
			}
			
    	}
		else{
			sprintf(mainDir,"%s",temp);
			char conMode[10]="";
			sprintf(conMode,"OLD");
			send(new_socket , conMode, 10, 0 );	
			syncronizeFiles(mainDir,new_socket);
			send(new_socket ,end, 10, 0 );
		
		
		}
		char forLog[512]="";
		int strSizem=sprintf(forLog,"Thread %d finished his job now\n",y);
		write(fd,forLog,strSizem);
	}
	
 				
} 
int CreateTheThreads(int threadPoolSiz){ 
   int i=0; 
   pthread_t tid[threadPoolSiz]; 
   int threadId[threadPoolSiz];
   char TheFifo[10]="";

    for (i = 0; i < threadPoolSiz;i++){
   		threadId[i]=i;      
  	}
  	
    for (i = 0; i < threadPoolSiz;i++){  		
        pthread_create(&tid[i], NULL, myThreadFun, (void *)&(threadId[i]));        
  	} 
 	
  	for (i = 0; i < threadPoolSiz;i++){		
       pthread_join(tid[i],NULL);        
  	} 

  
    
    return 0; 
} 
