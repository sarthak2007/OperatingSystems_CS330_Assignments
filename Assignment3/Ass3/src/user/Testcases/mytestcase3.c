#include<ulib.h>

/*mmap and call vfork testcase. Parent should not be able to print mm1[0]
since child has called munmap*/


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Map failed \n");
    return 1;
  }
  char *mm2;
  mm1[0] = 'A';
  pid = vfork();
  if(pid){
    printf("Parent mm1[0]:%c\n",mm1[0]);
    
    printf("Parent mm1[4096]:%c\n", mm1[4096]);
    mm1[4096] = 'C';
    printf("Parent mm1[4096]:%c\n", mm1[4096]);
    printf("Parent mm2[4097]:%c\n", mm2[4097]);
  	mm1[0] = 'C';
  }
  else{
      mm1[0] = 'B';
      printf("Child mm1[0]:%c\n",mm1[0]);
      int val1 =  mprotect(mm1, pages*1, PROT_READ);
      if(val1 < 0)
      {
        printf("Map failed\n");
        return 1;
      }

      mm2 = mmap(NULL, pages*4, PROT_READ | PROT_WRITE, MAP_POPULATE);

      mm2[4097] = 'C';

      mm1[4096] = 'B';
      pmap(1);
      exit(0);
  }

  return 0;
}

