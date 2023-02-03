#include <linux/slab.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <linux/personality.h>
#include <linux/mempolicy.h>
#include <linux/sem.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/iocontext.h>
#include <linux/key.h>
#include <linux/binfmts.h>
#include <linux/mman.h>
#include <linux/mmu_notifier.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <linux/nsproxy.h>
#include <linux/capability.h>
#include <linux/cpu.h>
#include <linux/cgroup.h>
#include <linux/security.h>
#include <linux/hugetlb.h>
#include <linux/seccomp.h>
#include <linux/swap.h>
#include <linux/syscalls.h>
#include <linux/jiffies.h>
#include <linux/futex.h>
#include <linux/compat.h>
#include <linux/kthread.h>
#include <linux/task_io_accounting_ops.h>
#include <linux/rcupdate.h>
#include <linux/ptrace.h>
#include <linux/mount.h>
#include <linux/audit.h>
#include <linux/memcontrol.h>
#include <linux/ftrace.h>
#include <linux/proc_fs.h>
#include <linux/profile.h>
#include <linux/rmap.h>
#include <linux/ksm.h>
#include <linux/acct.h>
#include <linux/tsacct_kern.h>
#include <linux/cn_proc.h>
#include <linux/freezer.h>
#include <linux/delayacct.h>
#include <linux/taskstats_kern.h>
#include <linux/random.h>
#include <linux/tty.h>
#include <linux/blkdev.h>
#include <linux/fs_struct.h>
#include <linux/magic.h>
#include <linux/perf_event.h>
#include <linux/posix-timers.h>
#include <linux/user-return-notifier.h>
#include <linux/oom.h>
#include <linux/khugepaged.h>
#include <linux/signalfd.h>
#include <linux/uprobes.h>
#include <linux/aio.h>
#include <linux/compiler.h>
#include <linux/sysctl.h>

#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/uaccess.h>
#include <asm/mmu_context.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include <trace/events/sched.h>


#include <trace/events/task.h>



static int bad_address(void *p)
{
	unsigned long dummy;

	return probe_kernel_address((unsigned long *)p, dummy);
}

static void flip_flag(int prot, pte_t *ptep, int flag){
	if(((prot | flag) != prot) && ((prot & flag) != flag)){
		printk(KERN_WARNING "set: %d",flag);
		set_pte(ptep, pte_set_flags(*ptep, flag));
	}else{
		printk(KERN_WARNING "clear: %d",flag);
		pte_clear_flags(*ptep, flag);
	}
	return;
}

static pte_t* get_pte(unsigned long address)		//reference page_dump
{
	pgd_t *base = __va(read_cr3() & PHYSICAL_PAGE_MASK);
	pgd_t *pgd = base + pgd_index(address);
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if (bad_address(pgd))
		goto bad;

	printk("PGD %lx ", pgd_val(*pgd));

	if (!pgd_present(*pgd))
		goto out;

	pud = pud_offset(pgd, address);
	if (bad_address(pud))
		goto bad;

	printk("PUD %lx ", pud_val(*pud));
	if (!pud_present(*pud) || pud_large(*pud))
		goto out;

	pmd = pmd_offset(pud, address);
	if (bad_address(pmd))
		goto bad;

	printk("PMD %lx ", pmd_val(*pmd));
	if (!pmd_present(*pmd) || pmd_large(*pmd))
		goto out;

	pte = pte_offset_kernel(pmd, address);
	if (bad_address(pte))
		goto bad;

	printk("PTE %lx", pte_val(*pte));
	return pte;
out:
	printk("BAD\n");
	return NULL;
bad:
	printk("BAD\n");
	return NULL;
}
static pte_t* get_pud(unsigned long address)		//reference page_dump
{
	pgd_t *base = __va(read_cr3() & PHYSICAL_PAGE_MASK);
	pgd_t *pgd = base + pgd_index(address);
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if (bad_address(pgd))
		goto bad;

	printk("PGD %lx ", pgd_val(*pgd));

	if (!pgd_present(*pgd))
		goto out;

	pud = pud_offset(pgd, address);
	if (bad_address(pud))
		goto bad;

	printk("PUD %lx ", pud_val(*pud));
	if (!pud_present(*pud) || pud_large(*pud))
		goto out;

	pmd = pmd_offset(pud, address);
	if (bad_address(pmd))
		goto bad;

	printk("PMD %lx ", pmd_val(*pmd));
	if (!pmd_present(*pmd) || pmd_large(*pmd))
		goto out;

	pte = pte_offset_kernel(pmd, address);
	if (bad_address(pte))
		goto bad;

	printk("PTE %lx", pte_val(*pte));
	return pud;
out:
	printk("BAD\n");
	return NULL;
bad:
	printk("BAD\n");
	return NULL;
}

SYSCALL_DEFINE3(copy_pte, unsigned long, from_va, unsigned long, to_va, pte_t __user *, pte)

{
	struct mm_struct *mm;
	pte_t *pte1;
	pte_t *pte2;
	mm = current->mm;
	printk(KERN_INFO "hello from copy_pte\n");

	if(bad_address((void *)from_va) || bad_address((void *)to_va || from_va==to_va)){
		printk(KERN_WARNING "bad address in from_va or to_va\n");
		return -EFAULT;
	} 
  //pgd_t *base = mm->pgd;
  if(pte!=NULL && bad_address((void *)pte)) return -EFAULT;
  /* start walking the page tables from the PGD. */

	pte1=get_pte(from_va);
	pte2=get_pte(to_va);

	if(pte1==NULL || pte2==NULL) return -EFAULT;
  
	*pte2 = *pte1;
  /* Once the PTE is copied, you need to flush the TLB */

  flush_tlb_mm(mm);
  return 0;
}

SYSCALL_DEFINE3(change_pte_prot, unsigned long, va, int, prot, int, flushtlb)

{
	pte_t *ptep;
	int allflag;
	printk(KERN_INFO "hello from change_pte_prot\n");
	if(bad_address((void *)va)){
		printk(KERN_WARNING "bad address va\n");
		return -EFAULT;
	} 
	allflag = PROT_NONE | PROT_NONE | PROT_WRITE | PROT_EXEC;
	if(prot>allflag){
		printk(KERN_WARNING "failed to match with any given protect flags\n");
	// 	return -EINVAL;
	}
	// if(!(((prot | PROT_NONE) == prot) && ((prot & PROT_NONE) == PROT_NONE)) ||
	// 	(((prot | PROT_READ) == prot) && ((prot & PROT_READ) == PROT_READ)) ||
	// 	(((prot | PROT_WRITE) == prot) && ((prot & PROT_WRITE) == PROT_WRITE)) ||
	// 	(((prot | PROT_EXEC)  == prot) && ((prot & PROT_EXEC)  == PROT_EXEC))) {

	// 	printk(KERN_WARNING "failed to match with any given protect flags");
	// 	return -EINVAL;
	// } 

	ptep = get_pte(va);

	//PROT_NONE
	//flip_flag(ptep, PROT_NONE);
	// flip_flag(prot, ptep, PROT_READ);
	// flip_flag(prot, ptep, PROT_WRITE);
	// flip_flag(prot, ptep, PROT_EXEC);

	

	if((prot == PROT_NONE)) {
		printk(KERN_WARNING "set PROT_NONE");
		//pmd_clear_flags(pmd, _PAGE_PRESENT | _PAGE_PROTNONE);
		set_pte(ptep, pte_clear_flags(*ptep, _PAGE_PRESENT));
	}

	if(((prot | PROT_READ) == prot) && ((prot & PROT_READ) == PROT_READ)){
		printk(KERN_WARNING "set PROT_READ");
		set_pte(ptep, pte_clear_flags(*ptep, _PAGE_RW));
	}
	if(((prot | PROT_WRITE) == prot) && ((prot & PROT_WRITE) == PROT_WRITE)){
		printk(KERN_WARNING "set PROT_WRITE");
		set_pte(ptep, pte_set_flags(*ptep, _PAGE_RW));
	}
	if(((prot | PROT_EXEC) == prot) && ((prot & PROT_EXEC) == PROT_EXEC)){
		printk(KERN_WARNING "set PROT_EXEC");
		set_pte(ptep, pte_clear_flags(*ptep, _PAGE_NX));
	}
	else{
		set_pte(ptep, pte_set_flags(*ptep, _PAGE_NX));
	}


	//set_pte(ptep, pte_set_flags(*ptep, prot));

  	if(flushtlb==1) __flush_tlb_single(va);
  	return 0;
}


SYSCALL_DEFINE2(zc_send, const char *, buffer, size_t, size)

{
	int i,c;
	//char *chbuf;
	c=0;
	//chbuf=current->mm->mmap->buf;
	//current->mm->mmap->buf_list=kmalloc(size*sizeof(char *), GFP_KERNEL);
	for(i=0;i<size && i<1000;i++){
		current->mm->mmap->buf[i]=kmalloc(sizeof(char *), GFP_KERNEL);
		(current->mm->mmap->buf[i])=buffer[i];
		printk(KERN_WARNING "current->mm->mmap->buf[%d]: %c\n",i,current->mm->mmap->buf[i]);
		c++;
	}
	return c;
}

SYSCALL_DEFINE2(zc_recv, char *, buffer, size_t, size)

{
	int i,c;
	//char *chbuf;
	pte_t* pte1;
	pte_t* pte2;
	c=0;
	if(size==15){
		printk(KERN_WARNING "malloc of rval buffer with size: %d \n",size);
		buffer = kmalloc(size*sizeof(char *), GFP_KERNEL);
	} 
	if(sizeof(buffer)!=size) {
		printk(KERN_WARNING "size of buf: %d, size: %d \n",sizeof(buffer),size);
	}
	// chbuf=current->mm->mmap->buf;
	for(i=0;i<size && i<1000;i++){

		if(!current->mm->mmap->buf[i]) {
			printk(KERN_WARNING "null pointer in buffer of process: %d\n",i);
			return c;
		}
		if(!(buffer[i])) {
			printk(KERN_WARNING "null pointer in rval buffer: %d\n",i);
			//return c;
		}
		printk(KERN_WARNING "current->mm->mmap->buf[%d]: %c\n",i,current->mm->mmap->buf[i]);
		//pte1=get_pte(&current->mm->mmap->buf[i]);
		// pte1=get_pte(current->mm->mmap->buf[i]);
		pte1=get_pud(current->mm->mmap->buf[i]);
		//pte1=get_pte(current->mm->mmap->buf+i);

		// *chbuf[i]=*(buffer+i);
		pte2=get_pte(&buffer[i]);
		printk(KERN_WARNING "buffer[%d]: %c",i,buffer[i]);
		if(pte1) *pte2=*pte1;
		else {
			printk(KERN_WARNING "Invalid pte of vma->buf");
			buffer[i]=current->mm->mmap->buf[i];
		}
		c++;
	}
	return c;
}