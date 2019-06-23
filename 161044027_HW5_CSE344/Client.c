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


char theChannel[20] = "communicate"; 
int stringToInt(char* temp);
void baslatClient(int totalClient);
void signalCatcher(int sinyal);
void client(int which);
int printer();
int fdr,fd;
mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;/*write/read permissions */	

void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT){
  	char theResult[255]="";
	sprintf(theResult,"Musteri %d parasını alamadı :(\n",(int)getpid());
	printf("%s",theResult);
    
    }
    char theRealFifo[20]="";
	sprintf(theRealFifo,"%d",(int)getpid());
	remove(theRealFifo);
    
    exit(1);
}


int main(int argc, char *argv[]) {
	fd= open(theChannel,0666,mode);
	if(fd<0){
		printf("Please start the bank first\n");
		return 0;
	}
	signal(SIGINT, signalCatcher);
	int a=stringToInt(argv[1]);
	baslatClient(a);
}
void baslatClient(int totalClient){

	 int pid[totalClient];
		 pid[0]=0;
 
		 int i=0;
	 while(i<totalClient){
		pid[i+1]==0;
		pid[i]=fork();
		if(pid[i]==0){
		 	client(i+1);
		break;
		}
		i++;
		if(i==totalClient){
			while( wait(NULL) > 0);
		}
	}
	
}
int printer(){
	char temp=' ';
	char tempFifo[20]="";
	char theRealFifo[20]="";
	sprintf(theRealFifo,"%d",(int)getpid());
	int i=0;
	while (read(fdr,&temp,1)){
		if(temp=='a')
			break;
		tempFifo[i]=temp;
		i++;
		tempFifo[i]='\0';
	}
	if(tempFifo[0]=='-'){
		kill(getpid(),SIGINT);
	}
	int para = stringToInt(tempFifo);      	
	char theResult[255]="";
	sprintf(theResult,"Musteri %d %d lira aldi :)\n",(int)getpid(),para);
	printf("%s",theResult);
	close(fdr);
}

void client(int which){
	int money=0;
	char theMessage[255]="";
	int sizeString;
	char theRealFifo[20]="";
	char theFifo[20]="";
	sprintf(theRealFifo,"%d",(int)getpid());
	mkfifo(theRealFifo,0666);
	fdr=open(theRealFifo,0666);
	
	sizeString=sprintf(theFifo,"%d\n",(int)getpid());	
	write(fd,theFifo,sizeString);
	close(fd);
	printer();
	remove(theRealFifo);
	


}

int stringToInt(char* temp){
	int a=0,result=0;
	while(temp[a]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
