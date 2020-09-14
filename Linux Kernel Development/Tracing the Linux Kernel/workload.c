#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
	const int loop_index = 500000000;
        if(argc != 2)
        {
                printf("Error: Program takes single integer argument\n");
                exit(-1); 
        }       
        int cpu_core_num = atoi(argv[1]);
        cpu_set_t cpu; 
        CPU_ZERO(&cpu);
        CPU_SET(cpu_core_num, &cpu);
        sched_setaffinity(0, sizeof(cpu), &cpu); 
        int i=0;
	while(i<= loop_index) {i++;}
        return 0;
}

