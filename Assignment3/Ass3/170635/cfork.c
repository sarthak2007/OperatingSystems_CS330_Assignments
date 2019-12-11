/// Name: Sarthak Singhal   Roll Number: 170635

#include <cfork.h>
#include <page.h>
#include <mmap.h>



/* You need to implement cfork_copy_mm which will be called from do_cfork in entry.c. Don't remove copy_os_pts()*/
void cfork_copy_mm(struct exec_context *child, struct exec_context *parent ){


	//copy vm_area
	struct vm_area *curr_parent = parent->vm_area;
	child->vm_area = NULL;
	if(curr_parent !=NULL ){
		child->vm_area = alloc_vm_area();
		child->vm_area->vm_start = curr_parent->vm_start;
		child->vm_area->vm_end = curr_parent->vm_end;
		child->vm_area->access_flags = curr_parent->access_flags;
		child->vm_area->vm_next = NULL;

		curr_parent = curr_parent->vm_next;
		struct vm_area *curr_child, *curr;
		curr_child = child->vm_area;

		while(curr_parent!=NULL){
			curr = alloc_vm_area();
			curr->vm_start = curr_parent->vm_start;
			curr->vm_end = curr_parent->vm_end;
			curr->access_flags = curr_parent->access_flags;
			curr->vm_next = NULL;
			curr_child->vm_next = curr;
			curr_parent = curr_parent->vm_next;
			curr_child = curr_child->vm_next;
		}
		curr_child->vm_next = NULL;
	}
	// printk("access: %x\n",parent->vm_area->vm_start);

   void *os_addr;
   u64 vaddr; 
   struct mm_segment *seg;

   child->pgd = os_pfn_alloc(OS_PT_REG);

   os_addr = osmap(child->pgd);
   bzero((char *)os_addr, PAGE_SIZE);
   
   //CODE segment
   seg = &child->mms[MM_SEG_CODE];
   for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE){
      u64 *parent_pte =  get_user_pte(parent, vaddr, 0);
      if(parent_pte){
      	   *parent_pte = (((*parent_pte)>>2)<<2) | 0x1;
           install_ptable((u64) os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
           u64 *child_pte = get_user_pte(child, vaddr, 0);
           *child_pte = *parent_pte;   
           struct pfn_info *temp = get_pfn_info((*parent_pte)>>12);
           increment_pfn_info_refcount(temp);
           asm volatile ("invlpg (%0);" 
                    :: "r"(vaddr) 
                    : "memory");   // Flush TLB
      }
   } 
   //RODATA segment
   
   seg = &child->mms[MM_SEG_RODATA];
   for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE){
      u64 *parent_pte =  get_user_pte(parent, vaddr, 0);
      if(parent_pte){
      	   *parent_pte = (((*parent_pte)>>2)<<2) | 0x1;
           install_ptable((u64) os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
           u64 *child_pte = get_user_pte(child, vaddr, 0);
           *child_pte = *parent_pte;   
           struct pfn_info *temp = get_pfn_info((*parent_pte)>>12);
           increment_pfn_info_refcount(temp);
           asm volatile ("invlpg (%0);" 
                    :: "r"(vaddr) 
                    : "memory");   // Flush TLB
      }   
   } 
   
   //DATA segment
  seg = &child->mms[MM_SEG_DATA];
  for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE){
  // printk("start: %u \n",vaddr);
      u64 *parent_pte =  get_user_pte(parent, vaddr, 0);
      if(parent_pte){
      	   *parent_pte = (((*parent_pte)>>2)<<2) | 0x1;
           install_ptable((u64) os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
           u64 *child_pte = get_user_pte(child, vaddr, 0);
           *child_pte = *parent_pte;   
           struct pfn_info *temp = get_pfn_info((*parent_pte)>>12);
           increment_pfn_info_refcount(temp);
           asm volatile ("invlpg (%0);" 
                    :: "r"(vaddr) 
                    : "memory");   // Flush TLB
      }
  } 
  
  //STACK segment
  seg = &child->mms[MM_SEG_STACK];
  for(vaddr = seg->end - PAGE_SIZE; vaddr >= seg->next_free; vaddr -= PAGE_SIZE){
      u64 *parent_pte =  get_user_pte(parent, vaddr, 0);
      
     if(parent_pte){
           u64 pfn = install_ptable((u64)os_addr, seg, vaddr, 0);  //Returns the blank page  
           pfn = (u64)osmap(pfn);
           memcpy((char *)pfn, (char *)(*parent_pte & FLAG_MASK), PAGE_SIZE); 
      }
  }

  //mmap areas
  curr_parent = parent->vm_area;
  while(curr_parent!=NULL){
  	for(vaddr = curr_parent->vm_start; vaddr<curr_parent->vm_end; vaddr += PAGE_SIZE){
  		u64 *parent_pte = get_user_pte(parent, vaddr, 0);

  		if(parent_pte){
      	   *parent_pte = (((*parent_pte)>>2)<<2) | 0x1;
           install_ptable((u64) os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
           u64 *child_pte = get_user_pte(child, vaddr, 0);
           *child_pte = *parent_pte;   
           struct pfn_info *temp = get_pfn_info((*parent_pte)>>12);
           increment_pfn_info_refcount(temp);
           asm volatile ("invlpg (%0);" 
                    :: "r"(vaddr) 
                    : "memory");   // Flush TLB
      	}

  	}
  	curr_parent = curr_parent->vm_next;

  }

    copy_os_pts(parent->pgd, child->pgd);
    return;
    
}

/* You need to implement cfork_copy_mm which will be called from do_vfork in entry.c.*/
void vfork_copy_mm(struct exec_context *child, struct exec_context *parent ){
  	parent->state = WAITING;

	u64 vaddr;
	struct mm_segment *seg;
	void *os_addr;
	os_addr = osmap(child->pgd);
	// bzero((char *)os_addr, PAGE_SIZE);

	seg = &child->mms[MM_SEG_STACK];
	for(vaddr = seg->end - PAGE_SIZE; vaddr >= seg->next_free; vaddr -= PAGE_SIZE);

	//offset by which stack should be lifted
	u64 offset = seg->end - vaddr;

	for(vaddr = seg->end - PAGE_SIZE; vaddr >= seg->next_free; vaddr -= PAGE_SIZE){
		u64 *parent_pte =  get_user_pte(parent, vaddr, 0);

		if(parent_pte){

			u64 pfn;
			u64 *curr_pte = get_user_pte(child, vaddr-offset, 0);
			if(curr_pte){
				pfn = *curr_pte & FLAG_MASK;
			}
			else{
				pfn = install_ptable((u64)os_addr, seg, vaddr-offset, 0);  //Returns the blank page  
				pfn = (u64)osmap(pfn);
			}

			memcpy((char *)pfn, (char *)(*parent_pte & FLAG_MASK), PAGE_SIZE); 
		}
	}

	(child->regs).entry_rsp = (parent->regs).entry_rsp - offset;
	(child->regs).rbp = (parent->regs).rbp - offset;
	seg->next_free -= offset;
	// seg->end -= offset;

    return;
    
}

/*You need to implement handle_cow_fault which will be called from do_page_fault 
incase of a copy-on-write fault

* For valid acess. Map the physical page 
 * Return 1
 * 
 * For invalid access,
 * Return -1. 
*/

int handle_cow_fault(struct exec_context *current, u64 cr2){
	// if(!(cr2>=MMAP_AREA_START && cr2<MMAP_AREA_END))
	// 	return -1;

	//address exists or not
	int flag=0;

	struct vm_area *curr = current->vm_area;
	// if(cr2 < curr->vm_start)	return -1;
	while(curr!=NULL){
		// if(curr->vm_next != NULL){
	// printk("access: %x %x %x\n",curr->vm_start, curr->vm_end, cr2);
		if(cr2>= curr->vm_start && cr2<curr->vm_end){
			flag = 1;break;
		}
		// }
		// else{
			// if(cr2>=curr->vm_end)	return -1;
		// }
		curr = curr->vm_next;
	}

	if(flag==0){
		u64 vaddr;
		struct mm_segment *seg;
		seg = &current->mms[MM_SEG_DATA];
		for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE){
		// printk("start: %u \n",vaddr);
		 	if(cr2>=vaddr && cr2<vaddr+PAGE_SIZE){
		 		flag=1;
		 		break;
		 	}	
		}
	}

	if(flag==0)	return -1;
	//access permitted or not
	curr = current->vm_area;
	while(curr!=NULL){
		if(cr2>=curr->vm_start && cr2<curr->vm_end){
			if(!(curr->access_flags & 0x2))	return -1;
		}
		curr = curr->vm_next;
	}

	u64 *current_pte = get_user_pte(current, cr2, 0);

	struct pfn_info *temp = get_pfn_info((*current_pte)>>12);

	u8 ref_count = get_pfn_info_refcount(temp);

	if(ref_count == 1){
		// printk("access: %x\n",*current_pte);
		*current_pte = (((*current_pte)>>2)<<2) | 0x3;
		asm volatile ("invlpg (%0);" 
                    :: "r"(cr2) 
                    : "memory");   // Flush TLB
		return 1;
	}
	// printk("ref_count = %d\n", ref_count);

	decrement_pfn_info_refcount(temp);

	u64 pfn = os_pfn_alloc(USER_REG);

	pfn = (u64)osmap(pfn);
   	memcpy((char *)pfn, (char *)(*current_pte & FLAG_MASK), PAGE_SIZE);

    *current_pte = pfn | 0x7;

    asm volatile ("invlpg (%0);" 
                    :: "r"(cr2) 
                    : "memory");   // Flush TLB

    return 1;

}

/* You need to handle any specific exit case for vfork here, called from do_exit*/
void vfork_exit_handle(struct exec_context *ctx){
	struct exec_context *parent = get_ctx_by_pid(ctx->ppid);

	if(parent->pgd != ctx->pgd)	return;

	parent->state = READY;

	parent->mms[MM_SEG_DATA] = ctx->mms[MM_SEG_DATA];

	parent->vm_area = ctx->vm_area;

	return;
}