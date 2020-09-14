#include<stdio.h> 
#include<unistd.h> 
#include<stdlib.h>
#include<sys/types.h> 
#include<sys/socket.h> 
#include<sys/un.h>

struct myIntegers{
	int num[10];
};
int main(){
	const char *Socket_name = "/tmp/my_socket";
	struct sockaddr_un address;
	int fds; 
	int i;
	struct myIntegers msg;
	fds = socket(AF_UNIX, SOCK_STREAM, 0);
	
	if(fds == -1){
		perror("Socket");
		exit(1);
	}	
	
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX; 
	strncpy(address.sun_path, Socket_name, sizeof(address.sun_path) -1);
	if(connect(fds, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) == -1){
		perror("Connect");
		exit(1);
	}
	for(i = 0; i < 10; i++){
		scanf("%d", &(msg.num));
	}
	send(fds, &(msg.num), sizeof(msg.num), 0);
	for(i=0; i < 10; i++){
		printf("Numbers are: %d\n", msg.num);
	}
	close(fds);
	return 0;
}
