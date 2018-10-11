#include "func_time.h"
#include "perf.h"
#include <time.h>
#include <stdio.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define SAMPLE_SIZE 1000

/**
 * @brief Computes an estimate for the value of delta on the system's
 * interval timer.
 *
 * @return The estimated value of delta in seconds.
 */
long double get_delta(void)
{
    int i;
    long double accum, estimate, start, end;

    accum = 0;
    init_etime();

    for (i = 0; i < SAMPLE_SIZE; ++i)
    {
        // find smallest granularity of differences in interval timer
        start = get_etime();
        while((end = get_etime()) == start)
            continue;

        estimate = end - start;
        accum += estimate;
    }

    return accum / SAMPLE_SIZE;
}

/**
 * @brief Computes the value of delta for the systems CLOCK_MONOTONIC timer
 * as reported by clock_getres.
 *
 * @note Since this timer is much higher precision than the interval timer,
 * using a tight loop to estimate the value of delta may no longer be accurate,
 * which is why I used clock_getres instead.
 *
 * @return The value of delta for the CLOCK_MONOTONIC timer.
 */
long double get_delta_hw(void)
{
    struct timespec res;
    clock_getres(CLOCK_MONOTONIC, &res);
    long double s = (long double) res.tv_sec;
    long double ns = (long double) res.tv_nsec;
    return s + (ns * 1e-9);
}

/**
 * @brief Computes the time taken to run a function P n-many times using the
 * interval timer.
 *
 * @param P The function to time.
 * @param n The number of executions to time.
 *
 * @return The total time taken for n-many executions of function P.
 */
long double time_for_n_iterations(test_funct P, unsigned n)
{
    unsigned i;
    long double ts, tf;

    init_etime_real();
    ts = get_etime_real();
    for(i = 0; i < n; ++i) P();
    tf = get_etime_real();

    return tf - ts;
}

/**
 * @brief Computes the time taken to run a function P n-many times using a
 * high resolution hardware timer.
 *
 * @param P The function to time.
 * @param n The number of executions to time.
 *
 * @return The total time taken for n-many executions of function P.
 */
long double time_for_n_iterations_hw(test_funct P, unsigned n)
{
    unsigned i;
    long double ts, tf;

    // initialize the timer and warm the cache
    init_etime_hw();
    P();

    // time n-many iterations of the operation
    ts = get_etime();
    for(i = 0; i < n; ++i) P();
    tf = get_etime();

    return tf - ts;
}

/**
 * @brief Computes the minimum T_observed we must see in order to ensure
 * an error below a specified error threshold.
 *
 * @param E The error threshold.
 *
 * @return The minimum T_observed allowable to ensure we meet the error
 * threshold.
 */
long double minimum_observed_time(long double E, long double delta)
{
    return delta + (delta / E);
}

/**
 * @brief Times how long it takes to execute a given function using a doubling
 * procedure to ensure that the measurement error is below a given bound.
 *
 * @param P The function to time.
 * @param E The acceptable measurement error with respect to T_actual.
 *
 * @return An estimate of the running time of function P.
 */
long double func_time(test_funct P, long double E)
{
    unsigned n = 1;
    long double delta = get_delta();
    long double t_threshold = minimum_observed_time(E, delta);

    while (1) {
        long double t_aggregate = time_for_n_iterations(P, n);
        // in order for our math to hold, observed time must be >= delta
        if (t_aggregate >= MAX(delta, t_threshold)) {
            return t_aggregate / n;
        }
        n *= 2;
    }
}

/**
 * @brief Times how long it takes to execute a given function using a doubling
 * procedure to ensure measurement error is within a specified bound. Uses the
 * hardware timer to achieve finer grained measurements.
 *
 * @param P The function to time.
 * @param E The acceptable measurement error with respect to T_actual.
 *
 * @return An estimate of the running time of function P.
 */
long double func_time_hw(test_funct P, long double E)
{
    unsigned n = 1;
    long double delta = get_delta();
    long double t_threshold = minimum_observed_time(E, delta);

    while (1) {
        long double t_aggregate = time_for_n_iterations_hw(P, n);
        // in order for our math to hold, observed time must be >= delta
        if (t_aggregate >= MAX(delta, t_threshold)) {
            return (t_aggregate * 10e3) / n;
        }
        n *= 2;
    }
}
