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
#include <time.h>
#include <sys/file.h>
#include <sys/time.h>

char theChannel[20] = "communicate"; 
char theLog[20]="Banka.log";
int fdr,fd,fdLog,runTime;
struct timespec start, end;
int stringToInt(char* temp);
int processNumber=0;
void signalCatcher(int sinyal);
void timerCatcher(union sigval sinyal);
void baslatBankayi(int time);
void process(int processNumber);
void theClientCame(char* ClientName);
void sayThatYouCantGetMoney(char* pidName);
void sayThatAllCantGetMoney();
void writeTheHeaderOfLog(int timee);
void writeTheEndOfLog();
void writeLogToFile();
int printer();


mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
timer_t timer,timerProcess;
int pid[4],processCounter=0;
int pipeFd[2];
int pipeLog[2];
int finishPipeLog[2];
char pidName[10]="";
sigset_t myset;


int main(int argc, char *argv[]) {
	signal(SIGINT, signalCatcher);
	fdLog=0;
	fdLog= open(theLog,O_WRONLY | O_TRUNC | O_CREAT, mode);/*open file for write, if not exist create, and if exists delete data */
	int a=stringToInt(argv[1]);
	writeTheHeaderOfLog(a);
	runTime=a;
	baslatBankayi(a);
	

}

void writeLogToFile(){
	int a[4],b[4];
	int x,y;
	int i=0;
	char theData[255]="";
	close(finishPipeLog[1]);
	while(read(finishPipeLog[0],&x, sizeof(int))>0){
		read(finishPipeLog[0],&y, sizeof(int));
		a[i]=x;
		b[i]=y;
		i++;

	}
	close(finishPipeLog[0]);
	int count=0,sizeStr;
	while(count<i){
		sizeStr=sprintf(theData,"process%d %d musteriye hizmet sundu …\n",a[i-(count+1)],b[i-(count+1)]);
		write(fdLog,theData,sizeStr);
		count++;
	}
		
	
	close(fdLog);
	
}

void writeTheEndOfLog(){	
	int a=write(finishPipeLog[1],&processNumber,sizeof(int));
	int b=write(finishPipeLog[1],&processCounter,sizeof(int));
	close(finishPipeLog[1]);
}

void sayThatYouCantGetMoney(char* pidName){
	int para=-1;  
	char param[5]="";
	int sizeString=sprintf(param,"%da",para);	
	fd=-1;
	fd= open(pidName,0666,mode);
	write(fd,param,sizeString);
	close(fd);

}

void readTheLogPipe(){
		int getPidCustomer;
		int getProcessNumber;
		int getMoney;
		int getTime;
		char theData[255]="";
		int sizeStr;
		int i=0;
		while(read(pipeLog[0],&getPidCustomer, sizeof(int))>0){
			read(pipeLog[0],&getProcessNumber, sizeof(int));
			read(pipeLog[0],&getMoney, sizeof(int));
			read(pipeLog[0],&getTime, sizeof(int));
			sizeStr=sprintf(theData,"%d\t Process%d\t\t %d\t\t %dmsec\t\n",getPidCustomer,getProcessNumber,getMoney,getTime);
			write(fdLog,theData,sizeStr);
			i++;
		}
		sizeStr=sprintf(theData,"%d saniye dolmustur %d müsteriye hizmet verdik \n",runTime,i);
		write(fdLog,theData,sizeStr);
		

}

void sayThatAllCantGetMoney(){
		close(pipeFd[1]);
		int getPidCustomer;
		char piddName[10]="";
		while(read(pipeFd[0],&getPidCustomer, sizeof(int))>0){
			sprintf(piddName,"%d",getPidCustomer);
			sayThatYouCantGetMoney(piddName);
		}

}	
void timerCatcher(union sigval sinyal)
{
	close(finishPipeLog[1]);
	close(pipeLog[1]);
    printf("Bankamızın çalışma süresinin sonuna geldik\n");
    close(fdr);
    remove(theChannel);
    timer_delete(timer);
    
    kill(pid[0],SIGINT);
    kill(pid[1],SIGINT);
    kill(pid[2],SIGINT);
    kill(pid[3],SIGINT);
    sayThatAllCantGetMoney();
    close(pipeFd[0]);
    close(pipeFd[1]);
    readTheLogPipe();
    writeLogToFile();
    exit(1);
}

void timerProcessCatcher(union sigval sinyal)
{
    timer_delete(timerProcess);
    theClientCame(pidName);
    processCounter++;
    process(processNumber);
}

void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT&&pid[0]!=0&&pid[1]!=0&&pid[2]!=0&&pid[3]!=0){
  	sayThatAllCantGetMoney();
    printf("Bankamızda acil durum, sonlanıyor\n");
    readTheLogPipe();
    writeLogToFile();
  	
  } 
  else{
  	writeTheEndOfLog();	
  	sayThatYouCantGetMoney(pidName);
  
  }
  	close(pipeFd[0]);
    close(pipeFd[1]);

    close(fdr);
    remove(theChannel);
    timer_delete(timer);
    if(processCounter!=0){
    	timer_delete(timerProcess);
	}  
	close(finishPipeLog[1]);
	close(pipeLog[1]);  
    exit(1);
}

void writeTheHeaderOfLog(int timee){
   time_t rawtime;
   struct tm *info;
   char forWrite[255]="";
   char Style[511]="";
	char months[12][10]={"Ocak","Şubat","Mart","Nisan","Mayıs","Haziran","Temmuz","Ağustos","Eylül","Ekim","Kasım","Aralık"};
   time( &rawtime );
   info = localtime( &rawtime );
   
   int sizeStr=sprintf(forWrite,"%d %s %d tarihinde islem başladı. Bankamız %d saniye hizmet verecek.\n",info->tm_mday, months[info->tm_mon],1900+info->tm_year,timee);
   write(fdLog,forWrite,sizeStr);
   sizeStr=sprintf(Style,"clientPid\tprocessNo\tPara\tislem bitis zamanı\n----\t--------------\t------\t----------------------\n");
   write(fdLog,Style,sizeStr);
   
   

}
 
void baslatBankayi(int timee){
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    struct sigevent timer_signal_event;
    struct itimerspec timer_period;
    timer_signal_event.sigev_notify = SIGEV_THREAD;
    timer_signal_event.sigev_notify_function = timerCatcher;       /* Timer dolunca bu fonksiyon çağrılcak*/
    timer_signal_event.sigev_notify_attributes = NULL;
    timer_create(CLOCK_MONOTONIC, &timer_signal_event, &timer);
    timer_period.it_value.tv_sec = timee;                                   
    timer_period.it_value.tv_nsec = 0;                                  
    timer_period.it_interval.tv_sec = 0;                               
    timer_period.it_interval.tv_nsec = 0;
    timer_settime(timer, 0, &timer_period, NULL);
                          
	 pid[0]=0;
	 pid[1]=0;
	 pid[2]=0;
	 pid[3]=0;
	 mkfifo(theChannel,0666);
	 int i=0;
 	 pipe(pipeFd);
 	 pipe(pipeLog);
 	 pipe(finishPipeLog);
	 while(i<4){
	 	pid[i+1]==0;	
	 	pid[i]=fork();
	 	if(pid[i]==0){
	 		close(fdLog);
	 		processNumber=4-i;
	 		close(finishPipeLog[0]);
	 		close(pipeLog[0]);
	 		srand(time(NULL)+getpid());
	 		process(processNumber);
	 		break;
	 	}
	 	i++;
	 }
	 printer();
}


int printer(){
	char temp=' ';

	fdr=open(theChannel,O_RDONLY);
	char tempFifo[20]="";
	int i=0;
	int pidCustomer;
		while (read(fdr, &temp, 1)==1){	
				if(temp!='\n'){
					tempFifo[i]=temp;
					i++;
				}else{
					tempFifo[i]='\0';
					pidCustomer=stringToInt(tempFifo);
					write(pipeFd[1],&pidCustomer, sizeof(int));
					i=0;
					tempFifo[i]='\0';
				}	
		}
	close(fdr);
	printer();	
}

void theClientCame(char* clientName){
		rand();				
		int para=1+(rand() % 100);  
		char param[5]="";
		int sizeString=sprintf(param,"%da",para);	
		fd=-1;
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		int dif=(int)( (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000);
		dif=dif/1000;
		fd= open(clientName,0666,mode);
		write(fd,param,sizeString);
		int pidCust=stringToInt(clientName);
		
		
		//printf("%d %d %d %d\n",pidCust,para,processNumber,dif);
		write(pipeLog[1],&pidCust, sizeof(int));
		write(pipeLog[1],&processNumber, sizeof(int));
		write(pipeLog[1],&para, sizeof(int));
		write(pipeLog[1],&dif, sizeof(int));
		close(fd);

}
void process(int processNumber){
	int getPidCustomer;
	if(read(pipeFd[0],&getPidCustomer, sizeof(int))){
		sprintf(pidName,"%d",getPidCustomer);
	
	
		struct sigevent timer_signal_event;
		struct itimerspec timer_period;
		timer_signal_event.sigev_notify = SIGEV_THREAD;
		timer_signal_event.sigev_notify_function = timerProcessCatcher;       // This function will be called when timer expires
		timer_signal_event.sigev_notify_attributes = NULL;
		timer_create(CLOCK_MONOTONIC, &timer_signal_event, &timerProcess);
		timer_period.it_value.tv_sec = 1;                                   // 1 second timer
		timer_period.it_value.tv_nsec = 500000000 ;                                  // no nano-seconds
		timer_period.it_interval.tv_sec = 0;                                // non-repeating timer
		timer_period.it_interval.tv_nsec = 0;
		timer_settime(timerProcess, 0, &timer_period, NULL);   

		(void) sigemptyset(&myset);
		(void) sigsuspend(&myset);
    }
}

int stringToInt(char* temp){
	int a=0,result=0;
	while(temp[a]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
