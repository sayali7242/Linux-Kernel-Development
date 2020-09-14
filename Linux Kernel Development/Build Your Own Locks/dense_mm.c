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
#include <asm/page.h>
#include <paging.h>

static atomic_t count_alloc_pages;
static atomic_t count_free_pages;

static unsigned int demand_paging = 1;
module_param(demand_paging, uint, 0644);

static unsigned int order = 0;

struct data_struct{
        atomic_t ref_counter;
        struct page **pm_page;
};

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

static int
do_fault(struct vm_area_struct * vma,
         unsigned long           fault_address)
{
        struct page * ptr;
        unsigned long vaddr, pfn;
        struct data_struct *data;
        int i;

        printk(KERN_INFO "paging_vma_fault() invoked: took a page fault at VA 0x%lx\n", fault_address);

        ptr = alloc_page(GFP_KERNEL);

        if(ptr == NULL){
                printk(KERN_ERR "Page allocation failed\n");
                return VM_FAULT_OOM;
        }

        vaddr = PAGE_ALIGN(fault_address);
        pfn = page_to_pfn(ptr);

        if(remap_pfn_range(vma, vaddr, pfn, PAGE_SIZE, vma->vm_page_prot) == 0){
                data = (struct data_struct *)kmalloc(sizeof(struct data_struct), GFP_KERNEL);
                data = vma->vm_private_data;
                i= atomic_read(&ptr->ref_counter);
                ptr->pm_page[i] = ptr;
                atomic_add(1, &ptr->ref_counter);
                atomic_add(1, &count_alloc_pages);
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
        page = vma->vm_private_data;
        atomic_inc(&page->ref_counter);
}

static void
paging_vma_close(struct vm_area_struct * vma)
{
        struct data_struct *pages;
        long unsigned int nr_pages;
        printk(KERN_INFO "paging_vma_close() invoked\n");

        nr_pages = (vma->vm_end - vma->vm_start)/PAGE_SIZE;
        if((vma->vm_end - vma->vm_start) % PAGE_SIZE){
                nr_pages++;
        }

        pages = vma->vm_private_data;

        if(atomic_read(&pages->ref_counter) == nr_pages){
                for(i = 0; i < nr_pages; i++){
                        __free_page(pages->pm_page[i]);
                        atomic_add(1, &count_free_pages);
                }
                kfree(pages->pm_page);
                kfree(pages);
        }
}

static struct vm_operations_struct
paging_vma_ops =
{
        .fault = paging_vma_fault,
        .open  = paging_vma_open,
        .close = paging_vma_close
};

static int
paging_mmap(struct file           * filp,
            struct vm_area_struct * vma)
{
        struct data_struct *data;
        struct page *pages;
        unsigned long vaddr, pfn;
        long unsigned int nr_pages;

        data = (struct data_struct *)kmalloc(sizeof(struct data_struct), GFP_KERNEL);
        atomic_set(&data->ref_counter, 0);
        vma->vm_private_data = (void *)data;

        vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE
              | VM_DONTDUMP | VM_PFNMAP;

        vma->vm_ops = &paging_vma_ops;

        printk(KERN_INFO "paging_mmap() invoked: new VMA for pid %d from VA 0x%lx to 0x%lx\n",
        current->pid, vma->vm_start, vma->vm_end);

        if(demand_paging == 0){
                nr_pages = (vma->vm_end - vma->vm_start)/PAGE_SIZE;

                if((vma->vm_end - vma->vm_start) % PAGE_SIZE){
                        nr_pages++;
                }

                order = my_get_order(nr_pages);
                pages = alloc_pages(GFP_KERNEL, order);

                if(pages == NULL){
                        printk(KERN_ERR "Page allocation failed\n");
                        return -ENOMEM;
                }
                atomic_add(nr_pages, &count_alloc_pages);
                for(i = 0; i < nr_pages; i++){
                        data->pages[i] = pages + i;
                        atomic_add(1, &data->ref_counter);
                }
 vaddr = PAGE_ALIGN(vma->vm_start);
                pfn = page_to_pfn(pages);

                if(remap_pfn_range(vma, vaddr, pfn, PAGE_SIZE*nr_pages, vma->vm_page_prot) != 0){
                printk(KERN_ERR "Address mapping failed\n");
                return -EFAULT;
                }
        }
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