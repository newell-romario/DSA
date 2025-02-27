#ifndef R2_RING_BUFFER_H_
#define R2_RING_BUFFER_H_
#include "r2_types.h"

/**
 * A ring is a circular buffer in the simplest terms. A ring uses First in First Out (FIFO) policy
 * similar to a queue. The  only difference between a queue and ring is that a ring overwrites the 
 * oldest element whenever it becomes full.
 * 
 */
struct r2_ring{
        void **data;/*array that stores data item*/
        r2_uint64 front; /*front of ring*/
        r2_uint64 rear;/*rear of ring*/
        r2_uint64 ncount;/*number of items in ring*/
        r2_uint64 rsize;/*size of the ring*/
        r2_cmp cmp;/*A comparison callback function*/
        r2_cpy cpy; /*A callback function to copy values*/
        r2_fd  fd;/*A callback function that release memory*/
}; 

struct r2_ring* r2_create_ring(r2_uint64,r2_cmp, r2_cpy, r2_fd);
struct r2_ring* r2_destroy_ring(struct r2_ring *);
void r2_ring_insert(struct r2_ring *, void *); 
void r2_ring_delete(struct r2_ring *);
struct r2_ring* r2_ring_copy(const struct r2_ring *);
r2_uint16 r2_ring_compare(const struct r2_ring*, const struct r2_ring*);
void*  r2_ring_front(const struct r2_ring *); 
void*  r2_ring_at(const struct r2_ring*, r2_uint64);
r2_uint16 r2_ring_empty(const struct r2_ring *);

#endif