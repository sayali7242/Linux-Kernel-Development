/*
*This module helps observe memory management and paging mechanisms
*/

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <linux/list.h> 
#include <asm/page.h> 
#include <asm/uaccess.h>
#include <paging.h>

//Counter variables to keep track of number of pages allocated and freed
static atomic_t count_alloc_pages; 
static atomic_t count_free_pages; 

//Module parameter to unable and disable demand paging 
static unsigned int demand_paging = 1;
module_param(demand_paging, uint, 0644);

//No of orders
static unsigned int order = 0;

//Define data structure to keep track of vma 
struct data_struct{
	atomic_t ref_counter; 
	struct page *pm_page;
	struct list_head h_list;
};

//Get number of orders when number of pages is given 
static unsigned int
my_get_order(unsigned int value){
	unsigned int shifts = 0;
    	if(!value)
        	return 0;
    	if(!(value & (value - 1)))
        	value--;
    	while(value > 0){
        	value >>= 1;
        	shifts++;
    	}
    	return shifts;
}

//Define function to handle page faults 
static int
do_fault(struct vm_area_struct * vma,
         unsigned long           fault_address)
{
	struct page * ptr;
	unsigned long vaddr, pfn;	
	struct data_struct *data, *pages;

   	printk(KERN_INFO "paging_vma_fault() invoked: took a page fault at VA 0x%lx\n", fault_address);
    	
	//Allocate a new page of physical memory via the function alloc_page
	ptr = alloc_page(GFP_KERNEL);
	
	//Check if the allocation was successful
    	if(ptr == NULL){
        	printk(KERN_ERR "Page allocation failed\n");
        	return VM_FAULT_OOM;
    	}
	
	//Get page aligned address for virtual address and page frame number
	vaddr = PAGE_ALIGN(fault_address);
	pfn = page_to_pfn(ptr);

	//Update the process' page tables to map the faulting virtual address to the new physical address just allocated. If the update is successful, remember that the created page is allocated to the process. 
	if(remap_pfn_range(vma, vaddr, pfn, PAGE_SIZE, vma->vm_page_prot) == 0){
    		data = (struct data_struct *)kmalloc(sizeof(struct data_struct), GFP_KERNEL);
    		data->pm_page = ptr;
    		INIT_LIST_HEAD(&data->h_list);
    		pages =  vma->vm_private_data;
    		list_add_tail(&(data->h_list), &(pages->h_list));
    		atomic_inc(&count_alloc_pages);
    		return VM_FAULT_NOPAGE;
	}
	else{
		printk(KERN_ERR "Adress mapping failed\n");
                return VM_FAULT_SIGBUS;
	}

}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
static int
paging_vma_fault(struct vm_area_struct * vma,
                 struct vm_fault       * vmf)
{
    	unsigned long fault_address = (unsigned long)vmf->virtual_address
#else
static int
paging_vma_fault(struct vm_fault * vmf)

{
    	struct vm_area_struct * vma = vmf->vma;
    	unsigned long fault_address = (unsigned long)vmf->address;
#endif

    	return do_fault(vma, fault_address);
}

static void
paging_vma_open(struct vm_area_struct * vma)
{
	struct data_struct *page;
    	printk(KERN_INFO "paging_vma_open() invoked\n");
	//Increament the reference pointer to the data structure upon retrieval 
	page = vma->vm_private_data;
	atomic_inc(&page->ref_counter);
}

//Define function for close call.
static void
paging_vma_close(struct vm_area_struct * vma)
{
	struct data_struct *pages, *temp_page;
	struct list_head *position, *q;
    	printk(KERN_INFO "paging_vma_close() invoked\n");
	
	//Decreament the reference counter to the data structure and if becomes to zero free the memory allocated to it through demand paging 
	pages = vma->vm_private_data; 
	atomic_sub(1, &pages->ref_counter);
	if(atomic_read(&pages->ref_counter) == 0){
		list_for_each_safe(position, q, &pages->h_list){
			temp_page = list_entry(position, struct data_struct, h_list);
			__free_page(temp_page->pm_page);
			atomic_inc(&count_free_pages);
			list_del(position); 
			kfree(temp_page);
		}
	}
	
	//Free the allocated memory through pre-paging
	if(pages->pm_page != NULL){
		__free_pages(temp_page->pm_page, order);
		atomic_inc(&count_free_pages);
	}
	kfree(pages);
}

static struct vm_operations_struct
paging_vma_ops = 
{
    	.fault = paging_vma_fault,
    	.open  = paging_vma_open,
    	.close = paging_vma_close
};

//Define mmap() function for paging
static int
paging_mmap(struct file           * filp,
            struct vm_area_struct * vma)
{
	struct data_struct *data; 
	struct page *pages; 
	unsigned long vaddr, pfn;
	long unsigned int nr_pages;
	
	//Protect vma from operating system hamperings
    	vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE
              | VM_DONTDUMP | VM_PFNMAP;

	//Setup vma operations to catch page faults
    	vma->vm_ops = &paging_vma_ops;

    	printk(KERN_INFO "paging_mmap() invoked: new VMA for pid %d from VA 0x%lx to 0x%lx\n",
        current->pid, vma->vm_start, vma->vm_end);
	
	//When demand paging is disables and pre-paging is unabled
	if(demand_paging == 0){
		//Get the total number of required pages by dividing the length of virtual address by PAGE_SIZE
		nr_pages = (vma->vm_end - vma->vm_start)/PAGE_SIZE;
		
		if((vma->vm_end - vma->vm_start) % PAGE_SIZE){
			nr_pages++;
		}
		
		//Get number of orders
		order = my_get_order(nr_pages); 
		//Allocate pages for pre-paging
		pages = alloc_pages(GFP_KERNEL, order);
		
		//Check if the allocation was successful
		if(pages == NULL){
			printk(KERN_ERR "Page allocation failed\n");
			return -ENOMEM; 		
		}
		
		//Check if the page table update was successful for the total number of pages needed by the application program
		vaddr = PAGE_ALIGN(vma->vm_start);
		pfn = page_to_pfn(pages);
		if(remap_pfn_range(vma, vaddr, pfn, PAGE_SIZE*nr_pages, vma->vm_page_prot) != 0){
		printk(KERN_ERR "Address mapping failed\n");
		return -EFAULT;
		}
	}
	
	//Define data structure and initialize the list head
	data = (struct data_struct *)kmalloc(sizeof(struct data_struct), GFP_KERNEL);
	atomic_set(&data->ref_counter, 1); 
	data->pm_page = NULL; 
	if(!demand_paging){
		data->pm_page = pages; 
		atomic_inc(&count_alloc_pages); 
	}
	INIT_LIST_HEAD(&(data->h_list));
	vma->vm_private_data = (void *)data; 
	return 0;
}

static struct file_operations
dev_ops =
{
    	.mmap = paging_mmap,
};

static struct miscdevice
dev_handle =
{
    	.minor = MISC_DYNAMIC_MINOR,
    	.name = PAGING_MODULE_NAME,
    	.fops = &dev_ops,
};

static int
kmod_paging_init(void)
{
    	int status;

    	status = misc_register(&dev_handle);
    	if(status != 0){
        	printk(KERN_ERR "Failed to register misc. device for module\n");
        	return status;
    	}

    	printk(KERN_INFO "Loaded kmod_paging module\n");
	atomic_set(&count_alloc_pages, 0); 
	atomic_set(&count_free_pages, 0);
    	return 0;
}

static void
kmod_paging_exit(void)
{
    	misc_deregister(&dev_handle);
	printk("Number of allocated pages: %d\n", atomic_read(&count_alloc_pages));
	printk("Number of freed pages: %d\n", atomic_read(&count_free_pages));
    	printk(KERN_INFO "Unloaded kmod_paging module\n");
}

module_init(kmod_paging_init);
module_exit(kmod_paging_exit);

MODULE_LICENSE("GPL");

