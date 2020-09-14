/*Simulated workload using OpenMP
 *
 *This program will create some number of seconds of work on each processor
 *on the system.
 *      
 *This program requires the use of the OpenMP compiler flag, and that 
 *optimizations are turned off, to build correctly. E.g.: 
 *gcc -fopenmp workload.c -o workload
*/

#define _GNU_SOURCE
#include<stdio.h> // for printf()
#include<sched.h> // for sched_getcpu()
#include<stdbool.h>
#include<unistd.h>
#include<linux/futex.h>
#include<sys/syscall.h>
#include<limits.h>
#include<sys/times.h>
#define LOCKED 0
#define UNLOCKED 1

volatile int state = UNLOCKED; 

// 500 million iterations should take several seconds to run

#define ITERS 500000000

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

void lock(volatile int *ptr){
	int ret_val = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	while(ret_val < 0){
		__atomic_store_n(ptr, -1, __ATOMIC_RELEASE);
	    	syscall(SYS_futex, ptr, FUTEX_WAIT, -1, NULL);	
		ret_val = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	}
	usleep(1);
	printf("Lock acquired by CPU: %d\n", sched_getcpu());
}

void unlock(volatile int *ptr){
        int ret_val = __atomic_add_fetch(ptr, 1, __ATOMIC_ACQ_REL);
        if(ret_val != 1){
                __atomic_store_n(ptr, 1, __ATOMIC_RELEASE);
                syscall(SYS_futex, ptr, FUTEX_WAKE, INT_MAX);
        }
        printf("Lock released by CPU: %d\n", sched_getcpu());
}

int main (int argc, char* argv[]){
	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		lock(&state);
		critical_section();
		unlock(&state);
 		printf("CPU %d finished!\n", sched_getcpu());
	}
	return 0;
}
