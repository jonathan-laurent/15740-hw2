#ifndef _ATOMIC_H_
#define _ATOMIC_H_

extern int atomic_swap(int *dst, int val);
extern int atomic_increment(int *dst, int delta);

#endif /* _ATOMIC_H_ */
