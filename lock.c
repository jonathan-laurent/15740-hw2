#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <assert.h>
#include "atomic.h"
#include "func_time.h"

static void *(*incr_func)(void*);
static pthread_t *threads;
static sem_t sem;
static int n_threads;
static int n_increments;
static int n = 0;

enum {
	MODE_SEMAPHORE,
	MODE_ATOMIC
};

typedef struct {
	int *n;
	int n_iters;
	sem_t *sem;
} args_t;


void *do_semaphore(void *arg) {
	args_t *args = arg;
	for (int i = 0; i < args->n_iters; i++) {
		sem_wait(args->sem);
		(*args->n)++;
		sem_post(args->sem);
	}
}

void *do_atomic(void *arg) {
	args_t *args = arg;
    for (int i = 0; i < args->n_iters; i++) {
		atomic_increment(args->n, 1);
	}
}

void do_test(void) {
	args_t args  = { .n = &n, .n_iters = n_increments, .sem = &sem };
    n = 0;
	for (int i = 0; i < n_threads; i++) {
		pthread_create(&threads[i], NULL, incr_func, &args);
	}

	for (int i = 0; i < n_threads; i++) {
		pthread_join(threads[i], NULL);
	}
}

/**
 * @brief Prints the usage instructions to stderr.
 *
 * @param argv The argv program's argument vector.
 *
 * @return void
 */
void print_usage(char *argv[]) {
	fprintf(stderr, "Usage: %s <n_threads>\n", argv[0]);
	fprintf(stderr, "Optional Arguments:\n");
	fprintf(stderr, "\t--sem    Use a semaphore\n");
	fprintf(stderr, "\t--atomic Use atomic_increment [DEFAULT]\n");
}

/**
 * @brief Gets the mode to use for this test run.
 *
 * @param argc The number of command line arguments.
 * @param argv Vector of command line arguments.
 *
 * @return The mode to use for this test (atomic/semaphore).
 */
int get_mode(int argc, char *argv[]) {
	for(int i = 0; i < argc; i++) {
		char *s = argv[i];
		if (strcmp(s, "--sem") == 0)
			return MODE_SEMAPHORE;
	}

	return MODE_ATOMIC;
}

inline int *get_index(int *addr, int cols, int r, int c) {
	return addr + (cols * r) + c;
}

void print_2d_array(int *addr, int rows, int cols) {

}

/**
 * @brief Parses the command line arguments, runs the test using the
 * specified parameters, and logs the results.
 *
 * @param argc The number of command line arguments.
 * @param argv A vector of the command line arguments.
 *
 * @return Zero on success or negative error code on failure.s
 */
int main(int argc, char *argv[]) {
	int mode;

	if (argc < 2) {
		print_usage(argv);
		exit(-1);
	}

	n_threads = atoi(argv[1]);
	n_increments = (1 << 20) / n_threads;
	mode = get_mode(argc, argv);
	incr_func = (mode == MODE_SEMAPHORE) ? do_semaphore : do_atomic;

	sem_init(&sem, 0, 1);
	threads = malloc(n_threads * sizeof(pthread_t));

	double time = func_time(do_test, 0.001);
    printf("Using %s with %d thread(s): time = %lfs\n",
            mode == MODE_SEMAPHORE ? "semaphore" : "atomic operations",
            n_threads, time);

	sem_destroy(&sem);
	free(threads);

	assert(n == n_threads * n_increments);

	return 0;
}
