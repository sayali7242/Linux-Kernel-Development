#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int val = syscall(398);
	if (setuid_status != 0) {
		printf("Error: failed! Reason: %s\n", strerror(errno));
	}			    
	int val1 = syscall(399, 0);
	if (val1 != 0) {
		printf("Error: failed! Reason: %s\n", strerror(errno));
	}									  	
	return 0;
}
