#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////

#define NITER 1
#define NTHREADS_MAX 8
#define CLOCK_ID CLOCK_MONOTONIC

////////////////////////////////////////////////////////////////////////////////

typedef struct timespec timespec;

double timespec_diff(timespec *start, timespec *end) {
  double sec_diff  = (double) (end->tv_sec  - start->tv_sec );
  double nsec_diff = (double) (end->tv_nsec - start->tv_nsec);
  return sec_diff + nsec_diff * 1e-9;
}

////////////////////////////////////////////////////////////////////////////////

void *dumb_work() {
  volatile uint32_t x = 0;
  for(int i = 0; i < NITER; i++) while(++x);
  return NULL;
}

pthread_t tid[NTHREADS_MAX];

double experiment(int n) {

  timespec start, end;
  clock_gettime(CLOCK_ID, &start);

  for(int i = 0; i < n-1; i++)
    pthread_create(&tid[i], NULL, dumb_work, NULL);

  dumb_work();

  for(int i = 0; i < n-1; i++)
    pthread_join(tid[i], NULL);

  clock_gettime(CLOCK_ID, &end);
  return timespec_diff(&start, &end);

}

////////////////////////////////////////////////////////////////////////////////

int main (int argc, char *argv[]) {

  printf("n  time    \n");
  printf("-- ------\n");
  for(int n = 1; n <= NTHREADS_MAX; n++) {
    double time = experiment(n);
    printf("%-2d %5.2lf\n", n, time);
  }

  return 0;

}
