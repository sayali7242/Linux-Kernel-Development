#include<stdio.h> 
#include<unistd.h> 
#include<stdlib.h>
#include<sys/types.h> 
#include<sys/socket.h> 
#include<sys/un.h>

int main(){
	const char *Socket_name = "/tmp/my_socket";
	struct sockaddr_un address;
	int fds, new_sock;
	FILE *fp; 
	int num[10]; 
	fds = socket(AF_UNIX, SOCK_STREAM, 0);
	
	if(fds == -1){
		perror("Socket");
		exit(1);
	}	
	
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX; 
	strncpy(address.sun_path, Socket_name, sizeof(address.sun_path) -1);
	if(bind(fds, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) == -1){
		perror("Bind");
		exit(1);
	}

	if(listen(fds, 5) == -1){
		perror("Listen");
		exit(1);
	}
	
	for(;;){
		new_sock = accept(fds, NULL, NULL);
		if(new_sock == -1){
			perror("Accept");
			exit(1);
		}
		fp = fdopen(new_sock, "r");
		if(fp == NULL){
			printf("No integer given\n");
			return 0;
		}
		
		while(fscanf(fp, "%d", &num) == 1){
			printf("Numbers are: %d\n", num);
		}
	}
	close(fds);
	return 0;
}
