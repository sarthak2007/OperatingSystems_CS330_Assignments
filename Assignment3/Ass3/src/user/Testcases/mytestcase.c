#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*3, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  char * mm2 = mmap(NULL, pages*2, PROT_READ, 0);
  if((long)mm2 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  int munmap1 = munmap(mm1, pages);
  if((long)munmap1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  munmap1 = munmap(mm1 + pages*2, pages*2);
  if((long)munmap1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  pmap(1);
  
  char * mm3 = mmap(mm1, pages*4, PROT_READ|PROT_WRITE, 0);
  if((long)mm3 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  char * mm4 = mmap(mm1, pages*1, PROT_READ, 0);
  if((long)mm4 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  char * mm5 = mmap(mm1 + pages*3, pages*1, PROT_READ, 0);
  if((long)mm5 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);


  int mpro;

  mpro = mprotect(mm3 - pages, pages*3, PROT_WRITE);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  mpro = mprotect(mm1 + pages*4, pages*3, PROT_READ);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  char * mm6 = mmap(mm1 + pages*2, pages*1, PROT_READ, 0);
  if((long)mm6 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  mpro = mprotect(mm1 + pages*2, pages*5, PROT_READ | PROT_WRITE);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  munmap1 = munmap(mm1 + pages*1, pages*5);
  if((long)munmap1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  char * mm7 = mmap(mm1 + pages*3, pages*1, PROT_READ, 0);
  if((long)mm7 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  char * mm8 = mmap(mm1 , pages*2, PROT_READ, 0);
  if((long)mm8 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  mpro = mprotect(mm1, pages*2, PROT_WRITE);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  munmap1 = munmap(mm1, pages*5);
  if((long)munmap1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  char * mm9 = mmap(NULL , pages*4, PROT_READ, 0);
  if((long)mm9 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  char * mm10 = mmap((u64 *)(0x7FE000000 - pages * 4) , pages*4, PROT_READ, 0);
  if((long)mm10 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

    char * mm11 = mmap((u64 *)(0x7FE000000 - pages * 7) , pages*4, PROT_READ, 0);
  if((long)mm11 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  mpro = mprotect(mm1, pages*2, PROT_WRITE);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  mpro = mprotect(mm1 + pages*6, pages*3, PROT_READ);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  char * mm12 = mmap(mm1 + pages*5 , pages*1, PROT_READ, MAP_FIXED);
  if((long)mm12 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  mpro = mprotect(mm1 + pages*5, pages*3, PROT_WRITE);
  if((long)mpro < 0) {

    printf("Test Case Failed \n");
    return 1;
  }
  pmap(1);

  char * mm13 = mmap(mm1 + pages*4 , pages*1, PROT_READ, MAP_FIXED);
  if((long)mm13 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);



 return 0;
}
