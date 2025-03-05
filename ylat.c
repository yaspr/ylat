#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "../../../ynotif/ynotif.h"

#define MAX_BUFFER 4096

void  *pos = NULL;
void **mem = NULL;

static inline u64 ybench_rand(const u64 n)
{
  return ((u64)(((u64)rand() << 48) ^ ((u64)rand() << 32) ^ ((u64)rand() << 16) ^ (u64)rand())) % n;
}

f64 rpc(const u64 size, const u64 iter)
{
  f64 elapsed = 0.0;
  f64 ns_per_iter = 0.0;
  struct timespec t1, t2;
  
  if (!pos)
    pos = &mem[0];
  
  //Initialize
  for (u64 i = 0; i < size; i++)
    mem[i] = &mem[i];

  //Shuffle
  for (u64 i = size - 1; i > 0; i--)
    {
      u64 ii = ybench_rand(i);
      
      void *tmp = mem[i];
      
      mem[i] = mem[ii];
      mem[ii] = tmp;
    }
  
  //Random pointer chase
  void *p = pos;

  do
    {
      clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

      for (u64 i = iter; i; i--)
	{
	  p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
          p = *(void **)p;
	}

      clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
      
      elapsed = (f64)(t2.tv_nsec - t1.tv_nsec);
    }
  while (elapsed <= 0.0);
  
  pos = p;

  ns_per_iter = elapsed / ((f64)iter * 16.0);

  return ns_per_iter;
}

void measure_cache_latency(const ascii *fpath, u64 max_size)
{
  printf("# Running cache latency benchmark with size: %llu B, %.2lf KiB, %.2lf MiB, %.2lf GiB\n\n",
	 max_size,
	 (f64)max_size / (1024.0),
	 (f64)max_size / (1024.0 * 1024.0),
	 (f64)max_size / (1024.0 * 1024.0 * 1024.0));
  
  u64 rounds = 0;
  u64 nb_dumps = 0;  
  u64 min_iter = 1024;
  u64 max_iter = 8388608;
  u64 iter     = max_iter;
  f64 nanos[MAX_BUFFER];
  u64 sizes[MAX_BUFFER];
  
  FILE *fp = fopen(fpath, "wb");
  
  if (!fp)
    ynotif_error("measure_cache_latency", YNOTIF_EXIT, "cannot create file '%s'\n", fpath);
  
  mem = malloc(max_size);
  
  if (!mem)
    ynotif_error("measure_cache_latency", YNOTIF_EXIT, "cannot allocate memory for cache latency benchmark\n");
  
  for (u64 i = 0; i < (max_size / sizeof(void *)); i++)
    mem[i] = &mem[i];
  
  for (u64 size = 512, step = 8; size <= (max_size / sizeof(void *)); size += step)
    {
      f64 ns_per_iter = rpc(size, iter);
      
      printf("%llu\t%lf\n", size * sizeof(void *), ns_per_iter);
      
      sizes[rounds] = size * sizeof(void *);
      nanos[rounds] = ns_per_iter;
      rounds++;
      
      if (!((size - 1) & size))
	step <<= 1;
      
      iter = (u64)((f64)max_iter / ns_per_iter);
      
      if (iter < min_iter)
	iter = min_iter;
      
      //Dump partial data (MAX_BUFFER) 
      if (rounds == MAX_BUFFER)
	{
	  printf("# Dumping round %llu of the cache latency data\n", nb_dumps);
	  
	  for (u64 i = 0; i < MAX_BUFFER; i++)
	    fprintf(fp, "%llu\t%lf\n", sizes[i], nanos[i]);
	  
	  rounds = 0;
	  nb_dumps++;
	}
    }
  
  printf("\n# Dumping the final round of the cache latency data\n");
  
  for (u64 i = 0; i < rounds; i++)
    fprintf(fp, "%llu\t%lf\n", sizes[i], nanos[i]);
  
  fclose(fp);
  
  free(mem);
  
  printf("# Cache latency benchmark done. Data written to '%s'.\n", fpath);
}

i32 main(i32 argc, ascii **argv)
{
  if (argc < 3)
    return printf("Usage: %s [output file path] [size limit in bytes]\n", argv[0]), 1;
  
  u64 size = atoll(argv[2]);
  
  if (size < 4096)
    ynotif_error("main", YNOTIF_EXIT, "size must be greater or equal to 512, current value is '%llu'\n", size);
  
  measure_cache_latency(argv[1], size);
  
  return 0;
}
