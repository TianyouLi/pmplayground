#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <emmintrin.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libpmem.h>
#include <stdint.h>


const char CONTENT[]="abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgh";

/* using 80MB of pmem for this example */
#define PMEM_LEN   81920000
#define OUTERLOOP  100
#define CONTENTLEN (sizeof(CONTENT)/sizeof(char)-1)
#define INNERLOOP  PMEM_LEN/CONTENTLEN
#define TOTALWRITE (INNERLOOP)*(CONTENTLEN)
#define FLUSH_ALIGN ((uintptr_t)64)

#ifdef FLUSHOPT
#  define _mm_clflushopt(addr)                                          \
  asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)addr))
#else
#  define _mm_clflushopt(addr)                                \
  asm volatile("clflush %0" : "+m" (*(volatile char *)addr)) 
#endif
	
#ifdef NOFLUSH
#  define CL_FLUSH(uptr) 
#else
#  define CL_FLUSH(uptr) _mm_clflushopt((char *)uptr)
#endif


void test_line_flush(char* path);
void test_batch_flush(char* path); 

void test_line_flush(char* path) {
  char *pmemaddr;
  size_t mapped_len;
  int is_pmem;

  /* create a pmem file and memory map it */
  if ((pmemaddr = pmem_map_file(path, PMEM_LEN + FLUSH_ALIGN, PMEM_FILE_CREATE,
                                0666, &mapped_len, &is_pmem)) == NULL) {
		perror("pmem_map_file");
		exit(1);
  }

  pmemaddr = (char*)((uintptr_t)(pmemaddr+FLUSH_ALIGN-1) & ~(FLUSH_ALIGN-1));
  
  /* store a string to the persistent memory */
  size_t loop = OUTERLOOP;
  while(loop--) {
    for(int i=0; i < INNERLOOP; i++) {
      char* uptr = (pmemaddr+i*CONTENTLEN);
      memcpy(uptr, CONTENT, CONTENTLEN);
      CL_FLUSH(uptr);
#ifdef DOUBLEFLUSH
      CL_FLUSH(uptr);
#endif
    }
  }
}

void test_batch_flush(char* path) {
  char *pmemaddr;
  size_t mapped_len;
  int is_pmem = 0;

  /* create a pmem file and memory map it */
  if ((pmemaddr = pmem_map_file(path, PMEM_LEN + FLUSH_ALIGN, PMEM_FILE_CREATE,
                                0666, &mapped_len, &is_pmem)) == NULL) {
    perror("pmem_map_file");
    exit(1);
  }

  pmemaddr = (char*)(((uintptr_t)pmemaddr+FLUSH_ALIGN-1) & ~(FLUSH_ALIGN-1));
  
  /* store a string to the persistent memory */
  size_t loop = OUTERLOOP;
  while (loop--) {
    for(int i =0; i < INNERLOOP; i++) {
      memcpy(pmemaddr + i*CONTENTLEN, CONTENT, CONTENTLEN);
    }
    for (uintptr_t uptr = (uintptr_t)pmemaddr; uptr < (uintptr_t)pmemaddr + TOTALWRITE;
         uptr += FLUSH_ALIGN) {
      CL_FLUSH((char *)uptr);
    }
  }
}

int
main(int argc, char *argv[])
{
	struct timeval start, end;
	char* pmem = "/mnt/mem/pmem/myfile";
  
  // start benchmarking
	gettimeofday(&start, NULL);
	test_line_flush(pmem);
	gettimeofday(&end, NULL);
	printf("pmem line flush: %ldms\n", ((end.tv_sec * 1000000 + end.tv_usec)
                                      - (start.tv_sec * 1000000 + start.tv_usec)) / 1000);

	gettimeofday(&start, NULL);
	test_batch_flush(pmem);
	gettimeofday(&end, NULL);
	printf("pmem batch flush: %ldms\n", ((end.tv_sec * 1000000 + end.tv_usec)
                                       - (start.tv_sec * 1000000 + start.tv_usec)) / 1000);

}
