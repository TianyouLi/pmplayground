#include <stdio.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#define ERROR(format,...) error_at_line(1, errno, __FILE__, __LINE__, \
                                        format, ##__VA_ARGS__)
#define FILESIZE (1024*1024*1024)
#define BUFFSIZE (256)


int init_datafile(const char* filename, const uint64_t size)
{
  FILE* fp = fopen(filename, "w+");
  char buff[BUFFSIZE];
  
  if (NULL==fp) {
    ERROR("Can't create file <%s>", filename);
  }

  for (uint64_t i = 0; i < size; i++) {
    buff[i & (BUFFSIZE-1)] = i & (BUFFSIZE-1);
    if (((i+1) % BUFFSIZE) == 0) {
      int len = fwrite(buff, sizeof(char), BUFFSIZE, fp);
      if (len < BUFFSIZE) {
        ERROR("Write <%s> error on %ldth", filename, i);
      }
    }
  }

  fclose(fp);
  
  return 0;
}

typedef struct PMMap {
  void* start;
  uint64_t capacity;
  uint64_t pos;
} PMMap;

typedef struct PMFile {
  FILE* fp;
  PMMap pmap;
} PMFile;

uint64_t fcapacity(FILE* fp) {
  uint64_t result = 0;
  uint64_t cur = ftell(fp);

  fseek(fp, 0, SEEK_END);
  result = ftell(fp);

  fseek(fp, 0, cur);

  return result;
}

PMFile* pmfopen(const char* filename, const char* mode)
{
  PMFile* result = (PMFile*) malloc(sizeof(PMFile));
  if (NULL == result) {
    ERROR("Can't alloc memory for PMFile");
  }

  result->fp = fopen(filename, mode);
  if (NULL == result->fp) {
    free(result);
    ERROR("Can't open file %s", filename);
  }

  int fd = fileno(result->fp);
  
  result->pmap.capacity = fcapacity(result->fp);
  result->pmap.pos = 0;
  result->pmap.start = NULL;

  if (result->pmap.capacity == 0) {
    fclose(result->fp);
    free(result);
    ERROR("File is zero length");
  }

  result->pmap.start =
    mmap(NULL, result->pmap.capacity, PROT_READ | PROT_WRITE,
         MAP_PRIVATE, fd, 0);
  if (result->pmap.start == MAP_FAILED) {
    fclose(result->fp);
    free(result);
    ERROR("Failed to map file");
  }

  return result;
}

typedef enum {
  STREAM,
  MMAP
} OPMode;

int pmfread(PMFile* pmf, uint64_t step, OPMode mode)
{
  uint64_t length = pmf->pmap.capacity;
  if (step > length) {
    ERROR("The stepping(%ld) is larger than file size(%ld) ", step, length);
  }
  
  char* buff = (char*) malloc(step);
  if (NULL == buff) {
    ERROR("Can't allocate size = %ld memory", step);
  }
  
  if (mode == STREAM) {
    rewind(pmf->fp);
    while(length > 0) {
      if (step != fread(buff, 1, step, pmf->fp)) {
        ERROR("Read file error");
      }
      length -= step;
    }
  } else {
    char *p = (char*) pmf->pmap.start;
    while(length > 0) {
      memcpy(buff, p, step);
      p +=step;
      length -=step;
    }
  }

  free(buff);

  return 0;
}

int pmfwrite(PMFile* pmf, uint64_t step, OPMode mode)
{
  uint64_t length = pmf->pmap.capacity;
  if (step > length) {
    ERROR("The stepping(%ld) is larger than file size(%ld) ", step, length);
  }
  
  char* buff = (char*) malloc(step);
  if (NULL == buff) {
    ERROR("Can't allocate size = %ld memory", step);
  }
  
  if (mode == STREAM) {
    rewind(pmf->fp);
    while(length > 0) {
      fwrite(buff, 1, step, pmf->fp);
      length -= step;
    }
  } else {
    char *p = (char*) pmf->pmap.start;
    while(length > 0) {
      memcpy(p, buff, step);
      p +=step;
      length -=step;
    }
  }

  free(buff);

  return 0;
}

int pmfclose(PMFile* pmf)
{
  if (NULL == pmf) {
    return 0;
  }
  
  fclose(pmf->fp);
  free(pmf);

  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc != 4) {
    ERROR("Invalid number of arguments: prog <fullfilepath> <filesize> <read/write step>");
  }

  uint64_t filesize = atoll(argv[2]);
  if (filesize <=0) {
    ERROR("Invalid filesize");
  }

  uint64_t step = atoll(argv[3]);
  if (step <=0) {
    ERROR("Invalid step length");
  }

  const char * filepath = argv[1];

  
  init_datafile(filepath, filesize);

  PMFile* pmf = pmfopen(filepath,"r+");

  struct timeval start, end;

  gettimeofday(&start, NULL);
 
  pmfread(pmf, step, STREAM);
  pmfwrite(pmf, step, STREAM);

  gettimeofday(&end, NULL);

  printf("stream read/write: %ldms\n", ((end.tv_sec * 1000000 + end.tv_usec)
                                      - (start.tv_sec * 1000000 + start.tv_usec)) / 1000);
  

  gettimeofday(&start, NULL);
  
  pmfread(pmf, step, MMAP);
  pmfwrite(pmf, step, MMAP);
  
  gettimeofday(&end, NULL);

  printf("mmap read/write: %ldms\n", ((end.tv_sec * 1000000 + end.tv_usec)
                                      - (start.tv_sec * 1000000 + start.tv_usec)) / 1000);

  pmfclose(pmf);
  
  return 0;
}
