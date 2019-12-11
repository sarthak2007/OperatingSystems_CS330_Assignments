#include<types.h>
#include<mmap.h>
#include <page.h>


u64* get_pte(struct exec_context *ctx, u64 addr) 
{
    u64 *vaddr_base = (u64 *)osmap(ctx->pgd);
    u64 *entry;
    u32 phy_addr;
    
    entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
    phy_addr = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
    vaddr_base = (u64 *)osmap(phy_addr);
  
    /* Address should be mapped as un-priviledged in PGD*/
    if( (*entry & 0x1) == 0 || (*entry & 0x4) == 0)
        goto out;

     entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);
     phy_addr = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
     vaddr_base = (u64 *)osmap(phy_addr);
    
     /* Address should be mapped as un-priviledged in PUD*/
      if( (*entry & 0x1) == 0 || (*entry & 0x4) == 0)
          goto out;

      entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
      phy_addr = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
      vaddr_base = (u64 *)osmap(phy_addr);
      
      /* 
        Address should be mapped as un-priviledged in PMD 
         Huge page mapping not allowed
      */
      if( (*entry & 0x1) == 0 || (*entry & 0x4) == 0 || (*entry & 0x80) == 1)
          goto out;
     
      entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
      
      /* Address should be mapped as un-priviledged in PTE*/
      if( (*entry & 0x1) == 0 || (*entry & 0x4) == 0)
          goto out;
      
     return entry;

out:
      return NULL;
}

void pfn_unmap(struct exec_context *ctx, u64 addr) 
{
    u64 *pte_entry = get_pte(ctx, addr);
    if(!pte_entry)
             return;
  
    struct pfn_info *temp = get_pfn_info((*pte_entry)>>12);

	u8 ref_count = get_pfn_info_refcount(temp);

	if(ref_count == 1){
    	os_pfn_free(USER_REG, (*pte_entry >> PTE_SHIFT) & 0xFFFFFFFF);
	}
	else{
		decrement_pfn_info_refcount(temp);
	}
    *pte_entry = 0;  // Clear the PTE
  
    asm volatile ("invlpg (%0);" 
                    :: "r"(addr) 
                    : "memory");   // Flush TLB
    return;
}

void change_permissions(struct exec_context *ctx, u64 addr, int prot) 
{
    u64 *pte_entry = get_pte(ctx, addr);
    if(!pte_entry)
             return;

    u64 ac_flags = 0x5 | (prot & 0x2);

    struct pfn_info *temp = get_pfn_info((*pte_entry)>>12);

    u8 ref_count = get_pfn_info_refcount(temp);

	if(ref_count == 1){
	    *pte_entry = (((*pte_entry)>>3)<<3) | ac_flags;  // change the permissions
	}

	// if((((*pte_entry) & 0x2) && !(prot & 0x2)) || (!((*pte_entry) & 0x2) && (prot & 0x2)) ){
	    
  
    asm volatile ("invlpg (%0);" 
                    :: "r"(addr) 
                    : "memory");   // Flush TLB

	// }


    return;
}

void alloc_pfn(struct exec_context *current, u64 addr, int prot)
{
    u64 *vaddr_base = (u64 *)osmap(current->pgd);
    u64 *entry;
    u64 pfn;
    u64 ac_flags = 0x5 | (prot & 0x2);
  
    entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
    if(*entry & 0x1) {
      // PGD->PUD Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
      // allocate PUD
      pfn = os_pfn_alloc(OS_PT_REG);
      *entry = (pfn << PTE_SHIFT) | 0x7;
      vaddr_base = osmap(pfn);
    }
  
    entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);
    if(*entry & 0x1) {
       // PUD->PMD Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
       // allocate PMD
       pfn = os_pfn_alloc(OS_PT_REG);
       *entry = (pfn << PTE_SHIFT) | 0x7;
       vaddr_base = osmap(pfn);
    }
  
   entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
    if(*entry & 0x1) {
       // PMD->PTE Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
       // allocate PMD
       pfn = os_pfn_alloc(OS_PT_REG);
       *entry = (pfn << PTE_SHIFT) | 0x7;
       vaddr_base = osmap(pfn);
    }
   
   entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
   // since page is allocated recently so frame was not present, we don't need present check here
   pfn = os_pfn_alloc(USER_REG);
   *entry = (pfn << PTE_SHIFT) | ac_flags;
}

/**
 * Function will invoked whenever there is page fault. (Lazy allocation)
 * 
 * For valid acess. Map the physical page 
 * Return 1
 * 
 * For invalid access,
 * Return -1. 
 */
int vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
	if(!(addr>=MMAP_AREA_START && addr<MMAP_AREA_END))
		return -1;
	if(error_code & 0x1)	return -1;

	//address exists or not
	struct vm_area *curr = current->vm_area;

	if(curr==NULL)	return -1;

	if(addr < curr->vm_start)	return -1;
	while(curr!=NULL){
		if(curr->vm_next != NULL){
			if(addr>= curr->vm_end && addr<curr->vm_next->vm_start)	return -1;
		}
		else{
			if(addr>=curr->vm_end)	return -1;
		}
		curr = curr->vm_next;
	}

	//access permitted or not
	if(error_code & 0x2){
		curr = current->vm_area;
		while(curr!=NULL){
			if(addr>=curr->vm_start && addr<curr->vm_end){
				if(!(curr->access_flags & 0x2))	return -1;
			}
			curr = curr->vm_next;
		}
	}

    u64 *vaddr_base = (u64 *)osmap(current->pgd);
    u64 *entry;
    u64 pfn;
    u64 ac_flags = 0x5 | (error_code & 0x2);
  
    entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
    if(*entry & 0x1) {
      // PGD->PUD Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
      // allocate PUD
      pfn = os_pfn_alloc(OS_PT_REG);
      *entry = (pfn << PTE_SHIFT) | 0x7;
      vaddr_base = osmap(pfn);
    }
  
    entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);
    if(*entry & 0x1) {
       // PUD->PMD Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
       // allocate PMD
       pfn = os_pfn_alloc(OS_PT_REG);
       *entry = (pfn << PTE_SHIFT) | 0x7;
       vaddr_base = osmap(pfn);
    }
  
   entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
    if(*entry & 0x1) {
       // PMD->PTE Present, access it
       pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
       vaddr_base = (u64 *)osmap(pfn);
    }else{
       // allocate PMD
       pfn = os_pfn_alloc(OS_PT_REG);
       *entry = (pfn << PTE_SHIFT) | 0x7;
       vaddr_base = osmap(pfn);
    }
   
   entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
   // since this fault occured as frame was not present, we don't need present check here
   pfn = os_pfn_alloc(USER_REG);
   *entry = (pfn << PTE_SHIFT) | ac_flags;

   return 1;
}

/**
 * mprotect System call Implementation.
 */
int vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot)
{
    if(!(addr>=MMAP_AREA_START && addr+length<=MMAP_AREA_END))
		return -1;

	if((length>>12)<<12 != length)
		length = (((length>>12) + 1)<<12);

	if(length == 0)	return 0;

	struct vm_area *curr = current->vm_area;
	struct vm_area *prev = NULL;

	if(curr == NULL)	return -1;

	if(addr < curr->vm_start)	return -1;

	int flag=0;

	//check continuous range or not
	while(curr!=NULL){
		if(flag==0){
			if(addr>=curr->vm_start && addr<curr->vm_end){
				flag=1;
				if(addr+length <= curr->vm_end)
					break;
			}
			else if(curr->vm_next==NULL || (curr->vm_next!=NULL && addr < curr->vm_next->vm_start))
				return -1;			
		}
		else{
			if(curr->vm_start != prev->vm_end)
				return -1;
			if(addr+length <= curr->vm_end)
				break;
			if(curr->vm_next == NULL && addr+length>curr->vm_end)
				return -1;
		}
		prev = curr;
		curr = curr->vm_next;
	}

	//assign different permissions
	curr = current->vm_area;
	prev = NULL;
	flag=0;
	while(curr!=NULL){
		if(flag==0){
			if(addr==curr->vm_start){
				flag=1;
				if(addr+length < curr->vm_end){
					if(curr->access_flags != prot){
						struct vm_area *temp = alloc_vm_area();
						temp->vm_start = addr;
						temp->vm_end = addr+length;
						temp->access_flags = prot;
						temp->vm_next = curr;
						if(prev==NULL)	current->vm_area = temp;
						else prev->vm_next = temp;
						curr->vm_start = addr+length;
					}
					break;
				}
				else if(addr+length == curr->vm_end){
					if(curr->access_flags != prot)
						curr->access_flags = prot;
					break;
				}
				else{
					if(curr->access_flags != prot)
						curr->access_flags = prot;
				}
			}
			else if(addr>curr->vm_start && addr<curr->vm_end){
				flag=1;
				if(addr+length < curr->vm_end){
					if(curr->access_flags != prot){
						struct vm_area *temp = alloc_vm_area();
						temp->vm_start = addr;
						temp->vm_end = addr+length;
						temp->access_flags = prot;
						struct vm_area *temp1 = alloc_vm_area();
						temp1->vm_start = addr+length;
						temp1->vm_end = curr->vm_end;
						temp1->access_flags = curr->access_flags;
						temp->vm_next = temp1;
						temp1->vm_next = curr->vm_next;
						curr->vm_next = temp;
						curr->vm_end = addr;
					}
					break;
				}
				else if(addr+length == curr->vm_end){
					if(curr->access_flags != prot){
						struct vm_area *temp = alloc_vm_area();
						temp->vm_start = addr;
						temp->vm_end = curr->vm_end;
						temp->access_flags = prot;
						temp->vm_next = curr->vm_next;
						curr->vm_next = temp;
						curr->vm_end = addr;
					}
					break;
				}
				else{
					if(curr->access_flags != prot){
						struct vm_area *temp = alloc_vm_area();
						temp->vm_start = addr;
						temp->vm_end = curr->vm_end;
						temp->access_flags = prot;
						temp->vm_next = curr->vm_next;
						curr->vm_next = temp;
						curr->vm_end = addr;
					}
				}
			}
		}
		else{
			if(addr+length < curr->vm_end){
				if(curr->access_flags != prot){
					struct vm_area *temp = alloc_vm_area();
					temp->vm_start = curr->vm_start;
					temp->vm_end = addr+length;
					temp->access_flags = prot;
					temp->vm_next = curr;
					if(prev==NULL)	current->vm_area = temp;
					else prev->vm_next = temp;
					curr->vm_start = addr+length;
				}
				break;
			}
			else if(addr+length == curr->vm_end){
				if(curr->access_flags!=prot)
					curr->access_flags = prot;
				break;
			}
			else{
				if(curr->access_flags!=prot)
					curr->access_flags = prot;
			}
		}
		prev=curr;
		curr=curr->vm_next;
	}

	//merging
	prev = current->vm_area;
	curr = prev->vm_next;
	while(curr!=NULL){
		if(prev->vm_end == curr->vm_start && prev->access_flags == curr->access_flags){
			prev->vm_end = curr->vm_end;
			prev->vm_next = curr->vm_next;
			struct vm_area *temp = curr;
			curr = curr->vm_next;
			dealloc_vm_area(temp);
			continue;
		}
		if(curr == NULL)	break;
		prev = curr;
		curr = curr->vm_next;
	}

	for(unsigned long i=addr;i<addr+length;i+=4096)
		change_permissions(current, addr, prot);

	return 0;
}
/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{
	if(addr!=0  && !(addr>=MMAP_AREA_START && addr<=MMAP_AREA_END))
		return -1;

	if(addr!=0  && (flags&MAP_FIXED) && !(addr>=MMAP_AREA_START && addr+length<=MMAP_AREA_END))
		return -1;

	if((length>>12)<<12 != length)
		length = (((length>>12) + 1)<<12);

	// unsigned long start = ((MMAP_AREA_START>>12) + 1)<<12;
	unsigned long start = MMAP_AREA_START;
	unsigned long ret = -1;

	if(addr == 0){
		if(current->vm_area == NULL){
			if(start + length > MMAP_AREA_END)
				return -1;
			struct vm_area *temp = alloc_vm_area();
			temp->vm_start = start;
			temp->vm_end = temp->vm_start + length;
			temp->access_flags = prot;
			temp->vm_next = NULL;
			current->vm_area = temp;
			ret = start;
		}
		else{
			struct vm_area *temp = current->vm_area;
			
			if(temp->vm_start > length + start){
				struct vm_area *curr = alloc_vm_area();
				curr->vm_start = start;
				curr->vm_end = curr->vm_start + length;
				curr->access_flags = prot;
				curr->vm_next = temp;
				current->vm_area = curr;
				ret = start;
			}
			else if(temp->vm_start == length + start){
				if(temp->access_flags == prot){
					temp->vm_start -= length;
					ret = start;
				}
				else{
					struct vm_area *curr = alloc_vm_area();
					curr->vm_start = start;
					curr->vm_end = curr->vm_start + length;
					curr->access_flags = prot;
					curr->vm_next = temp;
					current->vm_area = curr;
					ret = curr->vm_start;
				}
			}
			else{
				while(temp->vm_next != NULL){
					if(temp->vm_end + length < temp->vm_next->vm_start){
						if(temp->access_flags == prot){
							temp->vm_end += length;
							ret = temp->vm_end - length;
							break;
						}
						else{
							struct vm_area *curr = alloc_vm_area();
							curr->vm_start = temp->vm_end;
							curr->vm_end = curr->vm_start + length;
							curr->access_flags = prot;
							curr->vm_next = temp->vm_next;
							temp->vm_next = curr;
							ret = curr->vm_start;
							break;
						}
					}
					else if(temp->vm_end + length == temp->vm_next->vm_start){
						if(prot==temp->access_flags && prot==temp->vm_next->access_flags){
							unsigned long ret1 = temp->vm_end;
							temp->vm_end = temp->vm_next->vm_end;
							struct vm_area *del = temp->vm_next;
							temp->vm_next = temp->vm_next->vm_next;
							dealloc_vm_area(del);
							ret = ret1;
							break;
						}
						else if(prot==temp->access_flags){
							temp->vm_end = temp->vm_next->vm_start;
							ret = temp->vm_end - length;
							break;
						}
						else if(prot==temp->vm_next->access_flags){
							temp->vm_next->vm_start = temp->vm_end;
							ret = temp->vm_next->vm_start;
							break;
						}
						else{
							struct vm_area *curr = alloc_vm_area();
							curr->vm_start = temp->vm_end;
							curr->vm_end = curr->vm_start + length;
							curr->access_flags = prot;
							curr->vm_next = temp->vm_next;
							temp->vm_next = curr;
							ret = curr->vm_start;
							break;
						}
					}
					else{
						temp = temp->vm_next;	
					}					
				}
				if(ret == -1){
					if(temp->vm_end + length > MMAP_AREA_END)
						return -1;
					if(temp->access_flags == prot){
						temp->vm_end += length;
						ret = temp->vm_end - length;
					}
					else{
						struct vm_area *curr = alloc_vm_area();
						curr->vm_start = temp->vm_end;
						curr->vm_end = curr->vm_start + length;
						curr->access_flags = prot;
						curr->vm_next = NULL;
						temp->vm_next = curr;
						ret = curr->vm_start;
					}
				}
			}
		}
	}
	else{
		struct vm_area *curr = current->vm_area;

		int flag=0;

		if(curr == NULL){
			if(addr+length > MMAP_AREA_END){
				addr = start;
				if(addr+length > MMAP_AREA_END)
					return -1;
			}
			struct vm_area *temp = alloc_vm_area();
			temp->vm_start = addr;
			temp->vm_end = temp->vm_start + length;
			temp->access_flags = prot;
			temp->vm_next = NULL;
			current->vm_area = temp;
			ret = temp->vm_start;
		}

		if(ret==-1 && addr< curr->vm_start){
			if(addr+length < curr->vm_start){
				struct vm_area *temp = alloc_vm_area();
				temp->vm_start = addr;
				temp->vm_end = temp->vm_start + length;
				temp->access_flags = prot;
				temp->vm_next = curr;
				current->vm_area = temp;
				ret = temp->vm_start;
			}
			else if(addr+length == curr->vm_start){
				if(curr->access_flags == prot){
					curr->vm_start -= length;
					ret = curr->vm_start;
				}
				else{
					struct vm_area *temp = alloc_vm_area();
					temp->vm_start = addr;
					temp->vm_end = temp->vm_start + length;
					temp->access_flags = prot;
					temp->vm_next = curr;
					current->vm_area = temp;
					ret = temp->vm_start;
				}	
			}
			else
				flag=1;
		}

		if(ret == -1){
			while(flag==0 && curr->vm_next != NULL){
				if(addr>=curr->vm_start && addr<curr->vm_end){
					flag=1;
					break;
				}
				else if(addr == curr->vm_end){
					if(addr+length < curr->vm_next->vm_start){
						if(curr->access_flags == prot){
							curr->vm_end += length;
							ret = curr->vm_end - length;
							break;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = curr->vm_end;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr->vm_next;
							curr->vm_next = temp;
							ret = temp->vm_start;
							break;
						}
					}
					else if(addr + length == curr->vm_next->vm_start){
						if(prot==curr->access_flags && prot==curr->vm_next->access_flags){
							unsigned long ret1 = curr->vm_end;
							struct vm_area *del = curr->vm_next;
							curr->vm_end = curr->vm_next->vm_end;
							curr->vm_next = curr->vm_next->vm_next;
							dealloc_vm_area(del);
							ret = ret1;
							break;
						}
						else if(prot==curr->access_flags){
							curr->vm_end = curr->vm_next->vm_start;
							ret = curr->vm_end - length;
							break;
						}
						else if(prot==curr->vm_next->access_flags){
							curr->vm_next->vm_start = curr->vm_end;
							ret = curr->vm_next->vm_start;
							break;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = curr->vm_end;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr->vm_next;
							curr->vm_next = temp;
							ret = temp->vm_start;
							break;
						}
					}
					else{
						flag=1;
						break;
					}
				}
				else if(addr > curr->vm_end && addr < curr->vm_next->vm_start){
					if(addr+length < curr->vm_next->vm_start){
						struct vm_area *temp = alloc_vm_area();
						temp->vm_start = addr;
						temp->vm_end = temp->vm_start + length;
						temp->access_flags = prot;
						temp->vm_next = curr->vm_next;
						curr->vm_next = temp;
						ret = temp->vm_start;
						break;
					}
					else if(addr+length == curr->vm_next->vm_start){
						if(curr->vm_next->access_flags == prot){
							curr->vm_next->vm_start -= length;
							ret = curr->vm_next->vm_start;
							break;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = addr;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr->vm_next;
							curr->vm_next = temp;
							ret = temp->vm_start;
							break;
						}
					}
					else{
						flag=1;
						break;
					}
				}
				curr = curr->vm_next;
			}
		}

		if(ret==-1 && flag==0 && addr + length > MMAP_AREA_END)
				flag=1;

		if(ret==-1 && flag==0){
			if(addr>=curr->vm_start && addr<curr->vm_end){
				flag=1;
			}
			else if(addr == curr->vm_end){
				if(curr->access_flags == prot){
					curr->vm_end += length;
					ret = curr->vm_start;
				}
				else{
					struct vm_area *temp = alloc_vm_area();
					temp->vm_start = curr->vm_end;
					temp->vm_end = temp->vm_start + length;
					temp->access_flags = prot;
					temp->vm_next = NULL;
					curr->vm_next = temp;
					ret = temp->vm_start;
				}
			}
			else{
				struct vm_area *temp = alloc_vm_area();
				temp->vm_start = addr;
				temp->vm_end = temp->vm_start + length;
				temp->access_flags = prot;
				temp->vm_next = NULL;
				curr->vm_next = temp;
				ret = temp->vm_start;
			}
		}

		if(ret==-1 && flag==1){
			if(flags&MAP_FIXED)
				return -1;
			else{
				while(curr->vm_next!=NULL){
					if(curr->vm_end + length < curr->vm_next->vm_start){
						if(curr->access_flags == prot){
							curr->vm_end += length;
							ret = curr->vm_end - length;
							break;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = curr->vm_end;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr->vm_next;
							curr->vm_next = temp;
							ret = temp->vm_start;
							break;
						}
					}
					else if(curr->vm_end + length == curr->vm_next->vm_start){
						if(prot==curr->access_flags && prot==curr->vm_next->access_flags){
							unsigned long ret1 = curr->vm_end;
							struct vm_area *del = curr->vm_next;
							curr->vm_end = curr->vm_next->vm_end;
							curr->vm_next = curr->vm_next->vm_next;
							dealloc_vm_area(del);
							ret = ret1;
							break;
						}
						else if(prot==curr->access_flags){
							curr->vm_end = curr->vm_next->vm_start;
							ret = curr->vm_end - length;
							break;
						}
						else if(prot==curr->vm_next->access_flags){
							curr->vm_next->vm_start = curr->vm_end;
							ret = curr->vm_next->vm_start;
							break;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = curr->vm_end;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr->vm_next;
							curr->vm_next = temp;
							ret = temp->vm_start;
							break;
						}
					}
					else{
						curr = curr->vm_next;	
					}		
				}


				if(ret==-1){
					if(curr->vm_end + length <= MMAP_AREA_END){
						if(curr->access_flags == prot){
							curr->vm_end += length;
							ret = curr->vm_end - length;
						}
						else{
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = curr->vm_end;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = NULL;
							curr->vm_next = temp;
							ret = temp->vm_start;
						}
					}
					else{
						curr = current->vm_area;

						if(curr->vm_start > length + start){
							struct vm_area *temp = alloc_vm_area();
							temp->vm_start = start;
							temp->vm_end = temp->vm_start + length;
							temp->access_flags = prot;
							temp->vm_next = curr;
							current->vm_area = temp;
							ret = temp->vm_start;
						}
						else if(curr->vm_start == length + start){
							if(curr->access_flags == prot){
								curr->vm_start -= length;
								ret = start;
							}
							else{
								struct vm_area *temp = alloc_vm_area();
								temp->vm_start = start;
								temp->vm_end = temp->vm_start + length;
								temp->access_flags = prot;
								temp->vm_next = curr;
								current->vm_area = temp;
								ret = start;
							}
						}
						else{
							while(curr->vm_end < addr){
								if(curr->vm_end + length < curr->vm_next->vm_start){
									if(curr->access_flags == prot){
										curr->vm_end += length;
										ret = curr->vm_end - length;
										break;
									}
									else{
										struct vm_area *temp = alloc_vm_area();
										temp->vm_start = curr->vm_end;
										temp->vm_end = temp->vm_start + length;
										temp->access_flags = prot;
										temp->vm_next = curr->vm_next;
										curr->vm_next = temp;
										ret = temp->vm_start;
										break;
									}
								}
								else if(curr->vm_end + length == curr->vm_next->vm_start){
									if(prot==curr->access_flags && prot==curr->vm_next->access_flags){
										unsigned long ret1 = curr->vm_end;
										struct vm_area *del = curr->vm_next;
										curr->vm_end = curr->vm_next->vm_end;
										curr->vm_next = curr->vm_next->vm_next;
										dealloc_vm_area(del);
										ret = ret1;
										break;
									}
									else if(prot==curr->access_flags){
										curr->vm_end = curr->vm_next->vm_start;
										ret = curr->vm_end - length;
										break;
									}
									else if(prot==curr->vm_next->access_flags){
										curr->vm_next->vm_start = curr->vm_end;
										ret = curr->vm_next->vm_start;
										break;
									}
									else{
										struct vm_area *temp = alloc_vm_area();
										temp->vm_start = curr->vm_end;
										temp->vm_end = temp->vm_start + length;
										temp->access_flags = prot;
										temp->vm_next = curr->vm_next;
										curr->vm_next = temp;
										ret = temp->vm_start;
										break;
									}
								}
								else{
									curr = curr->vm_next;	
								}		
							}
							if(ret==-1)	return -1;
						}
					}
				}
			}
		}
	}


	if(flags & MAP_POPULATE){
		for(unsigned long i = ret; i<ret+length; i+=4096)
			alloc_pfn(current, i, prot);
	}

	return ret;

}
/**
 * munmap system call implemenations
 */

int vm_area_unmap(struct exec_context *current, u64 addr, int length)
{
    if(addr!=0  && !(addr>=MMAP_AREA_START && addr+length<=MMAP_AREA_END))
		return -1;

	if((length>>12)<<12 != length)
		length = (((length>>12) + 1)<<12);

	struct vm_area *curr = current->vm_area;
	struct vm_area *prev = NULL;

	if(curr == NULL)
		return 0;

	int flag=0;

	int ret = 0;

	while(curr!=NULL){
		if(flag==0){
			if(curr->vm_start >= addr){
				flag=1;
				if(addr+length <= curr->vm_start){
					ret = 0;
					break;
				}
				if(addr+length< curr->vm_end){
					curr->vm_start = addr+length;
					ret = 0;
					break;
				}
				else{
					if(prev == NULL)	current->vm_area = curr->vm_next;
					else prev->vm_next = curr->vm_next;
					struct vm_area *del = curr;
					curr = curr->vm_next;
					dealloc_vm_area(del);
				}
			}
			else if(addr > curr->vm_start && addr < curr->vm_end){
				flag=1;
				if(addr+length < curr->vm_end){
					struct vm_area *temp = alloc_vm_area();
					temp->vm_start = addr+length;
					temp->vm_end = curr->vm_end;
					temp->access_flags = curr->access_flags;
					temp->vm_next = curr->vm_next;
					curr->vm_end = addr;
					curr->vm_next = temp;
					ret = 0;
					break;
				}
				else{
					curr->vm_end = addr;
					prev = curr;
					curr = curr->vm_next;
				}
			}
			else{
				prev = curr;
				curr = curr->vm_next;
			}
		}
		else if(flag==1){
			if(curr->vm_start >= addr+length){
				ret = 0;
				break;
			}
			else if(curr->vm_end > addr+length){
				curr->vm_start = addr+length;
				ret = 0;
				break;
			}
			else if(curr->vm_end == addr+length){
				if(prev == NULL)	current->vm_area = curr->vm_next;
				else prev->vm_next = curr->vm_next;
				dealloc_vm_area(curr);
				ret = 0;
				break;
			}
			else{
				if(prev == NULL)	current->vm_area = curr->vm_next;
				else prev->vm_next = curr->vm_next;
				struct vm_area *del = curr;
				curr = curr->vm_next;
				dealloc_vm_area(del);
			}
		}
	}

	if(ret==-1 && flag==1)
		if(prev == NULL)	current->vm_area = NULL;
		else prev->vm_next = NULL;


	if(ret==0){
		for(unsigned long i = addr; i<addr+length; i+=4096)
			pfn_unmap(current, i);
	}

	return ret;
}
