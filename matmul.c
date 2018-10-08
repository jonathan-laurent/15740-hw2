#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

/** @brief The number of threads to use to compute the matrix multiplication */
#define THREADS 10
/** @brief The block size to use */
#define BLOCK 16
/** @brief The value (in blocks) of the width and height of the matrix */
#define SIZE 4

/** @brief Represents the coordinates of a block in the matrix */
typedef struct {
	size_t row; /**< The block's row */
	size_t col; /**< The block's column */
} coord_t;

/* the matrices to multiply - we compute the equation A * B = C */
static uint64_t A[SIZE * BLOCK][SIZE * BLOCK];
static uint64_t B[SIZE * BLOCK][SIZE * BLOCK];
static uint64_t C[SIZE * BLOCK][SIZE * BLOCK];

/** @brief Next block location that will be accessed by a thread */
static coord_t next_location;
/** @brief Lock to synchronize access to the next block location */
pthread_mutex_t next_location_lock;

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
	int ret = 0;

	/* lock the next location metadata */
	pthread_mutex_lock(&next_location_lock);

	/* claim a new block in the matrix */
	loc->row = next_location.row;
	loc->col = next_location.col;

	/* update the block that the next thread will take */
	next_location.col = (next_location.col + 1) % SIZE;
	if (next_location.col == 0) {
		next_location.row = (next_location.row + 1) % SIZE;
	}
	if (next_location.row == 0) {
		ret = -1;
	}

	/* release lock on next location metadata */
	pthread_mutex_unlock(&next_location_lock);
	return ret;
}

int main(int argc, char *argv[]) {
	return 0;
}