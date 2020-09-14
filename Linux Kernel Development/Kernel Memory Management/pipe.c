#include<stdio.h> 
#include<unistd.h>
#include<sys/types.h>
#include<string.h> 
#include<stdlib.h>

int main(){
	int fds[2]; 
	char str[400]; 
	pid_t proc; 
	
	if(pipe(fds) == -1){
		fprintf(stderr, "Creation of pipe failed\n"); 
		return 1;
	}
	
	
	scanf("%s", str);
	int str_len = strlen(str);

	proc = fork();
	
	if(proc < 0){
		fprintf(stderr, "Creation of child process failed\n");
	}
	else if(proc > 0){
		close(fds[0]);
		write(fds[1], str, str_len+1);
		close(fds[1]);
	}
	else{
		char str2[400];
                close(fds[1]);
                read(fds[0], str2, 400);
		printf("The string is: %s\n", str2);
                exit(0);
        }
}

