/*
 * This lab creates a kernel module that (1) allocates kernel memory for an array of integers from 2 up to a specified upper bound; (2) initializes that memory to contain those integers in ascending order; (3) spawns a specified number of kernel threads, which will then go through the array concurrently, "crossing out" numbers that are not primes by setting them to zero; and (4) once all the threads have comp_finished, prints out the remaining prime numbers along with several statistics about the module's performance.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <asm/spinlock.h>
#include <linux/time.h>
#include <linux/atomic.h>
#include <linux/slab.h>

//Define module parameters and set their default values 
static unsigned long num_threads = 1; 
static unsigned long upper_bound = 10; 
module_param(num_threads, ulong, 0);
module_param(upper_bound, ulong, 0);

//Define a pointer to an array of counter variables 
int *counter_vars; 

//Define a pointer to the array of integers 
int *integers; 

//Define an integer for the position of the current number being processed
int current_pos;

//Define atomic variable to indicate whether the prime computation has finished or not and for barrier syncs
atomic_t comp_finished; 
atomic_t barrier1_sync;
atomic_t barrier2_sync;

//Define int variable to keep track of the number of threads reaching barriers
int thread_barrier1;
int thread_barrier2;

//Define spinlock
DEFINE_SPINLOCK(spinlock1);
DEFINE_SPINLOCK(spinlock2);
DEFINE_SPINLOCK(lock1);
DEFINE_SPINLOCK(lock2);

//Define kernel thread of type threads_struct 
struct task_struct **threads; 

//Define strcuture of type timespec to record time
struct timespec time0, time1, time2; 

//Define barrier fuctions for two barriers respectively
static void barrier1_function(void){
	spin_lock(&lock1);
	thread_barrier1++;
	if(thread_barrier1 == num_threads){
		ktime_get_ts(&time1);
		printk(KERN_ALERT "First barrier reached\n");
		atomic_set(&barrier1_sync, 1);
	}
	spin_unlock(&lock1);
}

static void barrier2_function(void){
        spin_lock(&lock2);
        thread_barrier2++;
        if(thread_barrier2 == num_threads){
                ktime_get_ts(&time2);
                printk(KERN_ALERT "Second barrier reached\n");
                atomic_set(&barrier2_sync, 1);
        }
        spin_unlock(&lock2);
}


//Define a function to mark non-prime numbers based on Sieve of Eratosthenes algorithm 
static void non_primes(int *counter){
	int i, position, num;
	while(1){
		spin_lock(&spinlock1);
		position = current_pos;
		current_pos++;
		while (current_pos < upper_bound-1){
			if(integers[current_pos] != 0)
				break;
			else
				current_pos++;
		}	
		spin_unlock(&spinlock1);
		if (position >= upper_bound-1) 
			return;
		else{
			num = integers[position];
			for(i = position + num; i < upper_bound-1; i = i + num){
				spin_lock(&spinlock2);
				integers[i] = 0;
				spin_unlock(&spinlock2);
				(*counter)++;
			}
		}
	}
}

//Define a function to run on all the spawned threads
static int thread_do_work(void *counters){
	barrier1_function();
	while(atomic_read(&barrier1_sync) == 0);

	non_primes(counters);

	barrier2_function();
	while(atomic_read(&barrier2_sync) == 0);
	
	atomic_add(1, &comp_finished);
	return 0;
}

//Define module initialization function
static int init_function(void){
	int i;
	printk(KERN_ALERT "Module initialization started\n");
	ktime_get_ts(&time2);
	
	//Initialize all the module variables
	atomic_set(&comp_finished, 0);
	counter_vars = 0;
	integers = 0;
	//Set atomic variables to show that computation has completed 
	atomic_set(&comp_finished, num_threads);

	//Check for the validity of module parameters
	if(num_threads < 1 || upper_bound < 2){
		printk(KERN_ALERT "Invalid module parameter(s) entered:\n");
		if (num_threads < 1)
			printk(KERN_ALERT "Number of threads should be greater than or equal to 1\n");
		if (upper_bound < 2)
			printk(KERN_ALERT "Value of upper bound should be greater than or equal to 2\n");
		return 0;
	}
	
	//Allocate kernel memory for integers and check if the allocation was successful. If not, set all the module variables back to zero.
	integers = (int *)kmalloc(sizeof(int)*(upper_bound-1), GFP_KERNEL);
	if(!integers){
		printk(KERN_ALERT "Memory allocation for integers failed\n");
		counter_vars = 0;
		integers = 0;
		return 0;
	}
	
	//Allocate kernel memory for counter variables and check if the allocation was successful. If not, set all the module variables back to zero and free the memory allocated to integers 
	counter_vars = (int *)kmalloc(sizeof(int)*num_threads, GFP_KERNEL);
	if(!counter_vars){
		printk(KERN_ALERT "Memory allocation for counter variables failed\n");
		kfree(integers);
		counter_vars = 0;
		integers = 0;
		return 0;
	}

	//Set the counter variables per thread to zero
	for(i = 0; i < num_threads; i++){
		counter_vars[i] = 0;
	}
	
	//Set the integer array to from 2 till the upper bound
	for(i = 0; i < upper_bound-1; i++) {
		integers[i] = i + 2;
	}
	
	//set the current position to the position that contains integer 2 
	current_pos = 0; 

	//Set the atomic variables to show that the processing is not finished yet
	atomic_set(&barrier1_sync, 0);
	atomic_set(&barrier2_sync, 0);

	//Spawn kernel threads equal in number as value passed in the parameter num_threads
	threads = kmalloc(sizeof(struct task_struct*)*num_threads, GFP_KERNEL);
	for(i=0; i < num_threads; i++){
		threads[i] = kthread_create(thread_do_work, (void *)&counter_vars[i], "Kernel thread(%d)", i);
		wake_up_process(threads[i]);
	}
	return 0;
}

//Define a function to get elapsed time
static struct timespec elapsed_time(struct timespec *t2, struct timespec *t1){
	struct timespec time_diff;
	time_diff.tv_sec = (*t2).tv_sec - (*t1).tv_sec;
	time_diff.tv_nsec = (*t2).tv_nsec - (*t1).tv_nsec;
	if (time_diff.tv_nsec < 0) {
		time_diff.tv_nsec += 1000000000L;
		time_diff.tv_sec--;
	}
	return time_diff;
}

//Define module exit function
static void exit_function(void){
	int i;
	
	//Total number of primes
	int primes = 0;
	
	//Total number of non-primes
	int nonprimes = 0; 
	
	//Total number of crossouts
	int crossouts = 0;
	
	//Total number of unnecessary crossouts 
	int un_crossouts = 0;
	
	//Timespec structures to get elapsed times
	struct timespec elapse1, elapse2, total_elapse; 
	
	//Check if the computation has finished
	printk(KERN_ALERT "Module exit process started\n");
	if (atomic_read(&comp_finished) != num_threads){
		printk(KERN_ALERT "Prime computation isn't finished yet\n");
		return;
	}

	//Deallocate the kernel memory missed by init function
	if(!integers){
		printk(KERN_ALERT "Memory allocation for integers failed\n");
		counter_vars = 0;
		integers = 0;
		threads = 0;
		return;
	}
	if(!counter_vars){
		printk(KERN_ALERT "Memory allocation for counter variables failed\n");
		kfree(integers);
		counter_vars = 0;
		integers = 0;
		threads = 0;
		return;
	}

	//Calculate the total number of primes
	for(i = 0; i < upper_bound-1; i++){
		if(integers[i] != 0){
			printk(KERN_CONT "%d ", integers[i]);
			primes++;
			if (primes % 8 == 0) {
				printk(KERN_CONT "\n");
			}
		}
	}
	
	//Calculate the total number of non-primes
	nonprimes = upper_bound - 1 - primes;

	//Calculate the total number of crossouts
	for(i = 0; i < num_threads; i++){
		crossouts += counter_vars[i];
	}
	
	//Calculate the total number of unnecessary crossouts
	un_crossouts = crossouts - nonprimes;
	
	//Print the total number of primes, non-primes, and unnecessary crossouts
	printk(KERN_INFO "Total count of prime numbers: %d\n Total count of non-prime numbers: %d\n Total number of unnecessary crossouts: %d\n" , primes, nonprimes, un_crossouts);
	
	//Print the values of module parametes
	printk(KERN_INFO "Number of threads: %lu\n Upper bound: %lu\n", num_threads, upper_bound);
	
	//Calculate the time of initialization and print
	elapse1 = elapsed_time(&time1, &time0);
	printk(KERN_INFO "Time taken in module initialization: %ld.%.9ld\n", elapse1.tv_sec, elapse1.tv_nsec);
        
	//Calculate the time of prime computation and print
	elapse2 = elapsed_time(&time2, &time1);
	printk(KERN_INFO "Time taken in prime computation: %ld.%.9ld\n", elapse2.tv_sec, elapse2.tv_nsec);
        
	//Calculate the total time taken by the module and print
	total_elapse = elapsed_time(&time2, &time0);	
	printk(KERN_INFO "Total time taken by the module: %ld.%.9ld\n", total_elapse.tv_sec, total_elapse.tv_nsec);
	
	//Free up the memory
	kfree(integers);
	kfree(counter_vars);
	integers = 0;
	counter_vars = 0;
	return;
}

module_init(init_function);
module_exit(exit_function);

MODULE_DESCRIPTION ("Lab2(CSE 422S): Primes computation by Sayali Patil");
MODULE_LICENSE("GPL");






