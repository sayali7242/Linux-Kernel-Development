/*
 * This kernel module creates one thread on each core of our Raspberry pi
 */

#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/kthread.h> 
#include<linux/module.h>
#define iters 1000000

struct task_struct *kthread[4];
volatile int shared_data = 0;

static int kthread_do_work(void *func){
	int i;
	printk(KERN_INFO "Kernel function is running on CPU: %d\n", smp_processor_id());
	while(1){
		for( i=0; i<iters; i++){
			shared_data++;
		}
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if(kthread_should_stop())
			break;
	}
	printk(KERN_INFO "Kernel function finished execution on CPU: %d\n", smp_processor_id());
	return 0; 
}

static int init_function(void){
	int i;
	printk(KERN_INFO "Module initialization started\n"); 
	for(i = 0; i < 4; i++){
		kthread[i] = kthread_create(kthread_do_work, NULL, "Kernel Thread: %d", i); 
		kthread_bind(kthread[i], i);
		wake_up_process(kthread[i]);
	}
	return 0;
}

static void exit_function(void){
	int i; 
	printk(KERN_INFO "Module exit process started"); 
	for(i = 0; i < 4; i++){
		kthread_stop(kthread[i]);
	}
	printk(KERN_INFO "shared_data value is: %d\n", shared_data);
	printk(KERN_INFO "Module exit process finished");
} 

module_init(init_function);
module_exit(exit_function);

MODULE_DESCRIPTION ("CSE 422S - Studio 10 by Sayali Patil");
MODULE_LICENSE("GPL");

