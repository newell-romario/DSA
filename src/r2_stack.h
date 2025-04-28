#ifndef         R2_STACK_H_
#define         R2_STACK_H_
#include        "r2_types.h"

/**
 * A stack is a linear data structure that follows the last in first out (LIFO) philosophy where data is inserted only at the front 
 * and removed only at the front. A stack would be reminiscent of a pack of playing cards where one has to remove the topmost card to 
 * see the next. The insertion operation of a stack is called a push while the deletion is called a pop. These two operations have a 
 * time complexity of O(1). A stack can be implemented either as an array based structure or list based structure. 
 * For a more thorough treatment of the stack operations reference Algorithms by Robert Sedgewick and Kevin Wayne.
 * 
 * A linked list implementation is below.
 * 
 */
struct r2_stacknode{
        void *data; /*data portion of node*/
        struct r2_stacknode *next; /*links the next node to the current node*/
};

struct r2_stack{
        struct r2_stacknode *top; /*top of stack*/
        r2_int64 ssize;/*number of elements in stack*/
        r2_cmp cmp; /*A callback comparison function*/
        r2_cpy cpy; /*A callback copy function*/
        r2_fd fd; /*A callback function to free memory occupied by data*/           
};



struct r2_stack* r2_create_stack(r2_cmp, r2_cpy, r2_fd);
struct r2_stack* r2_destroy_stack(struct r2_stack *);
r2_uint16  r2_stack_push(struct r2_stack *,void *);
r2_uint16  r2_stack_pop(struct r2_stack *);
struct r2_stacknode*   r2_stack_peek(const struct r2_stack *);
r2_int16 r2_stack_empty(const struct r2_stack *);
struct r2_stack*   r2_stack_copy(const struct r2_stack *);
r2_uint16 r2_stack_compare(const struct r2_stack *, const struct r2_stack*);
#endif
