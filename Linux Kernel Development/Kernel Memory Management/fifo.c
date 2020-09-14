#include<stdio.h>
#include<unistd.h> 
#include<stdlib.h> 
#include<string.h> 
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h> 
#define MY_FIFO "My_Fifo"

int main(){
	char str[400];
	int num;
	int num_reads;
	mkfifo(MY_FIFO, S_IRUSR | S_IWUSR); 
	
	int fifo = open(MY_FIFO, O_RDONLY);
	if(fifo == -1){
		fprintf(stderr, "Operation Failed: Could not open FIFO\n");
		unlink(MY_FIFO);
		return 1;
	}
	
	while(1){
		num_reads = read(fifo, str, sizeof(str));
		if(num_reads == 0){
			break;
		}	
		else{
			printf("The string is: %s", str);
		}
	}
	int fifo_w = open(MY_FIFO, O_WRONLY);
	return 0;
}
