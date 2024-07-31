#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include<errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<ctype.h>
#include<signal.h>
char* newstr(char *cmd) {
    int len = strlen(cmd);
    static char *dup;
    char *dup_offset;
    dup = (char *)malloc(sizeof(char)*len+1);
    dup_offset = dup;
    while(*cmd)
    {
        *dup_offset = *cmd;
        dup_offset++;
        cmd++;
    }
    *dup_offset = '\0';

    return(dup);
}


void sigint_handler(int signo) {
	printf("Signal Caught");
}
int parsenorm(char *cmd) {
	char *tok;
	char *args[128];
	//char arg[128][128];
	tok = strtok(cmd, " ");
	int i = 0,err;
	while(tok != NULL) {
		args[i] = tok;
		tok = strtok(NULL, " ");
		i++;
	}
	//printf("i = %d\n", i);
	args[i] = NULL;
	args[i-1][strlen(args[i-1]) - 1] = '\0';
	//signal(SIGINT, SIG_DFL);	
	err = execvp(args[0],args);
	if(err == -1){
		perror("ERROR");
	}
	return err;
}

void parsein(char *cmd) {
	char args[128][128];
	char *temp,*tok, *tok1;
	char *argv[128];
	temp = newstr(cmd);
	tok = strtok(temp,"<");
	int i = 0;
	while(tok != NULL) {
		strcpy(args[i++], tok);
		tok = strtok(NULL, "<");
	}
	args[1][strlen(args[1]) -1] ='\0';
	strcpy(temp, args[1]);
	while(isspace(*temp)) {
		temp++;
	}
	
	args[i-1][strlen(args[i-1]) -1] ='\0';
	char *file = newstr(temp);
	char *temp1 = args[0];
	tok1 = strtok(temp1, " ");
	int k = 0;
 	while(tok1 != NULL) {
		argv[k++] = tok1;
		tok1 = strtok(NULL, " ");
	}
	//argv[i-1][strlen(argv[i -1]) -1] = '\0';
	argv[i] = NULL;	
	int fd3 = open(file, O_RDONLY);
	if(fd3 < 0){
		perror(file);
		return;
	}
	
	dup2(fd3,0);
	close(fd3);
	//ignal(SIGINT, SIG_DFL);
	execvp(argv[0], argv);
}

void parseout(char *cmd) {
	char args[128][128];
	char *temp,*tok, *tok1;
	char *argv[128];
	temp = newstr(cmd);
	tok = strtok(temp,">");
	int i = 0;
	while(tok != NULL) {
		strcpy(args[i++], tok);
		tok = strtok(NULL, ">");
	}
	args[1][strlen(args[1]) -1] ='\0';
	strcpy(temp, args[1]);
	while(isspace(*temp)) {
		temp++;
	}
	args[i-1][strlen(args[i-1]) -1] ='\0';
	char *file = newstr(temp);
	char *temp1 = args[0];
	tok1 = strtok(temp1, " ");
	int k = 0;
 	while(tok1 != NULL) {
		argv[k++] = tok1;
		tok1 = strtok(NULL, " ");
	}
	//argv[i-1][strlen(argv[i -1]) -1] = '\0';
	argv[k] = NULL;	
	int fd3 = creat(file, 0644);
	dup2(fd3,1);
	close(fd3);
	//signal(SIGINT, SIG_DFL);
	execvp(argv[0], argv);
}

int proc(char *temp, int fd3, int first, int last) {

	//pipe having input redirection
	if(strchr(temp, '<')) {
		char args[128][128];
		char *cmd,*tok, *tok1;
		char *argv[128];
		cmd = newstr(temp);
		//printf("%s", cmd);
		tok = strtok(cmd,"<");
		int i = 0;
		while(tok != NULL) {
			strcpy(args[i++], tok);
			tok = strtok(NULL, "<");
		}
		args[1][strlen(args[1]) -1] ='\0';
		//printf("%s\n",args[1]);
		strcpy(cmd, args[1]);
		while(isspace(*cmd)) {
			cmd++;
		}
		
		args[i-1][strlen(args[i-1]) -1] ='\0';
		char *file = newstr(cmd);
		//printf("%s",file);
		char *temp1 = args[0];
		tok1 = strtok(temp1, " ");
		int k = 0;
	 	while(tok1 != NULL) {
			argv[k++] = tok1;
			tok1 = strtok(NULL, " ");
		}
		//argv[i-1][strlen(argv[i -1]) -1] = '\0';
		argv[k] = NULL;	
		//while(i--)
			//printf("%s \n",argv[i]);
		//close(0);
		int fd4 = open(file, O_RDONLY);
		//printf("%s", file);
		dup2(fd4,0);
		//close(fd4);
		//int x = execvp(argv[0], argv);
		int pid;
		int pfd[2];
		pipe(pfd);
		pid = fork();
		if(pid == 0){
			if(first == 1 && last == 0 && fd3 == 0){
				//close(1);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else if(first == 0 && last == 0 && fd3 != 0){
				dup2(fd3, STDIN_FILENO);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else{
				//close(0);
				dup2(fd3, STDIN_FILENO);
			}
			printf("done executing");	
			int x = execvp(argv[0], argv);
			printf("done executing");
		}
		else {
			wait(0);
		}
		if(fd3 != 0){
			close(fd3);
		}
		close(pfd[1]);
		if(last == 1){
			close(pfd[0]);
		}
		close(fd4);
		return pfd[0];

	}

	//pipe having output redirection.
	else if(strchr(temp, '>')) {
		char args[128][128];
		char *temp,*tok, *tok1;
		char *argv[128];
		temp = newstr(temp);
		tok = strtok(temp,">");
		int i = 0;
		while(tok != NULL) {
			strcpy(args[i++], tok);
			tok = strtok(NULL, ">");
		}
		args[1][strlen(args[1]) -1] ='\0';
		strcpy(temp, args[1]);
		while(isspace(*temp)) {
			temp++;
		}
		args[i-1][strlen(args[i-1]) -1] ='\0';
		char *file = newstr(temp);
		char *temp1 = args[0];
		tok1 = strtok(temp1, " ");
		int k = 0;
	 	while(tok1 != NULL) {
			argv[k++] = tok1;
			tok1 = strtok(NULL, " ");
		}
		//argv[i-1][strlen(argv[i -1]) -1] = '\0';
		argv[k] = NULL;	
		int fd4 = creat(file, 0644);
		
		dup2(fd4,1);
		close(fd4);
		int x = execvp(argv[0], argv);
		int pid;
		int pfd[2];
		pipe(pfd);
		pid = fork();
		if(pid == 0){
			if(first == 1 && last == 0 && fd3 == 0){
				close(1);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else if(first == 0 && last == 0 && fd3 != 0){
				dup2(fd3, STDIN_FILENO);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else{
				close(0);
				dup2(fd3, STDIN_FILENO);
			}	
			int x = execvp(argv[0], argv);
			//printf("done executing");
			//printf("%d", x);
		}
		else {
			wait(0);
		}
		if(fd3 != 0){
			close(fd3);
		}
		close(pfd[1]);
		if(last == 1){
			close(pfd[0]);
		}
		close(fd3);
		//int x = pfd[0];
		//close(fd3);
		return pfd[0];
	}
	
	//pipe without redirections.
	else {
		int i = 0, n;
		char *args[128];
		temp[strlen(temp) - 1] = ' ';
		char *token = strtok(temp, " ");
			
		while(token != NULL){
			args[i] = token;
			token = strtok(NULL, " ");
			i++;
		}
		args[i] = NULL;
		int pid;
		int pfd[2];
		pipe(pfd);
		pid = fork();
		if(pid == 0){
			if(first == 1 && last == 0 && fd3 == 0){
				close(1);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else if(first == 0 && last == 0 && fd3 != 0){
				dup2(fd3, STDIN_FILENO);
				dup2(pfd[1], STDOUT_FILENO);
			}
			else{
				close(0);
				dup2(fd3,STDIN_FILENO);
			}	
			int x = execvp(args[0], args);
		}
		else {
			wait(0);
		}
		if(fd3 != 0){
			close(fd3);
		}
		close(pfd[1]);
		if(last == 1){
			close(pfd[0]);
		}
		return pfd[0];
	}
	//return pfd[0];
}
		
int main() {
	int pid, count =0, x;
	char cmd[128];
    signal(SIGINT, sigint_handler);
   // signal(SIGTSTP,sigint_handler);

	while(1) {
		char *init;
		printf("prompt>");
		fgets(cmd,128,stdin);
		int fd3 =0;
		int first = 1;
		char* temp = cmd;
		if(strchr(temp, '|')) { 
			init = strchr(temp, '|');
			while(init != NULL) {
				*init = '\0';
				fd3 = proc(temp, fd3, first, 0);
				temp = init + 1;
				init = strchr(temp, '|');
				first = 0;
			}
			fd3 = proc(temp, fd3, first, 1);
		}
		else {
	
			pid = fork();
			if(pid == 0) {
				//signal(SIGINT, SIG_DFL);
				if(strchr(cmd,'<'))
					parsein(cmd);
				else if(strchr(cmd, '>'))
					parseout(cmd);
				else
					parsenorm(cmd);
				

			} 
			else {	
				wait(0);
			}
		}
	}
	return 0;
}

