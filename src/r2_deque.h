#ifndef R2_DEQUE_H_
#define R2_DEQUE_H_
#include "r2_types.h"

/**
 * A deque or double ended queue is a linear data structure where insertion and deletion is allowed to happen at both ends. 
 * It's a queue where the restriction of only inserting at the rear is lifted and also the restriction of deleting at the 
 * front is lifted. The main operations of a deque are insert at front/rear and delete at front/rear. The insertion operations 
 * for a deque run in constant time whilst only one of the deletions run in constant time. The deletion at the rear runs in linear 
 * time whilst the deletion at the front runs in constant time. 
 */
struct r2_dequenode{
        void *data;/*data*/
        struct r2_dequenode *next; /*Link to the next dequenode*/
};

struct r2_deque{
        struct r2_dequenode  *front;/*Front of deque*/
        struct r2_dequenode  *rear;/*Rear of deque*/
        r2_cmp cmp;/*A comparison callback function*/
        r2_cpy cpy;/*A callback function to copy values*/
        r2_fd fd;/*A callback function that release memory*/
        r2_uint64 dsize; /*Number of elements in deque*/
};


struct r2_deque*        r2_create_deque(r2_cmp, r2_cpy, r2_fd);
struct r2_dequenode*    r2_create_dequenode();
struct r2_deque*        r2_destroy_deque(struct r2_deque *);
r2_uint16               r2_deque_empty(const struct r2_deque *);
struct r2_deque*        r2_deque_insert_at_front(struct r2_deque *, void*); 
struct r2_deque*        r2_deque_insert_at_back(struct  r2_deque *, void*);
struct r2_deque*        r2_deque_delete_at_front(struct r2_deque  *); 
struct r2_deque*        r2_deque_delete_at_back(struct r2_deque *);
struct r2_dequenode*    r2_deque_front(const struct r2_deque *); 
struct r2_dequenode*    r2_deque_rear(const struct r2_deque *);
struct r2_deque*        r2_deque_copy(const struct r2_deque *);
r2_uint16               r2_deque_compare(const struct r2_deque *, const struct r2_deque *);
#endif