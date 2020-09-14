Lab 1 README.txt


Students: 
Sayali Patil, sayali.patil@wustl.edu
Shadi Davari, shadidavari@wustl.edu
Priyanshu Jain, priyanshu@wustl.edu
 
Module Design
To design our modules, we used the module_param() macro, defined in linux/module.param.h, which takes 3 arguments. The first variable is the name of the variable seen by the user and the variable which holds the parameter inside the module. The second variable is the type of the variable. The third variable is an argument which specifies the permissions of the corresponding file in sysfs. The macro does not declare the variable for us, so it’s important to note that we must do that before using the macro.
 
We first declared appropriate default value,
static unsigned long long_sec = 1;
static unsigned long long_nsec = 0;
which defaults the timer to expiring once every second. We then defined our two ulong module parameters using the macro:
module_param(long_sec, ulong, 0);
module_param(long_nsec, ulong, 0);
where one is called long_sec and the other is called long_nsec and they are both of type ulong, per the instructions. We set the third argument to be 0 because a value of 0 disables the sysfs entry entirely.
 
Timer Design and Evaluation
To start with, we defined a static variable for ktime_t and a static enum hrtimer. These variables were used to enumerate and reschedule the defined timer’s next expiration and adding one time interval specific to the timer. The function defined for this operation takes a pointer to the static struct hrtimer as its only parameter and returns HRTIMER_RESTART letting the kernel know that the timer is being restarted. Inside the init() function, unsigned long variables (long_sec and long_nsec) are passed to the ktime_set() and the result of this operation is stored in the static variable defined for ktime_t. After that we initialized the hrtimer with the inbuilt hrtimer_init() function using the clock type CLOCK_MONOTONIC. In the exit function of this module we are storing the value  returned by hrtime_cancel() and using the same value we are checking if the time was still running before the termination.
 
Timing
Example 1 – Repeat every 1 second, average deviation of 0.00005449 seconds
pi@pikachan:~ $ sudo insmod single_thread.ko
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ dmesg
[ 3325.016568] Initializing kernel module
[ 3326.016617] Restarting the timer
[ 3327.016620] Restarting the timer
[ 3328.016627] Restarting the timer
[ 3329.016637] Restarting the timer
[ 3330.016637] Restarting the timer
[ 3331.016642] Restarting the timer
[ 3332.016642] Restarting the timer
[ 3333.016653] Restarting the timer
[ 3334.016658] Restarting the timer
         [ 3335.016663] Restarting the timer
[ 3336.016669] Restarting the timer
[ 3337.016675] Restarting the timer
[ 3338.016682] Restarting the timer
[ 3339.016690] Restarting the timer
[ 3340.016694] Restarting the timer
[ 3341.016702] Restarting the timer
[ 3342.016707] Restarting the timer
[ 3343.016712] Restarting the timer
[ 3344.016715] Restarting the timer
[ 3345.016724] Restarting the timer
[ 3346.016726] Restarting the timer


Example 2 – Repeat every 2.5 seconds, average deviation of 0.0000135 seconds (calculated on first 5 after initialization)
pi@pikachan:~ $ sudo insmod single_thread.ko long_sec=2 long_nsec=500000000
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ tail -f /var/log/syslog
Sep 28 15:54:35 pikachan kernel: [ 4920.619504] Initializing kernel module
Sep 28 15:54:38 pikachan kernel: [ 4923.119562] Restarting the timer
Sep 28 15:54:40 pikachan kernel: [ 4925.619580] Restarting the timer
Sep 28 15:54:43 pikachan kernel: [ 4928.119589] Restarting the timer
Sep 28 15:54:45 pikachan kernel: [ 4930.619602] Restarting the timer
Sep 28 15:54:48 pikachan kernel: [ 4933.119616] Restarting the timer
Sep 28 15:54:49 pikachan kernel: [ 4934.148657] Unloading module
 
Example 3 – Repeat every 0.25 seconds second, average deviation of 0.000002749 seconds (calculated on first 5)
pi@pikachan:~ $ sudo insmod single_thread.ko long_sec=0 long_nsec=250000000
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ dmesg
[ 5157.835868] Initializing kernel module
[ 5158.085909] Restarting the timer
[ 5158.335908] Restarting the timer
[ 5158.585916] Restarting the timer
[ 5158.835917] Restarting the timer
[ 5159.085913] Restarting the timer
[ 5159.335918] Restarting the timer
[ 5159.585914] Restarting the timer
[ 5159.835914] Restarting the timer
[ 5160.085931] Restarting the timer
[ 5160.123356] Unloading module


Example 4 – Repeat every 1 second, average deviation of 0.0000043333 seconds
pi@pikachan:~ $ sudo insmod single_thread.ko
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ tail -f /var/log/syslog
Sep 28 15:56:07 pikachan kernel: [ 5012.704554] Restarting the timer
Sep 28 15:56:08 pikachan kernel: [ 5013.704558] Restarting the timer
Sep 28 15:56:09 pikachan kernel: [ 5014.704565] Restarting the timer
Sep 28 15:56:10 pikachan kernel: [ 5015.704567] Restarting the timer
Sep 28 15:56:10 pikachan kernel: [ 5015.884021] Unloading module


Timing trials are shown above with their calculated variations. We saw average deviation of 0.00005449 seconds and 0.0000043333 seconds for the 1 second timer (.005499% and .00043%) 0.0000135 seconds for the 2.5 second timer (.00054%), and 0.000002749 seconds for the .25 second timer (.00109%). The deviations measured are very small. They are not even close to 1% of the timer value— they are a slim fraction of even 1%. The deviation (in percent) for the .25 second timer is greater than the deviation (in percent) for timers of greater lengths. This was an interesting observation to make and matched our predictions because we expected that longer timers would see less deviation in relation to their timer length than shorter timers.
 
Thread Design and Evaluation
For this part of the module, to start with, we declared static struct pointer using task_struct to point to the current task_struct of the thread being used for our module. We then declared a function fun_kthread() that takes a single parameter of type void* in order to run on the kernel threads once they are awakened by our module. Inside of this function we are simply printing that the function has been called. In the static function that we have defined, we have added a loop that prints out the system log showing that the iteration of the loop has started and we are showing the values of voluntary and involuntary context switches. Inside the loop we are setting the current state of the task to be interruptible by using set_current_state(TASK_INTERRUPTIBLE) and right after that we are suspending the execution of a running thread and waiting for the next part of the code to be awakened.
We have also added kthread_should_stop() to check whether or not the module  should terminate the thread. At this point we have also made sure that every time the timer expiration function is called the module will wake up a thread by using wake_up_process(). We have used kthread_run() inside the timer_init() function to spawn the kernel thread defined earlier and have used kthread_stop() in the timer exit() function to finally terminate the thread. 
 
Timing
Example 1 – Repeat every 1 second, average deviation of -0.0000999 seconds
pi@pikachan:~ $ sudo insmod single_thread.ko
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ dmesg
[ 1177.878618] Initializing kernel module
[ 1177.878943] running fun_kthread
[ 1177.878964] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 1178.878675] Restarting the timer
[ 1178.878702] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 1179.878660] Restarting the timer
[ 1179.878685] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 1180.878642] Restarting the timer
[ 1180.878664] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 1181.878626] Restarting the timer
 
Example 2 – Repeat every 0.25 seconds, average deviation of 0.000061666 seconds
pi@pikachan:~ $ sudo insmod single_thread.ko long_sec=0 long_nsec=25000000
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ tail -f /var/log/syslog
Sep 28 19:28:37 pikachan kernel: [  293.289978] Restarting the timer
Sep 28 19:28:37 pikachan kernel: [  293.290019] fun_kthread has started another iteration, nvcsw: 80, nivcse: 0
Sep 28 19:28:37 pikachan kernel: [  293.314978] Restarting the timer
Sep 28 19:28:37 pikachan kernel: [  293.315021] fun_kthread has started another iteration, nvcsw: 81, nivcse: 0
Sep 28 19:28:37 pikachan kernel: [  293.340006] Restarting the timer
Sep 28 19:28:37 pikachan kernel: [  293.340088] fun_kthread has started another iteration, nvcsw: 82, nivcse: 0
Sep 28 19:28:37 pikachan kernel: [  293.365085] Restarting the timer
Sep 28 19:28:37 pikachan kernel: [  293.365204] fun_kthread has started another iteration, nvcsw: 83, nivcse: 0
Sep 28 19:28:37 pikachan kernel: [  293.382002] Unloading module
Sep 28 19:28:37 pikachan kernel: [  293.382058] fun_kthread terminated
 
Example 3: Repeat every 5.5 seconds, average deviation of -.0007273333 seconds
pi@pikachan:~ $ sudo insmod single_thread.ko long_sec=5 long_nsec=500000000
pi@pikachan:~ $ sudo rmmod single_thread
pi@pikachan:~ $ dmesg
pi@pikachan:~ $ tail -f /var/log/syslog
Sep 28 19:52:23 pikachan kernel: [ 1718.870361] running fun_kthread
Sep 28 19:52:23 pikachan kernel: [ 1718.870400] fun_kthread has started another iteration, nvcsw: 0, nivcse: 0
Sep 28 19:52:28 pikachan kernel: [ 1724.368142] Restarting the timer
Sep 28 19:52:28 pikachan kernel: [ 1724.368178] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
Sep 28 19:52:34 pikachan kernel: [ 1729.868167] Restarting the timer
Sep 28 19:52:34 pikachan kernel: [ 1729.868215] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
Sep 28 19:52:39 pikachan kernel: [ 1735.368189] Restarting the timer
Sep 28 19:52:39 pikachan kernel: [ 1735.368218] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
Sep 28 19:52:41 pikachan kernel: [ 1737.394826] Unloading module
 


Timing trials are shown above with their calculated deviation. We saw an average deviation of -0.0000999 seconds for the 1 second timer (0.00999%), 0.000061666 seconds for the 0.25 second timer (0.02466%) , and -.0007273333 seconds for the 5.5 second timer (.00013224%). These deviations are more than the deviations we saw in the previous part of the assignment. In comparing the results of the .25 second timer and the 1 second timer in this part of the assignment and the previous part, the timers in this part have average variations of almost twice the length. This is because waking up the kernel thread so it can make another iteration of its loop has more overhead with the voluntary interruption.
Our results indicate that the kernel thread of our module was preempted voluntarily but no involuntary preemptions were seen during the entire running time of our module. We can see that for every iteration the number of voluntary context switches increases but the number of involuntary switches is consistently 0. Since our module for single threading environment works on the same principle of voluntary context switching we see the increase in the voluntary context switches taking into consideration all the system calls used inside the module as well as the wake up calls to our module. The scheduler is not scheduling tasks other than this one with the use of an isolated CPU. 
We see no involuntary preemption here because an involuntary context switch occurs when a thread runs and another thread wants to suspend the operation of the running thread and take over the CPU time. But since we have only one working thread in this case and a total of 4 CPU cores, the running of the thread is never interrupted involuntarily and that’s why we see the involuntary preemption count to be 0.


Multi-threading Design and Evaluation
 
To be able to use the multithreading approach, firstly we defined four static struct pointers using task_struct for four kernel threads for our module (one for each CPU core). The process of creating threads was slightly changed for the multithreading program as compared to the single thread program. In the timer_init() function, we used kthread_create() function instead of using kthread_run() to create a thread and call the fun_kthread() function. After calling the inbuilt kthread_create() function we used kthread_bind() function to bind a thread to a specific processor core. This exact process was done 4 times to create 4 threads and bind them to 4 processor cores. For the exit function we made sure that the function checks for every thread and removes the thread by calling kthread_stop() for every thread and checks whether the thread was actually terminated or not. The static function defined for the initialization of threads works in the same way as it did for the single thread program; we print out the processor id that a thread is running on for multithreading environment.
In the restart function for our timer we made a change to make sure that the wake_up_process() function is called for all the 4 threads to awaken every thread every time the timer is restarts. 


Timing 
Example 1 - Repeat every 2.2 seconds, average deviation of -0.0001 seconds
pi@pikachan:~ $ sudo insmod multi_thread.ko long_sec=2 long_nsec=200000000
pi@pikachan:~ $ sudo rmmod multi_thread
pi@pikachan:~ $ dmesg
[ 2483.525361] Initializing kernel module
[ 2483.525742] running fun_kthread
[ 2483.525762] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2483.525906] running fun_kthread
[ 2483.525924] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2483.526096] running fun_kthread
[ 2483.526113] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2483.527596] running fun_kthread
[ 2483.527730] fun_kthread has started another iteration, nvcsw: 1, nivcse: 1
[ 2485.725434] Restarting the timer
[ 2485.725448] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2485.725453] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2485.725458] fun_kthread has started another iteration, nvcsw: 2, nivcse: 1
[ 2485.725516] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2487.925443] Restarting the timer
[ 2487.925453] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2487.925458] fun_kthread has started another iteration, nvcsw: 3, nivcse: 1
[ 2487.925463] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2487.925526] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2490.125450] Restarting the timer
[ 2490.125462] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2490.125469] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2490.125473] fun_kthread has started another iteration, nvcsw: 4, nivcse: 1
[ 2490.125532] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2492.325457] Restarting the timer


Example 2 - Repeat every 0.2 seconds, average deviation of -.002126 seconds
pi@pikachan:~ $ sudo insmod multi_thread.ko long_sec=0 long_nsec=200000000
pi@pikachan:~ $ sudo rmmod multi_thread
pi@pikachan:~ $ dmesg
[ 2248.307016] Initializing kernel module
[ 2248.313436] running fun_kthread
[ 2248.313468] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2248.319243] running fun_kthread
[ 2248.319271] fun_kthread has started another iteration, nvcsw: 1, nivcse: 1
[ 2248.319398] running fun_kthread
[ 2248.319417] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2248.319496] running fun_kthread
[ 2248.319512] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2248.507080] Restarting the timer
[ 2248.507092] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2248.507100] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2248.507105] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2248.507153] fun_kthread has started another iteration, nvcsw: 2, nivcse: 1
[ 2248.707076] Restarting the timer
[ 2248.707085] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2248.707097] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2248.707101] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2248.707134] fun_kthread has started another iteration, nvcsw: 3, nivcse: 1
[ 2248.907078] Restarting the timer
[ 2248.907088] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2248.907101] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2248.907106] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2248.907141] fun_kthread has started another iteration, nvcsw: 4, nivcse: 1
[ 2249.107082] Restarting the timer


Example 3 - Repeat every 1 second, average deviation of -0.000713 seconds
pi@pikachan:~ $ sudo insmod multi_thread.ko
pi@pikachan:~ $ sudo rmmod multi_thread 
pi@pikachan:~ $dmesg
[ 2070.114582] Initializing kernel module
[ 2070.116945] running fun_kthread
[ 2070.116978] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2070.117060] running fun_kthread
[ 2070.117080] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2070.117171] running fun_kthread
[ 2070.117189] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2070.121814] running fun_kthread
[ 2070.121844] fun_kthread has started another iteration, nvcsw: 1, nivcse: 0
[ 2071.114647] Restarting the timer
[ 2071.114656] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2071.114663] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2071.114671] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2071.114740] fun_kthread has started another iteration, nvcsw: 2, nivcse: 0
[ 2072.114651] Restarting the timer
[ 2072.114663] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2072.114671] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2072.114676] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2072.114730] fun_kthread has started another iteration, nvcsw: 3, nivcse: 0
[ 2073.114649] Restarting the timer
[ 2073.114664] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2073.114670] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2073.114675] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2073.114712] fun_kthread has started another iteration, nvcsw: 4, nivcse: 0
[ 2074.114651] Restarting the timer


In comparing the timing deviation results upon first glance to single threaded mode, we see that multithreading has more deviation. Timing trials are shown above with their calculated variations. We saw an average deviation of -0.0001 seconds for the 2.2 second timer (.0045%), -.002126 seconds for the (.0045%), -.002126 seconds for the .2 second timer (1.063%), and -0.000713 seconds for the 1 second timer (.0713%). We see the deviation to be greater here because of the overhead created by all the working threads, which is not an issue in the case of a single threaded program. The handling logic creates more overhead. 
Similar to single threaded environment, the number of voluntary context switches increases with every iteration for a multithreading program as well for the same reasons stated earlier. From our results we can see that even though the number of involuntary context switches doesn’t increase in case of the multithreading program in every iteration, it does occur more frequently as compared to the single threaded program where the value of involuntary context switches was zero for every iteration. We can guess from the output that the involuntary context switch has occurred when a thread was running and another thread needed to be executed on the CPU core that was of higher priority. As we increase the number of threads that our program leveraging on a CPU with a fixed number of cores, we would expect to see an increase in the number of involuntary context switches.




System Performance


Based on what we zoomed in on in Kernelshark, we saw our most of our defined threads run to completion every time. [a][b][c][d][e]In some cases, we saw certain kernel threads being broken up by other processes that were not necessarily kernel related. This happened much more infrequently 


Based on the preemption standards defined for 2.6 kernels, if a thread with a higher static priority than the currently running thread becomes ready to run then the current thread can get preempted and returned to the wait-list for its static priority level to come into the picture again as the scheduling policy for 2.6 kernels determines the ordering only within the list of runnable threads with equal static priority. So, higher priority tasks like migration, interrupts, loggers can preempt the running thread. 


We took 5 measurements for the total execution time of one of your thread wakeups. The readings we got were 0.000093 seconds, 0.000072 seconds, 0.000087 seconds, 0.000078 seconds, and 0.000118 seconds. This gave us a mean of 0.0000896 seconds.


We noticed that each thread does not start at exactly the same time. We took 4 measurements for the jitter that were 0.000059 seconds, 0.000067 seconds, 0.000050 seconds, and 0.000092 seconds. The minimum value was 0.000050 seconds, the maximum value was 0.000092 seconds, and the mean was 0.000067 seconds.


For our examination of the single trace using trace-cmd, we measured over 5 wakeups for our kernel-2608 thread and found out that our minimum was 0.0000246 seconds, maximum was  0.0000289 seconds, and mean was 0.0000266 seconds.


Development Time
Our team took 18-20 hours to work on this lab.
[a]need to review once more
[b]_Marked as resolved_
[c]_Re-opened_
[d]_Marked as resolved_
[e]_Re-opened_