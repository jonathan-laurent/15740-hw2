#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "atomic.h"
#include <assert.h>
#include "func_time.h"

#define DEBUG

#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf
#endif

/** @brief The number of threads to use to compute the matrix multiplication */
#define THREADS 32
/** @brief The block size to use */
#define BLOCK 16
/** @brief The value (in blocks) of the width and height of the matrix */
#define SIZE 1
/** @brief the size of the matrix in bytes */
#define MATRIX_SIZE_BYTES ((BLOCK * SIZE) * (BLOCK * SIZE) * sizeof(int))
/** @brief The maximum measurement error for timing functions */
#define ERR_MAX 0.001

/** @brief Represents the coordinates of a block in the matrix */
typedef struct {
	int row; /**< The block's row */
	int col; /**< The block's column */
} coord_t;

pthread_mutex_t mutex;

/* the matrices to multiply - we compute the equation A * B = C */
static int A[SIZE * BLOCK][SIZE * BLOCK];
static int B[SIZE * BLOCK][SIZE * BLOCK];
static int C[SIZE * BLOCK][SIZE * BLOCK];

/* matrix to hold a reference brute force solution for verification */
static int C_sol[SIZE * BLOCK][SIZE * BLOCK];

/** @brief Next block location that will be accessed by a thread */
static coord_t next_location;
/** @brief Lock to synchronize access to the next block location */
pthread_mutex_t next_location_lock;

/**
 * @brief Initializes the three matrices to their starting values.
 *
 * @return void
 */
void init_matrices(void) {
	srand(time(NULL));
	for (int r = 0; r < BLOCK * SIZE; r++) {
		for(int c = 0; c < BLOCK * SIZE; c++) {
			A[r][c] = rand();
			B[r][c] = rand();
			C[r][c] = 0;
		}
	}
}

/**
 * @brief Gets a block of the matrix to operate on.
 *
 * @param loc A struct to fill in with the coordinates of the block to
 * operate on.
 *
 * @return Zero on success or a negative error code if every block in the
 * matrix has already been claimed.
 */
int get_block(coord_t *loc) {

	/* lock the next location metadata */
	pthread_mutex_lock(&next_location_lock);

	if (next_location.row < 0 || next_location.col < 0) {
		pthread_mutex_unlock(&next_location_lock);
		return -1;
	}

	/* claim a new block in the matrix */
	loc->row = next_location.row;
	loc->col = next_location.col;

	/* update the block that the next thread will take */
	next_location.col = (next_location.col + 1) % SIZE;
	if (next_location.col == 0) {
		next_location.row = (next_location.row + 1) % SIZE;
		if(next_location.row == 0) {
			next_location.row = -1;
			next_location.col = -1;
		}
	}

	/* release lock on next location metadata */
	pthread_mutex_unlock(&next_location_lock);
	return 0;
}

/**
 * @brief Runs matrix multiplication on a sub block of the larger matrices
 * A, B, and C.
 *
 * @param block The coordinates representing the block to operate on.
 */
void mm_block(coord_t *block) {
    for (int rr = 0; rr < BLOCK; ++rr) {
        for (int cc = 0; cc < BLOCK; ++cc) {

        	int r = rr + (block->row * BLOCK);
        	int c = cc + (block->col * BLOCK);

        	for (int i = 0; i < BLOCK*SIZE; i++) {
        		int delta = A[r][c] * B[c][i];
        		atomic_increment(&C[r][i], delta);
        	}
        }
    }
}

/**
 * @brief Main routine for each thread we're using in the matrix
 * multiplication.
 *
 * @note Every thread simply tries to claim a new block to operate on, and
 * then performs the matrix multiplication calculation on that sub block of
 * the larger matrix.
 *
 * @param arg Unused, here for pthread_create to type check.
 *
 * @return NULL, also used for pthread_create to type check.
 */
void *mm_thread_main(void *arg) {
	coord_t block;
	while (get_block(&block) >= 0) {
		mm_block(&block);
	}
	return NULL;
}

/**
 * @brief A brute force single threaded matrix multiply function used for
 * verification of the parallel solution.
 *
 * @return void
 */
void mm_basic(void) {
	for(int i = 0; i < SIZE * BLOCK; i++) {
		for(int j = 0; j < SIZE * BLOCK; j++) {
			C_sol[i][j] = 0;
			for(int k = 0; k < SIZE * BLOCK; k++) {
				C_sol[i][j] += A[i][k] * B[k][j];
			}
		}
	}
}

/**
 * @brief Runs the parallel matrix multiplication function with a defined
 * number of threads.
 *
 * @note Each thread will claim a portion of the matrix to operate on, and will
 * return once it detects that there no more unclaimed portions. Once all
 * threads exit, the matrix multiplication is complete.
 *
 * @return void
 */
void mm_parallel(void) {
	pthread_t *threads = malloc(THREADS * sizeof(pthread_t));
	if (threads == NULL) {
		dbg_printf("Error: malloc returned NULL.\n");
		return;
	}

    for (int i = 0; i < THREADS; i++) {
    	pthread_create(&threads[i], NULL, mm_thread_main, NULL);
    }

    for(int i = 0; i < THREADS; i++) {
    	pthread_join(threads[i], NULL);
    }

    free(threads);
}

/**
 * @brief Check that the parallel solution matches the brute force solution.
 *
 * @return void
 */
void test_mm_parallel(void) {
	mm_basic();
	for(int i = 0; i < SIZE * BLOCK; i++) {
		for(int j = 0; j < SIZE * BLOCK; j++) {
			assert(C[i][j] == C_sol[i][j]);
		}
	}
	dbg_printf("Success!\n");
}

void time_mm_parallel(void) {
	double time = func_time(mm_parallel, ERR_MAX);
	double mbps = (MATRIX_SIZE_BYTES / time) / 10e3;
	printf("THREADS=%d, BLOCK=%d, Size=%db x %db: %f Mbps (time=%lfms)\n",
		THREADS, BLOCK, SIZE, SIZE, mbps, time * 10e3);
}

/**
 * @brief Spawns a pool of threads to perform matrix multiplication and waits
 * for them to all finish.
 *
 * @param argc The argc
 * @param argv The argv
 *
 * @return { description_of_the_return_value }
 */
int main(int argc, char *argv[]) {
    init_matrices();
    time_mm_parallel();
    return 0;
}
