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
int parser(char *argv[],int argc);
int historyParser(char *argv[],char *theNullPointer);
void help();
void signalCatcher(int sinyal);
void runThis(char *argv[],char* dir);
int parseThePipe(char *argv[],char *argv2[],int size);
int stringToInt(char* temp);
int pipeFlag=0,inputFlag=0,outputFlag=0,hist=0;
char theGecmis[512][512];

void help(){
printf("Welcome gtu shell \n");
printf("Supported commands --> \n");
printf("lsf-->\n list file type (R for regular file, S for non-regular(special) file), access rights (int the\n");
printf("form of rwxr-xr-x, , file size(bytes) and file name of all files (not directories) in\n");
printf("the present working directory. It doesn’t take any argument. \n");
printf("pwd-->\n printing the path of present working directory.\n");	
printf("cd \"path\"-->\nwhich will change the present working directory to the \"path\" provided as argument.\n");
printf("help-->\n This is your best friend when you're using gtushell \n");
printf("cat \"file\"-->\n which will print on standard output the contents of the file provided to it as argument.\n");
printf("wc \"file\"-->\n which will print on standard output the number of lines in the file provided to it as argument\n");
printf("bunedu\"file\"-->\n listing directories with sizes, with -z option showing total size of directory(with subdirectories).\n");
printf("exit-->\n exit the shell\n");

}
void signalCatcher(int sinyal)
{
  if (sinyal == SIGINT){
    printf("You pressed Ctrl+c,Dead id=%d\n",(int)getpid());
    exit(1);
   } 
   else if (sinyal == SIGTERM){
    printf("Its SIGTERM signal,Dead id=%d\n",(int)getpid());
    exit(1);
   } 
}

int parser(char *argv[],int argc){
	char myArgv[10][255];
	char input[511]="";
	int x=0,y=1,z=0,whileFlag=0;
	char command[255]="";
	while(x<argc){
		argv[x]=argv[argc];
		x++;
	}
	char* theNullPointer=argv[x];
	argv[1]=theNullPointer;
	x=0;
	char temp[255]="";
	int spaceFlag=0;
		
		strcpy(myArgv[0],"");
		read(0,&input,511);
		if(input[0]!='!'){
			strcpy(theGecmis[hist],input);
			hist++;
		}
		input[strlen(input)-1]='\0';
		while(input[x]!='\0'&&input[x]!=' '){
			command[x]=input[x];
			command[x+1]!='\0';
			x++;
		}
		strcpy(temp,command);
		argv[0]=temp;
		while(input[x]==' '){
					x++;
		}				
		while(input[x]!='\0'){
			whileFlag=1;
			while(input[x]==' '){
					x++;
					spaceFlag=1;
			}	
			if(spaceFlag==1){	
				argv[y]=myArgv[y];
				argv[y+1]=theNullPointer;
				z=0;
				spaceFlag=0;
				y++;
			}
			else{
				myArgv[y][z]=input[x];
				myArgv[y][z+1]='\0';
				z++;
				x++;
			}
						
		}
		if(whileFlag==1&&input[x]=='\0'){
				argv[y+1]=argv[y];
				argv[y]=myArgv[y];
				y++;
		}
		x=0;
		while(x<y){
			if(strcmp(argv[x],"|")==0){
				pipeFlag=1;	
			}
			else if(strcmp(argv[x],">")==0){
				outputFlag=1;	
			}
			else if(strcmp(argv[x],"<")==0){
				inputFlag=1;	
			}
			x++;
		}
		return y;	
}
int historyParser(char *argv[],char * theNullPointer){
		char myArgv[10][255];
		int x=0,y=1,z=0,whileFlag=0;
		char command[255]="";

		int a=stringToInt(argv[0]);
		if(hist-a<0){
			printf("Sorry but current history command is %d \n",hist);
			return 0;
		}
		char input[512];
		char temp[255]="";
		int spaceFlag=0;
		strcpy(input,theGecmis[hist-(a)]);
		
		input[strlen(input)-1]='\0';
		while(input[x]!='\0'&&input[x]!=' '){
			command[x]=input[x];
			command[x+1]!='\0';
			x++;
		}
		strcpy(temp,command);
		argv[0]=temp;
		while(input[x]==' '){
					x++;
		}				
		while(input[x]!='\0'){
			whileFlag=1;
			while(input[x]==' '){
					x++;
					spaceFlag=1;
			}	
			if(spaceFlag==1){	
				argv[y]=myArgv[y];
				argv[y+1]=theNullPointer;
				z=0;
				spaceFlag=0;
				y++;
			}
			else{
				myArgv[y][z]=input[x];
				myArgv[y][z+1]='\0';
				z++;
				x++;
			}
						
		}
		if(whileFlag==1&&input[x]=='\0'){
				argv[y+1]=argv[y];
				argv[y]=myArgv[y];
				y++;
		}
		x=0;
		while(x<y){
			if(strcmp(argv[x],"|")==0){
				pipeFlag=1;	
			}
			else if(strcmp(argv[x],">")==0){
				outputFlag=1;	
			}
			else if(strcmp(argv[x],"<")==0){
				inputFlag=1;	
			}
			x++;
		}
		return y;
}
int main(int argc, char *argv[]) {
	signal(SIGINT, signalCatcher);
	signal(SIGTERM, signalCatcher);
	DIR *dirp;
	int pid=0,status,x=0;
	int i=0;
	if(pid==0&&argc>1){
		fprintf(stderr, "Usage: %s without any argument \n", argv[0]);
		return -1;	
	}
	char cwd[511]="";
	int pipeFd[2];
	char * argv2[5];
	char* theNullPointer=argv[1];
	char execDirectory[511]="";
	getcwd(execDirectory,sizeof(execDirectory));
	while(1){
		x=0;
		argc=parser(argv,argc);
		if(argv[0][0]=='!'){
			argc=historyParser(argv,theNullPointer);
		}
		
	    if(strcmp(argv[0],"exit")==0){
				break;
		}
		else if(strcmp(argv[0],"cd")==0){
			if (chdir(argv[1])) {/*verilen argümanla dosya açarken hata varsa */
				perror ("Failed to open directory");
			}
			getcwd(cwd,sizeof(cwd));
			printf("%s\n",cwd);
		}
		else if(strcmp(argv[0],"help")==0){
			help();	
		}
		else{
			pid=fork();
			waitpid(pid, &status, 0);
			if(pid==0){
				if(outputFlag==0&&inputFlag==0&&pipeFlag==0){
					
					runThis(argv,execDirectory);
				}
				else if(inputFlag==1){
					int size=parseThePipe(argv,argv2,argc);
					int fd= open(argv2[0],O_RDONLY);
					char temp;
					char temp2[100];
					int j=0;
					while(read(fd,&temp, sizeof(char))==1){
						if(temp!='\n'){
							temp2[j]=temp;
							temp2[j+1]='\0';
							j++;
						}	
					}
					size=argc-size;
					size=size-1;
					argv[size]=temp2;
					argv[size+1]=theNullPointer;
					runThis(argv,execDirectory);			
					close(fd);	
				}	
				else{
					pid=0;
					pipe(pipeFd);
					int size;
					size=parseThePipe(argv,argv2,argc);	
					pid=fork();
					waitpid(pid, &status, 0);
					if(pid==0){
						close(pipeFd[0]);
						if(pipeFlag==1){
							dup2(pipeFd[1],1);
						}else if(outputFlag==1){
							dup2(pipeFd[1],1);
						}
						close(pipeFd[1]);							
						runThis(argv,execDirectory);
					}
					else{
						char temp;
						int j=0,newlineFlag=0;
						close(pipeFd[1]);
						if(pipeFlag==1){
							char temp2[1024];
							while(read(pipeFd[0],&temp, sizeof(char))==1){
								if(newlineFlag==0&&temp!='\n'){
									temp2[j]=temp;
									temp2[j+1]='\0';
									j++;
									newlineFlag=1;
								}
								else{
									temp2[j]=temp;
									temp2[j+1]='\0';
									j++;
								}		
							}
							
							close(pipeFd[0]);
							j=0;
							while(j<size){
								j++;
							}
							
							argv2[j]=temp2;
							argv2[j+1]=theNullPointer;
							runThis(argv2,execDirectory);
						}
						else if(outputFlag==1){
							mode_t mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;/*write/read permissions */	
							int fd= open(argv2[0],O_WRONLY | O_TRUNC | O_CREAT, mode);/*open file for read, if not exist create, and if exists delete data */
							char temp3[2];
							while(read(pipeFd[0],&temp, sizeof(char))==1){
								temp3[0]=temp;
								temp3[1]='\0';
								write(fd,temp3,sizeof(temp3));	
							}	
							close(pipeFd[0]);
							close(fd);
						}				
						return 0;					
					}
				}
				return 0;	
			}
			else{
				pipeFlag=0;
				inputFlag=0;
				outputFlag=0;
			}
		}	
	}
}
						
				
int parseThePipe(char *argv[],char *argv2[],int size){
	int i=0,j=0,indexPipe;	
	i=0;
	
	char* theNullPointer=argv[size];
	while(strcmp(argv[i],"|")!=0&&strcmp(argv[i],">")!=0&&strcmp(argv[i],"<")!=0)
		i++;
	indexPipe=i;	
	i++;	
	while(i<size){
		argv2[j]=argv[i];
		j++;
		i++;
	}
	argv2[j]=theNullPointer;
	argv[indexPipe]=theNullPointer;	
	
	return j;
}
void runThis(char *argv[],char* dir){
	char temp[512]="";		
	temp[0]='\0';
	if(strcmp(argv[0],"bunedu")==0){
		strcpy(temp,dir);
		strcat(temp,"/");
		strcat(temp,"BuNeDu");
		argv[0]=temp;
		execvp(argv[0],argv);
	}
	else if(strcmp(argv[0],"lsf")==0){
		strcpy(temp,dir);
		strcat(temp,"/");
		strcat(temp,"lsf");
		argv[0]=temp;
		execvp(argv[0],argv);
	}
	else if(strcmp(argv[0],"pwd")==0){
		strcpy(temp,dir);
		strcat(temp,"/");
		strcat(temp,"pwd");
		argv[0]=temp;
		execvp(argv[0],argv);
	}
	else if(strcmp(argv[0],"cat")==0){
		strcpy(temp,dir);
		strcat(temp,"/");
		strcat(temp,"cat");
		argv[0]=temp;
		execvp(argv[0],argv);
	}
	else if(strcmp(argv[0],"wc")==0){
		strcpy(temp,dir);
		strcat(temp,"/");
		strcat(temp,"wc");
		argv[0]=temp;
		execvp(argv[0],argv);
	}
}
int stringToInt(char* temp){
	int a=1,result=0;
	while(temp[a]!='\0'){
		result=(result*10)+(temp[a]-'0');
		a++;
	}
	return result;
}
