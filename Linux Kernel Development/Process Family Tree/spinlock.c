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
#define LOCKED 0
#define UNLOCKED 1

volatile int state = UNLOCKED; 

// 500 million iterations should take several seconds to run

#define ITERS 500000000

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

bool compare(volatile int* ptr, int pe, int pd){
	int *per = &pe; 
	int *pdr = &pd; 
	if(__atomic_compare_exchange(ptr, per, pdr, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
		return true; 
	else 
		return false;

}
void unlock(volatile int *ptr){
	if(!compare(ptr, LOCKED, UNLOCKED)){
		printf("Error: Unlocking failed on CPU: %d\n", sched_getcpu());
		return;
	}
	printf("Unlocking successful on CPU: %d\n", sched_getcpu());
}

void lock(volatile int *ptr){
	while(!compare(ptr, UNLOCKED, LOCKED));
	usleep(1);
	printf("Lock acquired by CPU: %d\n", sched_getcpu());
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
