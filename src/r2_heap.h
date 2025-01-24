#ifndef R2_HEAP_H_
#define R2_HEAP_H_
#include "r2_types.h"
/**
 * A heap is a common way of implementing the priority queue data structure. A priority queue data structure is concerned with
 * storing the elements by their associated priority (i.e. elements with lower priorities are stored first or elements with higher priority are stored first). 
 * A heap specifically a binary heap is a common way of implememnting a priority queue. A binary heap comes in two types
 * either it's a max heap or a min heap. 
 * 
 * A binary heap is fundamentally a binary tree, the binary in it's name comes from a binary tree. In a binary heap the children in the sub tree 
 * are always <= or >=  to the root of the subtree depending on whether it's a min or max heap. A binary heap has a few properties:
 *  1) The height is always log n. 
 *  2) It has maximum number of nodes at all depth except at depth h. 
 *  3) The tree is filled from left to right. 
 * 
 * We can implement a binary heap as either a tree or ingeniously impose a tree structure on an array. Imposing the tree structure on the 
 * array is the most common way of implementing a binary heap. We can use level order numbering to achieve this. Let's define p(r) to 
 * return the position of the root R. If we define the root of the tree to be p(r) = 1, then we can define it's children as 2p(r) and 2p(r) + 1 for 
 * left and right children respectively. With this in mind every node in the tree has a unique number and that unique number can be used to 
 * as an index into the array.
 * 
 * See Introduction to Algorithms by CLRS for a better explanation. 
 * 
 * Some of the common functionality of heaps are: 
 *      1)Return smallest or largest element
 *      2)Remove smallest or largest element
 *      3)Insert an element
 *
 * We'll discuss implementing a binary heap in more details below: 
 * ____________________________________________
 * Insertion
 * ____________________________________________
 * Inserting an element in a binary heap may break the fundamental property of a heap that is it's children must be <= or >= to depending on the type of heap. We
 * fix this by a process called heapify specifically up bubbling. When inserting an element into an heap we insert the element at the end of the array
 * and then move it to it's correct position through a series of swaps with parents which means we're slowly climbing the  tree hence up bubbling. 
 * 
 * ___________________________________________
 * Deleting an element 
 * ___________________________________________
 * Deletion of an element in a binary heap may break the fundamental property of a heap as well. We fix this by bubbling down the heap. If we delete 
 * the root of a tree we replace it with the element at the end of the array. We slowly bubble this element down the tree finding it's correct position.
 * 
 * See Introduction to Algorithms by CLRS for a better explanation or Algorithms by Sedgewick.
 */


/**
 * @brief We use this to map data to a position in the heap.
 * 
 */
struct r2_locator{
        r2_uint64 pos;/*position*/
        void *data;/*data*/
};

struct r2_pq{
        struct r2_locator **data;/*stores data along with position in heap*/
        r2_uint16 type;/*type of heap*/
        r2_uint64 ncount;/*current number of elements*/
        r2_uint64 pqsize;/*size of pq*/ 
        r2_cmp kcmp;/*A callback comparison function*/
        r2_fd  fd;/*A callback function frees memory used by data*/
        r2_cpy cpy;/*A callback function to copy key*/
};

struct r2_pq* r2_create_priority_queue(r2_uint64, r2_uint16, r2_cmp, r2_fd, r2_cpy);
struct r2_pq* r2_destroy_priority_queue(struct r2_pq *);
struct r2_locator* r2_pq_insert(struct r2_pq*, void *);
struct r2_locator* r2_pq_first(struct r2_pq *);
struct r2_pq* r2_pq_remove(struct r2_pq *, struct r2_locator *);
r2_uint16 r2_pq_empty(const struct r2_pq *);
struct r2_pq* r2_pq_adjust(struct r2_pq *, struct r2_locator *, r2_uint16);
#endif
/*0x75d5a0*/