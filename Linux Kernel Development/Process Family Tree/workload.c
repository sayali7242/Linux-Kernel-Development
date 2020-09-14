/*    Simulated workload using OpenMP
 *
 * This program will create some number of seconds of work on each processor
 * on the system.
 *
 * This program requires the use of the OpenMP compiler flag, and that 
 * optimizations are turned off, to build correctly. E.g.: 
 * gcc -fopenmp workload.c -o workload
 */

#define _GNU_SOURCE
#include <stdio.h> // for printf()
#include <sched.h> // for sched_getcpu()

// 500 million iterations should take several seconds to run
#define ITERS 500000000

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

int main (int argc, char* argv[]){

	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		critical_section();

		printf("CPU %d finished!\n", sched_getcpu());
	}

	return 0;
}
