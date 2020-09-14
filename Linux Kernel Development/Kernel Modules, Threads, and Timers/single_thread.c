/*
This program monitors the operation of the system using kernel modules, kernel timers, and kernel threads. The following program is written to monitor the system in a single threaded environment.
*/

#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/ktime.h>
#include<linux/hrtimer.h>
#include<linux/kthread.h>
#include<linux/module.h> 

// Define module parameters with default values to be able to govern how frequently the module will schedule or reschedule a repeating time. Default values of the module parameter make sure that in the event that the module is loaded without parameters, the timer defaults to expiring every second.

static unsigned long long_sec = 1;
static unsigned long long_nsec = 0;
module_param(long_sec, ulong, 0);
module_param(long_nsec, ulong, 0);

// Define static variables for time and timer 

static ktime_t kt;
static struct hrtimer timer;

// Define structure pointer of type task_struct

static struct task_struct *k_task; 

// Static function for timer's expiration

static enum hrtimer_restart fun_timer (struct hrtimer *timer)
{
	wake_up_process(k_task); 
	hrtimer_forward_now(timer, kt);
	printk(KERN_INFO "Restarting the timer\n");
	return HRTIMER_RESTART; 
}

// Function for spawning the thread every time it is awakened by the module

static int fun_kthread (void *f)
{
	printk(KERN_ALERT "running fun_kthread\n");
	while(1) {
		printk(KERN_DEBUG "fun_kthread has started another iteration, nvcsw: %lu, nivcse: %lu\n", current->nvcsw, current->nivcsw);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (kthread_should_stop()) {
			break;
		}	
	}
	printk(KERN_ALERT "fun_kthread terminated\n");
	return 0; 
}


// Define module initialization function

static int timer_init(void) 
{
	printk(KERN_ALERT "Initializing kernel module\n");
	kt = ktime_set(long_sec, long_nsec);
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL); 
	timer.function = fun_timer;
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL); 
	k_task = kthread_run(fun_kthread, NULL, "kernel thread 1");  
	return 0;
}

// Define module exit function

static void timer_exit(void)
{
	hrtimer_cancel(&timer);
	printk(KERN_ALERT "Unloading module\n");
	if (!IS_ERR(k_task)) {
		kthread_stop(k_task);
	}
}

// Define module general and functional description

module_init(timer_init);
module_exit(timer_exit); 
MODULE_DESCRIPTION ("Lab1 - single thread (CSE 422S) by Priyanshu Jain, Sayali Patil, Shadi Davari");
MODULE_LICENSE("GPL");
