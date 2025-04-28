#ifndef R2_QUEUE_H_
#define R2_QUEUE_H_
#include "r2_types.h"
/**
 * A queue is a linear data structure that follows the first in first out (FIFO) philosophy where elements are removed according
 * to the insertion order. A queue is reminiscent of cashier lines where the order in which you joined determines the order you're served. 
 * A queue has two main operations like its counterpart the stack, mainly enqueue and dequeue. 
 * Both of these operations run in constant time or O(1). 
 * 
 */
struct r2_queuenode{
    void *data;/*data*/
    struct r2_queuenode *next;/*links to the next element in the queue*/
};

struct r2_queue{
    struct r2_queuenode *front;/*first element in the queue*/
    struct r2_queuenode *rear;/*last element in the queue*/
    r2_int64 qsize; /*number of elements in the queue*/
    r2_cmp cmp;/*A comparison callback function*/
    r2_cpy cpy;/*A callback function to copy values*/
    r2_fd  fd;/*A callback function that release memory*/
};

struct r2_queue* r2_create_queue(r2_cmp, r2_cpy, r2_fd);
static struct r2_queuenode*  r2_create_queuenode();
struct r2_queue* r2_destroy_queue(struct r2_queue *);
r2_uint16  r2_queue_enqueue(struct r2_queue *, void *);
r2_uint16  r2_queue_dequeue(struct r2_queue*);
struct r2_queuenode* r2_queue_front(const struct r2_queue*);
struct r2_queuenode* r2_queue_rear(const struct r2_queue*);
r2_uint16  r2_queue_empty(const struct r2_queue*);
struct r2_queue*  r2_queue_copy(const struct r2_queue *);
r2_uint16  r2_queue_compare(const struct r2_queue*, const struct r2_queue *);        
#endif 
