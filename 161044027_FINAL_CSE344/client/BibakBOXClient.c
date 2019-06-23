// Client side C/C++ program to demonstrate Socket programming 
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
char spec[10]="spec";
char fifo[10]="fifo";
char Link[10]="link";
char reg[10]="regu";
char dir[10]="dire";
char end[10]="***end***";
char endFile[255]="***endFile***";
int stringToInt(const char* temp){
	int a=0,result=0;
	while(temp[a]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
int TheClient(const char* directory,const char* ipAdress,int portnumber);
int zFlag=0,directorySize=0;/* -z komutu geldiğinde kullandığım global değişkenler */
int sizepathfun (char *path); /*size döndürüyor,directory ise 0, error durumunda -1 */
int isRegular(char *path);/*Regular File olup olmadığına bakıyor */
int isDirectory(char *path); /*directory olup olmadığına bakıyor */
int isLink(char *path);
int postOrderApply(const char *path,int socket);/*ağaç şeklinde klasörlerde dosya dosya gezen fonksiyon */
void sendFiles(const char* directory,int socket); 
char* ignoreThePath(const char* temp);
void syncronizeFiles(const char *path,int socket);
void getFiles(const char *path,int socket);
mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; 
void sendFiles(const char* directory,int socket){
	 printf("Connected Succesfully to BibakBOXServer\n");
	 int x=0,a=0;
	char returnDir[255]="";
	int strLen=(int)strlen(directory);	
	while(a<strLen){
		if(directory[a]!='/'){
			returnDir[x]=directory[a];
			returnDir[x+1]='\0';
			x++;
		}
		else if(a+1<strLen&&directory[a+1]!='/'){
			returnDir[0]='\0';
			x=0;
		}		
		
		a++;	
	}
	send(socket , returnDir, 255, 0 );
	char conMode[10]="";
	read( socket , conMode, 10); 
	if(strcmp(conMode,"NEW")==0){
		printf("Welcome To BibakBOXServer, We hope you enjoy,Files are uploading... \n");
		postOrderApply(returnDir,socket);
		send(socket , end, 10, 0 );
		printf("Upload is DONE\n");
	} 
	else{
		printf("Its like you enjoyed,Welcome To BibakBOXServer again\nFiles are updating... \n");
		getFiles(directory,socket);
		printf("Updating is DONE\n");
		//syncronizeFiles(directory,socket);
	}
	//send(socket , end, 10, 0 );
}
void getFiles(const char *path,int socket){
	while(1){
		 char temp[255]="";
    	char mainDir[1024]="";
  		 char type[10]="";	
  		 //printf("type %s \n name %s\n",type,mainDir);
		read( socket , type, 10);
		if(strcmp(type,end)==0){
			break;
		} 
		read( socket , temp, 255);
		sprintf(mainDir,"%s",temp);
		//printf("type %s \n name %s\n",type,mainDir);
		if(strcmp(type,end)==0){
			break;
		}
		else if(strcmp(type,dir)==0){
			mkdir(mainDir,0777);
		}
		else if(strcmp(type,reg)==0){
			struct stat forTime;
			stat(mainDir, &forTime);
			char Time[25]="";
			read(socket ,Time, 25 );
			time_t t1=forTime.st_mtime;
			long int IncomingTime=0;
			IncomingTime=strtol(Time,NULL,10);
			forTime.st_mtime=IncomingTime;
			//printf("%ld -- %ld \n",IncomingTime,t1);
			int fd= open(mainDir,O_WRONLY | O_TRUNC | O_CREAT, mode);				
			char strSize[15]="";
			
			read( socket, strSize, 15);
			int sizeStr=stringToInt(strSize);
			//printf("%d %s\n",sizeStr,mainDir);
				while(sizeStr>0){
						char readData=' ';
						read( socket , &readData, 1);
						write(fd,&readData,1);
						sizeStr=sizeStr-1;
				}
		}
		else if(strcmp(type,Link)==0){
			char link[511]="";
			char cwd[511]="";
			getcwd(cwd,sizeof(cwd));
			strcat(cwd,"/");
			strcat(cwd,temp);
			read( socket , link, 511);
			symlink(cwd,link);
			//printf("%d linkk %s-- %s\n",a,cwd,link);
		}
		else if(strcmp(type,fifo)==0){
			mkfifo(temp,0666);
		}

		
	}
}			
void syncronizeFiles(const char *path,int socket){
char temp[255];/*klasör/dosya isimleri için değişken */
		int size=0,returnCounter=0;
		/*size boyut için,returnCounter pathfunun pozitif değerlerinin tutulduğu değişken*/
		struct dirent *direntp;
		DIR *dirp;/*dosya açmak ve gezmek için değişkenler */		
		if ((dirp = opendir(path)) == NULL) {/*verilen argümanla dosya açarken hata durumunda -1 döndürüyor */
			perror ("Failed to open directory");
		}
		while ((direntp = readdir(dirp)) != NULL){/*bilgi alınabilecek dosya varken */		
			temp[0]='\0';/*junk değer varsa diye nulluyorum */
			strcat(temp,path);
			strcat(temp,"/");
			strcat(temp,direntp->d_name);
			//printf("%s\n",direntp->d_name);
			/*bulunduğum klasör ve içindeki dosya arasına '/' karakteri koyuyorum */
			if(sizepathfun(temp)>0){
					if(isRegular(temp)&&!(isDirectory(temp))){
						/*struct stat forTime;
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
						}*/
						//printf("\n");
						/*istenildiği gibi kb cinsinde ekrana boyutu basıyorum */			
						//printf("%s\n",temp);/*ekrana klasör pathini basıyorum */
					}
				}	
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
			/*dosyaysa return counteri recursiveden dönen counterla toplayıp
			size'ı directorySize ile(flag ile uyarıldıysa değeri var yoksa 0)
			topluyorum. */
				
				/*send(socket , dir, 10, 0 );
				send(socket , temp, 255, 0 )*/;
		
				//printf("0 \t %s\n",temp);/*ekrana klasör pathini basıyorum */													
				syncronizeFiles(temp,socket);						
			} 
			else if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0 &&!(isRegular(temp))&&!isDirectory(temp))/*link ise ekrana specialFile olduğun basıyor */			
			{													
				/*if(isLink(temp)){
					char* cwd="";
					char temp3[511];
					cwd=realpath(temp,temp3);
					send(socket , Link, 10, 0 );
					send(socket , temp, 255, 0 );
					send(socket , temp3, 511, 0 );
				}
				else{
					send(socket , fifo, 10, 0 );
					send(socket , temp, 255, 0 );
				}*/	
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
int postOrderApply(const char *path,int socket){
		char temp[255];/*klasör/dosya isimleri için değişken */
		int size=0,returnCounter=0;
		/*size boyut için,returnCounter pathfunun pozitif değerlerinin tutulduğu değişken*/
		struct dirent *direntp;
		//printf("%s",path);
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
						//printf("%s\n",temp);/*ekrana klasör pathini basıyorum */
					}
				}	
			if(strcmp(direntp->d_name,".")!=0&&strcmp(direntp->d_name,"..")!=0&&isDirectory(temp)){
			/*dosyaysa return counteri recursiveden dönen counterla toplayıp
			size'ı directorySize ile(flag ile uyarıldıysa değeri var yoksa 0)
			topluyorum. */
				
				send(socket , dir, 10, 0 );
				send(socket , temp, 255, 0 );
		
				//printf("0 \t %s\n",temp);/*ekrana klasör pathini basıyorum */													
				returnCounter=returnCounter+postOrderApply(temp,socket);						
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
				}
				else{
					send(socket , fifo, 10, 0 );
					send(socket , temp, 255, 0 );
				}	
				//printf("Special file %s\n",temp);	
			}
		}
		while ((closedir(dirp) == -1) && (errno == EINTR)) ;/*tüm dosyaları kapatıyorum */ 
		return returnCounter;		
}
int TheClient(const char* directory,const char* ipAdress,int portnumber){
	struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr;  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET;
   	serv_addr.sin_port = htons(portnumber); 
    //"127.0.0.1"   
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET,ipAdress , &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed to BibakBOXServer \n"); 
		return -1;
    }
    sendFiles(directory,sock);
}
int main(int argc, char const *argv[]) 
{ 	
	if(argc==4){
   		 TheClient(argv[1],argv[2],stringToInt(argv[3]));
   }
   else{
   		printf("Usage: > BibakBOXClient [dirName] [ip address] [portnumber]\n");
   }
    return 0; 
} 
