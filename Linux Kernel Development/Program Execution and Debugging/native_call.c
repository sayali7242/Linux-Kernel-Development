#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	/* read and print the uid */
	printf("Current UID: %d\n", syscall(__NR_getuid));
			    
	/* try to change the uid */
	int setuid_status = syscall(__NR_setuid, 0);
	if (setuid_status != 0) {
		printf("Error: setuid failed! Reason: %s\n", strerror(errno));
	}
	else {
		printf("UID successfully changed!\n");
	}
									  	
	/* read and print the uid */
    	printf("Changed UID: %d\n", syscall(__NR_getuid)); 
	return 0;
}
