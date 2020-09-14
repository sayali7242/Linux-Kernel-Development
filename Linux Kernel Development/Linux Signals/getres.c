#include <stdio.h>
#include <unistd.h>
#include <time.h>
#define SIZE 8
int main()
{
        clockid_t clocks[SIZE] = { CLOCK_REALTIME, CLOCK_REALTIME_COARSE, CLOCK_MONOTONIC, CLOCK_MONOTONIC_COARSE, CLOCK_MONOTONIC_RAW, CLOCK_BOOTTIME, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID};

        struct timespec tspec;
        int i=0;
        for(i; i < SIZE; i++)
        {
                if( clock_getres(clocks[i], &tspec) == 0)
                {
                        printf("Timer: %d, Seconds: %ld, Nanos: %ld\n", i, tspec.tv_sec, tspec.tv_nsec);
                }
                else
                {
                        printf("Timer %d is not supported for this operation. \n", clocks[i]);
                }
        }

}
