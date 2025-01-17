#ifndef R2_ARRAY_STACK_H_
#define R2_ARRAY_STACK_H_
#include "r2_types.h"

/**
 * A stack is a linear data structure that follows the last in first out (LIFO) philosophy where data is inserted only at the front 
 * and removed only at the front. A stack would be reminiscent of a pack of playing cards where one has to remove the topmost card to 
 * see the next. The insertion operation of a stack is called a push while the deletion is called a pop. These two operations have a 
 * time complexity of O(1). A stack can be implemented either as an array based structure or list based structure. 
 * For a more thorough treatment of the stack operations reference Algorithms by Robert Sedgewick and Kevin Wayne.
 * 
 * A resizeable array implementation is below.
 * 
 */
struct r2_arrstack{
        void **data; /*data array*/
        r2_uint64 top; /*position where next element will be inserted*/
        r2_uint64 ncount; /*number of elements*/
        r2_uint64 ssize; /*size of stack*/
        r2_fd fd; /*A callback function to free each item in data array*/; 
        r2_cpy cpy; /*A callback function used to copy each item in data array */
        r2_cmp cmp; /*A comparison callback function used to compare data items in array*/
}; 

struct r2_arrstack* r2_arrstack_create_stack(r2_uint64, r2_fd fd, r2_cpy, r2_cmp);
struct r2_arrstack* r2_arrstack_destroy_stack(struct r2_arrstack *);
struct r2_arrstack* r2_arrstack_push(struct r2_arrstack *, void *); 
struct r2_arrstack* r2_arrstack_pop(struct r2_arrstack *);
void*  r2_arrstack_top(const struct r2_arrstack *);
r2_uint16   r2_arrstack_empty(const struct r2_arrstack *);
r2_uint16   r2_arrstack_full(const struct r2_arrstack *);
r2_uint16   r2_arrstack_compare(const struct r2_arrstack *, const struct r2_arrstack*);
struct r2_arrstack* r2_arrstack_copy(const struct r2_arrstack *);
#endif