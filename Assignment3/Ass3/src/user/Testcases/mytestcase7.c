#include<ulib.h>

/*parent mmap and call vfork, then child writes to mmaped area, parent should see the modification done by child to mmaped area */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

		int page = 4096;

		for(int i = 0 ; i < 64; ++i) {

			char *mm1 = mmap(NULL, page*3, PROT_READ, 0);
			if((long)mm1 < 0) {

				printf("Test Case Failed1 %d\n", i);
				return 1;
			}

			char *mm2 = mmap(NULL, page*3, PROT_WRITE, 0);
			if((long)mm2 < 0) {

				printf("Test Case Failed2 %d\n", i);
				return 1;
			}
		}

		int m = mprotect((u64 *) (0x180200000), page*3, PROT_WRITE | PROT_READ);
		if((long) m<0) {

			printf("TestCase Failed1\n");
			return 1;
		}

		m = mprotect((u64 *) (0x180200000 + page), page*4, PROT_WRITE | PROT_READ);
		if((long) m<0) {

			printf("TestCase Failed2\n");
			return 1;
		}

		m = mprotect((u64 *) (0x180200000 + page), page*4, PROT_WRITE);
		if((long) m<0) {

			printf("TestCase Failed3\n");
			return 1;
		}

		m = mprotect((u64 *) (0x180200000 + page * 9), page*6, PROT_WRITE);
		if((long) m<0) {

			printf("TestCase Failed\n");
			return 1;
		}

		m = mprotect((u64 *) (0x180200000 + page * 13), page*3, PROT_READ);
		if((long) m<0) {

			printf("TestCase Failed\n");
			return 1;
		}

		m = mprotect((u64 *) (0x180200000 + page * 25), page*4, PROT_WRITE | PROT_READ);
		if((long) m>=0) {

			printf("TestCase Failed\n");
			return 1;
		}

		m = munmap((u64 *) (0x180200000 + page * 37), page);
		if((long) m>=0) {

			printf("TestCase Failed\n");
			return 1;
		}		


		char *mm2 = mmap(NULL, page*3, PROT_READ, 0);
			if((long)mm2 >= 0) {

				printf("Test Case Failed2");
				return 1;
			}

		pmap(0);





	return 0;
}
