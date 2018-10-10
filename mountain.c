#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////

#define PURGE_CACHES  1
#define PURGE_SIZE    (1 << 22) // 4MB
#define CLOCK_ID      CLOCK_PROCESS_CPUTIME_ID //CLOCK_REALTIME
#define MIN_REPEAT    512    // Repeat each experiment at least ... times

#define LOGALIGN      26     // Align &data on a multiple of 64MB
#define TOLERANCE     1e-5   // Precision of time measurements
#define STRIDE_MIN    1
#define STRIDE_MAX    32
#define LOGSIZE_MIN   10     // Should be at least sizeof(uint64_t)
#define LOGSIZE_MAX   30

typedef double data_t;

////////////////////////////////////////////////////////////////////////////////

// Simple timer functions

typedef struct timespec timespec;

typedef void (*test_funct)(void);

// Clock resolution (should be about one microsecond)
double delta;
void init_delta() {
  timespec res;
  clock_getres(CLOCK_ID, &res);
  delta = (double) res.tv_nsec * 1e-9;
}

double timespec_diff(timespec *start, timespec *end) {
  double sec_diff  = (double) (end->tv_sec  - start->tv_sec );
  double nsec_diff = (double) (end->tv_nsec - start->tv_nsec);
  return sec_diff + nsec_diff * 1e-9;
}

double func_time(test_funct f, double tolerance) {
  timespec start, end;
  double elapsed;
  long n = MIN_REPEAT;
  while(1) {
    clock_gettime(CLOCK_ID, &start);
    for(long i = 0; i < n; i++) { f(); }
    clock_gettime(CLOCK_ID, &end);
    elapsed = timespec_diff(&start, &end);
    if(elapsed >= delta / tolerance) { break; }
    n *= 2;
  }
  return elapsed / n;
}

////////////////////////////////////////////////////////////////////////////////

data_t *data;     // Data (has to be large enough)
long size_param;    // Size of the array (in Bytes)
long stride_param;  // Stride (in words of 8 Bytes)

// Loads size/stride bytes of memory
void test_simple() {
  long n = size_param / sizeof(data_t);
  long stride = stride_param;
  data_t result = 0;
  volatile data_t sink;
  for (long i = 0; i < n; i += stride) {
    result += data[i];
  }
  sink = result; // So compiler does not optimize away the loop
}


////////////////////////////////////////////////////////////////////////////////

data_t *dummy;
long dummy_len;

void allocate_dummy(long size) {
  posix_memalign((void**) &dummy, 1 << LOGALIGN, size);
  dummy_len = size / sizeof(data_t);
}

void purge_caches() {
  volatile data_t x = 0;
  for(long i = 0; i < dummy_len; i++) {
    x += dummy[i];
  }
}

////////////////////////////////////////////////////////////////////////////////

void take_measurements(int simple_mode) {

  test_funct test;
  if (simple_mode) {
    test = test_simple;
  }
  else {
    test = test_simple;
  }

  long data_size = 1 << LOGSIZE_MAX;
  posix_memalign((void**) &data,  1 << LOGALIGN, data_size);

  allocate_dummy(PURGE_SIZE);

  // Make the measurements
  for(long logsize = LOGSIZE_MAX; logsize >= LOGSIZE_MIN; logsize--) {
    fprintf(stderr, "logsize=%ld  \r", logsize);
    for(long stride = STRIDE_MIN; stride <= STRIDE_MAX; stride++) {
      if(PURGE_CACHES) purge_caches();
      size_param = 1 << logsize;
      stride_param = stride;
      double time = func_time(test, TOLERANCE);
      double accessed = ((double) size_param) / stride_param;
      double speed = accessed / (time * 1024 * 1024); // MB/s
      printf("%-3ld  %-3ld  %.1lf\n", stride, logsize, speed);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

const char *arg_error = \
  "This program expects one argument ('simple' or 'better').";

int main (int argc, char *argv[]) {

  int simple = 1;
  if (argc != 2) {
    fprintf(stderr, "%s\n", arg_error);
    return 1;
  }
  if (strcmp(argv[1], "simple") == 0) {
    fprintf(stderr, "Using 'simple' measurement mode.\n");
  }
  else if (strcmp(argv[1], "better") == 0) {
    fprintf(stderr, "Using 'better' measurement mode.\n");
    simple = 0;
  }
  else {
    fprintf(stderr, "%s\n", arg_error);
    return 1;
  }

  fprintf(stderr, "Size of data_t: %luB\n", sizeof(data_t));

  init_delta();
  take_measurements(simple);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
