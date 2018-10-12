#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NITER     (1 << 30)
#define ALIGN     (1 << 26)
#define LINE_MIN  4
#define LINE_MAX  1024
#define CLOCK_ID  CLOCK_MONOTONIC

////////////////////////////////////////////////////////////////////////////////

typedef struct timespec timespec;

double timespec_diff(timespec *start, timespec *end) {
  double sec_diff  = (double) (end->tv_sec  - start->tv_sec );
  double nsec_diff = (double) (end->tv_nsec - start->tv_nsec);
  return sec_diff + nsec_diff * 1e-9;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t *data;

void init_data() {
   posix_memalign((void**) &data, ALIGN, 2 * LINE_MAX);
}

void *dumb_work(void* ptr) {
  uint8_t *x = (uint8_t*) ptr;
  for(int i = 0; i < NITER; i++) (*x)++;
  return NULL;
}

double experiment(int n) {
  pthread_t t1, t2;
  timespec start, end;
  clock_gettime(CLOCK_ID, &start);

  pthread_create(&t1, NULL, dumb_work, (void*) (data + 0));
  pthread_create(&t2, NULL, dumb_work, (void*) (data + n));
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  clock_gettime(CLOCK_ID, &end);
  return timespec_diff(&start, &end);
}

////////////////////////////////////////////////////////////////////////////////

int main (int argc, char *argv[]) {

  init_data();

  printf("sep   Mops/s \n");
  printf("----  ------\n");
  for(int n = LINE_MIN; n <= LINE_MAX; n *= 2) {
    double time = experiment(n);
    double freq = ((double) NITER) / (1e6 * time);
    printf("%-4d  %5.1lf\n", n, freq);
  }

  return 0;

}
