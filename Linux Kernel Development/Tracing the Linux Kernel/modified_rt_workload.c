#define _GNU_SOURCE
#include<sched.h> 
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[])
{
        const int loop_index = 500000000;
        if (argc != 4)
        {
                printf("ERROR! Program takes three integer arguments\n");
                exit(-1);
        }
        int cpu_core_num = atoi(argv[1]); 
        int rt_priority = atoi(argv[2]); 
	int num_of_tasks = atoi(argv[3]);
	
        if (rt_priority < sched_get_priority_min(SCHED_RR) || rt_priority > sched_get_priority_max(SCHED_RR))
                printf("This process is not the approved priority range\n");
        if (num_of_tasks >10 || num_of_tasks <0)
		printf("Number of tasks is out of the range (1-10)");
	struct sched_param prm;
        prm.sched_priority = rt_priority;
        int rt = sched_setscheduler(0, SCHED_RR, &prm); 
        if(rt == -1)
                perror("Error: operation sched_setscheduler failed!\n");
        cpu_set_t cpu;
        CPU_ZERO(&cpu); 
        CPU_SET(cpu_core_num, &cpu); 
        sched_setaffinity(0, sizeof(cpu), &cpu); 
	while (num_of_tasks >1)
	{
		int fork_process = fork(); 
		num_of_tasks--;
		if (fork_process == 0)
			break;
	}
        int i = 0;
        while (i <= loop_index) { i++; }
        return 0;
}

