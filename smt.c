#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "func_time.h"

// size of the array each thread has to access
#define WORKSIZE (1 << 16)
// max number of threads we'll ever have
#define MAX_THREADS 10

// array of arrays (one per thread), each thread increments its entire array
static int arr[MAX_THREADS][WORKSIZE];

static size_t thread_count;

void bind_to_core(void) {
	cpu_set_t cpuset;
	pthread_attr_t attr;

	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	pthread_attr_init(&attr);
	pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);

}

void *work(void *arg) {
	size_t id = (size_t) arg;
	for (int i = 0; i < WORKSIZE; ++i) arr[id][i]++;
	return NULL;
}

void _run_test(void) {
	cpu_set_t cpuset;
	pthread_attr_t attr;

	/* bind all threads to core 0 */
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	pthread_attr_init(&attr);
	pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);

	/* allocate array of threads */
	pthread_t *threads = malloc(thread_count * sizeof(pthread_t));

	/* create each thread and bind it to core 0 */
	for (size_t i = 0; i < thread_count; i++) {
		if (pthread_create(&threads[i], &attr, work, (void *) i) != 0) {
			fprintf(stderr, "pthread_create(): Failed to create thread.");
			exit(-1);
		}
	}

	/* wait for them all to terminate */
	for (size_t i = 0; i < thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
}

void run_test(size_t n_threads) {
	thread_count = n_threads;
	double time = func_time(_run_test, 0.01);
	printf("%lu Threads took %lf ms.\n", n_threads, time * 10e3);
}

int main(int argc, char *argv[]) {
	for (size_t i = 1; i < 10; i++) {
		run_test(i);
	}
	return 0;
}