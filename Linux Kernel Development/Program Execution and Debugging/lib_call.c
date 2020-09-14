#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	/* read and print the uid */
    int old_uid = getuid();
	printf("Current UID: %d\n", old_uid);
			    
	/* try to change the uid */
	int setuid_status = setuid(0);
	if (setuid_status != 0) {
		printf("Error: setuid failed! Reason: %s\n", strerror(errno));
	}
	else {
		printf("UID successfully changed!\n");
	}
									  	
	/* read and print the uid */
    int changed_uid = getuid();
	printf("Changed UID: %d\n", changed_uid); 
	return 0;
}
