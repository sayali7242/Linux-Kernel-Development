/* This file implements a demonstration syscall for an OS course. It
* takes one argument and prints out a simple message to the kernel
* log, indicating that it was called and showing the value of argument passed.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//Define a system call implementation that takes one argument
SYSCALL_DEFINE1( new_func2, int, singleArg ){

	// print out a simple message indicating the function was called, and return the value of parameter passed 
	printk("Someone invoked new_func2 system call"\n);
	printk("Value of parameter passed: %d\n", singleArg);
	return 0;  
}

//End of file
