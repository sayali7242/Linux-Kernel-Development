#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#define ARR_SIZE 8

typedef struct datatype_t {
    unsigned int array[ARR_SIZE];
} datatype_t;

static uint nr_structs = 2000;
module_param(nr_structs, uint, 0644);

static struct task_struct * kthread = NULL;
static struct page * pages = NULL;
static unsigned int nr_pages;
static unsigned int order;
static unsigned int nr_structs_per_page;

static unsigned int
my_get_order(unsigned int value)
{
        unsigned int shifts = 0;

        if (!value)
                return 0;

        if (!(value & (value - 1)))
                value--;

        while (value > 0) {
                value >>= 1;
                shifts++;
        }

        return shifts;
}

static int
thread_fn(void * data)
{
        int i, j, k;
        int size_d;
        datatype_t *dt = NULL;
        datatype_t *this_struct = NULL;

        printk("Hello from thread %s. nr_structs=%u\n", current->comm, nr_structs);
        printk("Kernel's page size is: %lu\n", PAGE_SIZE);
        size_d = sizeof(datatype_t);
        printk("Size of datatype struct is: %d\n", size_d);
        nr_structs_per_page = PAGE_SIZE / size_d;
        printk("The number of datatype structs that will fit in a single page of memory is: %u\n", nr_structs_per_page);

        nr_pages = nr_structs / nr_structs_per_page;
        if(nr_structs % nr_structs_per_page > 0)
            nr_pages++;
        order = my_get_order(nr_pages);
        printk("The number of struct elements that will fit in one page is: %u\n", nr_structs_per_page);
        printk("The number of pages needed to allocate to hold nr_structs is: %u\n", nr_pages);
        printk("The order to be passed to the page allocator is: %u\n", order);

        pages = alloc_pages(GFP_KERNEL, order);
        if (pages == NULL){
                printk("ERROR: alloc_pages() Failed");
                return -1;
        }

        for(i = 0; i < nr_pages; i++){
                dt = (datatype_t *)__va(PFN_PHYS(page_to_pfn(pages + i)));
                for(j = 0; j < nr_structs_per_page; j++){
                        this_struct = dt + j;
                        for(k = 0; k < 8; k++){
                                this_struct->array[k] = i*nr_structs_per_page*8 + j*8 + k;
                                if(j == 0 && k == 0){
                                        printk("Element for which j and k are both equal to 0 is: %u\n", this_struct->array[k]);
                                }
                        }
                }
        }

        while(!kthread_should_stop()){
                schedule();
        }

        for(i = 0; i < nr_pages; i++){
                dt = (datatype_t *)page_address(pages + i);
                for(j = 0; j < nr_structs_per_page; j++){
                        this_struct = dt + j;
                        for(k = 0; k < 8; k++){
                                if (this_struct->array[k] != (i*nr_structs_per_page*8 + j*8 + k)){
                                        printk("Error in [%d] array, %d element, and %d page", k, j, i);
                                }
                        }
                }
        }

        printk("Success: All the values match their expected values!\n");
        __free_pages(pages,order);
        return 0;
}

static int
kernel_memory_init(void)
{
        printk(KERN_INFO "Loaded kernel_memory module\n");

        kthread = kthread_create(thread_fn, NULL, "k_memory");
        if (IS_ERR(kthread)) {
                printk(KERN_ERR "Failed to create kernel thread\n");
                return PTR_ERR(kthread);
        }

        wake_up_process(kthread);

        return 0;
}

static void
kernel_memory_exit(void)
{
        kthread_stop(kthread);
        printk(KERN_INFO "Unloaded kernel_memory module\n");
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");

