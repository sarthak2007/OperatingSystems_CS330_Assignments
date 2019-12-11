#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
      
      char *addr1 = mmap(NULL, 22, PROT_READ|PROT_WRITE, 0);
      if((long)addr1 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);

      char *addr5 = mmap(addr1 + 4096*3, 22, PROT_READ | PROT_WRITE, 0);
      if((long)addr5 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      char *addr09 = mmap(addr1 + 4096, 2*4096, PROT_READ|PROT_WRITE, 0);
      if((long)addr09 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      char *addr6 = mmap(addr5+4096, 22, PROT_READ|PROT_WRITE, 0);
      if((long)addr6 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);

      char *addr2 = mmap(addr1 + 4096*4, 4096*10, PROT_WRITE, 0);
      if((long)addr2 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      int munmap0 = munmap(addr1+4*4096, 2*4096);
      if(munmap0 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1); 

      munmap0 = munmap(addr1+3*4096, 4*4096);
      if(munmap0 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1); 
    char *addr0 = mmap(addr2 + 4096*15, 4096*3, PROT_WRITE, 0);
      if((long)addr0 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      char *addr = mmap(addr0 - 4096 , 4096, PROT_READ, 0);
      if((long)addr < 0)
      {
        printf("this TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
    char *addr10 = mmap(addr2 + 4096, 4096, PROT_READ, 0);
      if((long)addr10 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);

    char *addr11 = mmap((u64 *)(0x7FE000000 - 4096 * 4), 4096, PROT_READ, 0);
      if((long)addr11 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);

    char *addr12 = mmap((u64 *)(0x7FE000000 - 4096 * 4), 4096*4, PROT_READ, 0);
      if((long)addr12 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      int munmap1 = munmap(addr1, 4096);
      if(munmap1 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      char *addr3 = mmap(addr1 + 4096*4, 4096, PROT_WRITE, 0);
      if((long)addr3 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      

      int munmap2 = munmap(addr2 + 4096*3, 22);

      if(munmap2 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);

      munmap0 = munmap(addr1, 30*4096);
      if(munmap0 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      munmap0 = munmap((u64 *)(0x7FE000000 - 4096 * 6), 6*4096);
      if(munmap0 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      char *addr01 = mmap(addr1 + 4096*3, 4096, PROT_WRITE, 0);
      if((long)addr01 < 0)
      {
        printf("TEST CASE FAILED\n");
        return 1;
      }
      pmap(1);
      
      return 0;
}