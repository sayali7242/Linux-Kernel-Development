#include<stdio.h> 
#include<stdlib.h> 
#include<sys/time.h> 
#include<sys/select.h>
#include<string.h> 

int main(int argc, char *argv[]){
	fd_set readfs;
	struct timeval time; 
	int return_val, len, i; 
	char str[200];
	char end[]= "quit"; 
	char *s1, *s2;

	FD_ZERO(&readfs); 
	FD_SET(0, &readfs); 
	
	time.tv_sec = 10; 
	time.tv_usec = 0; 

	return_val = select(1, &readfs, NULL, NULL, &time); 

	if(return_val == -1){
		perror("Select"); 
		exit(EXIT_FAILURE);
	}
	else if(return_val){
		fgets(str, sizeof(str), stdin); 
		len = strlen(str) - 1; 
		if(str[len] == '\n'){
			str[len] = '\0';
		}
		s1 = str; 
		s2 = end;
 		for(i = 0; i < len; i++){
			if(*s1 == *s2){
				exit(EXIT_SUCCESS);
			}
		}
		printf("The string is: %s\n", s1);
	}
	return 0;
}
