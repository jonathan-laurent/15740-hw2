/**
 * @brief Atomically increment a memory location by a specified amount.
 *
 * @param dst The memory address to increment (integer pointer)
 * @param delta The amount to increment by (signed integer)
 *
 * @note: On entry:
 *        %rdi - Contains first argument: dst
 *        %rsi - Contains second argument: delta
 *
 * @return The old value stored at the dst address.
 */
.global atomic_increment
atomic_increment:
    movl %esi, %eax
    xaddl %eax, (%rdi)
    retq

/**
 * @brief Atomically swap a value into a memory location and read the
 * old value.
 *
 * @param dst The memory address to write to
 * @param val The value to write into memory.
 *
 * @note On Entry:
 *       %rdi - Contains first argument: dst
 *       %rsi - Contains second argument: val
 *
 * @return The old value at the specified memory location.
 */
.global atomic_swap
atomic_swap:
    movl %esi, %eax
    xchgl %eax, (%rdi)
    retq
