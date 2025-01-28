#include "r2_heap.h"
#include <stdlib.h>
#define  PQSIZE 16
/********************File scope functions************************/
static struct r2_pq* r2_bubble_down(struct r2_pq *, r2_uint64);
static struct r2_pq* r2_bubble_up(struct r2_pq *, r2_uint64);
static void r2_free_data(r2_fd, struct r2_locator *);
static r2_uint16  r2_pq_resize(struct r2_pq *, r2_uint64);
/********************File scope functions************************/


/**
 * @brief                       Creates an empty priority queue. N.B This is an extendable priority queue.
 * 
 * @param pqsize                Priority queue size. If the size is 0 we default to a size of 32.
 * @param type                  Type represents whether the heap is a min or max heap. 0 == min heap or 1 => max heap.
 * @param kcmp                  A comparison callback function.
 * @param fd                    A callback function that frees memory used by data.
 * @param kcpy                  A callback function to copy key.
 * @return struct r2_pq*        Returns priority queue, else NULL. 
 */
struct r2_pq* r2_create_priority_queue(r2_uint64 pqsize, r2_uint16 type ,r2_cmp kcmp, r2_fd fd, r2_cpy kcpy)
{
        pqsize = (PQSIZE > pqsize? PQSIZE : pqsize);
        struct r2_pq *pq = malloc(sizeof(struct r2_pq)); 
        if(pq != NULL){
                /*We add 1 because the pq->data[0] is never used.*/
                pq->data   = malloc(sizeof(struct r2_locator *) * (pqsize + 1)); 
                pq->fd     = fd;
                pq->type   = type; 
                pq->pqsize = pqsize; 
                pq->kcmp   = kcmp;
                pq->cpy    = kcpy; 
                pq->ncount = 0;
                if(pq->data != NULL){
                        for(r2_uint64 i = 0; i <= pqsize; ++i)
                                pq->data[i] = NULL;
                }else{
                        free(pq);
                        pq = NULL;
                }
        }
        return pq;
}

/**
 * @brief                       Destroys priority queue.
 * 
 * @param pq                    Priority Queue.
 * @return struct r2_pq*        Returns NULL whenever priority queue is destroyed properly.
 */
struct r2_pq* r2_destroy_priority_queue(struct r2_pq *pq)
{
        for(r2_uint64 i = 1; i <=  pq->ncount; ++i)
                r2_free_data(pq->fd, pq->data[i]);

        free(pq->data); 
        free(pq);
        return NULL;
}

/**
 * @brief               Checks whether a priority queue is empty.
 * 
 * @param pq            Priority Queue.
 * @return r2_uint16    Returns TRUE when priority queue is empty, else FALSE.
 */
r2_uint16 r2_pq_empty(const struct r2_pq *pq)
{
        return pq->ncount == 0;
}

/**
 * @brief                       Repairs priority queue after deletion.
 *                              After deletion it is possible that a priority queue violates the property
 *                              that the root must be <= or >= to it's children. This function maintains 
 *                              the property.
 * 
 * @param pq                    Priority queue.
 * @param parent                Root.                
 * @return struct r2_pq*        Returns priority queue.
 */
static struct r2_pq* r2_bubble_down(struct r2_pq *pq,  r2_uint64 parent)
{
        r2_uint64 left      = 0;/*left child*/
        r2_uint64 right     = 0;/*right child*/
        r2_uint64 cswap     = 0;/*child that will be swapped with parent*/ 
        do{
                left  = 2 * parent;
                right = 2 * parent + 1;
                if(right <= pq->ncount){
                        if(pq->kcmp(pq->data[left]->data, pq->data[right]->data) == pq->type)
                                cswap = left;
                        else    cswap = right;
                }else if(left <= pq->ncount)
                        cswap = left;
                else 
                        break;
                
                if(pq->kcmp(pq->data[cswap]->data, pq->data[parent]->data) == pq->type){
                        struct r2_locator *temp = pq->data[cswap];
                        pq->data[cswap]         = pq->data[parent];
                        pq->data[cswap]->pos    = cswap;
                        pq->data[parent]        = temp;
                        pq->data[parent]->pos   = parent;
                        parent = cswap;
                }else
                        break; 
        }while(parent < pq->ncount);

        return pq;
}

/**
 * @brief                       Repairs a priority queue that has been violated due to insertion.
 * 
 * @param pq                    Priority queue.
 * @param root                  Root.                
 * @return struct r2_pq*        Returns priority queue.
 */
static struct r2_pq* r2_bubble_up(struct r2_pq *pq, r2_uint64 root)
{
        r2_uint64 parent = root / 2; 
        while(root > 1){
                if(pq->kcmp(pq->data[root]->data, pq->data[parent]->data) == pq->type){
                        struct r2_locator *temp = pq->data[root];
                        pq->data[root]        = pq->data[parent];
                        pq->data[root]->pos   = root;
                        pq->data[parent]      = temp;
                        pq->data[parent]->pos = parent;
                        root = parent;
                }else
                        break;
                parent = root / 2;
        }
        return pq;
}

/**
 * @brief                       Returns the root of the queue.
 * 
 * @param pq                    Priority Queue.
 * @return struct r2_locator*   Returns root.
 */
struct r2_locator* r2_pq_first(struct r2_pq *pq)
{
        return pq->data[1];
}

/**
 * @brief                       Inserts an element in the priority queue.
 * 
 * @param pq                    Priority Queue.
 * @param data                  Data.
 * @return struct r2_locator*   Returns locator for element.
 */
struct r2_locator* r2_pq_insert(struct r2_pq *pq, void *data)
{
        r2_uint16 RESIZE = TRUE;
        struct r2_locator *l = NULL;
        /**
         * @brief In the heap only the positions from data[1...n-1] are available. 
         */
        if(pq->ncount == pq->pqsize)
                RESIZE = r2_pq_resize(pq, pq->pqsize * 2);
        
        if(RESIZE == TRUE){
                l  = malloc(sizeof(struct r2_locator));
                if(l != NULL){
                        ++pq->ncount;
                        l->data = data;
                        l->pos  = pq->ncount;
                        pq->data[pq->ncount] = l;
                        pq = r2_bubble_up(pq, pq->ncount);
                }
        }

        return  l;
}

/**
 * @brief               Resize priority queue.
 * 
 * @param pq            Priority Queue.
 * @param size          Size.
 * @return r2_uint16    Returns TRUE whenever resize is successful, else FALSE.
 */
static r2_uint16  r2_pq_resize(struct r2_pq *pq, r2_uint64 size)
{
        r2_uint16 RESIZE = FALSE; 
        struct r2_locator **data = malloc(sizeof(void *) * (size + 1));
        if(data != NULL){
                for(r2_uint64 i = 0; i <= size; ++i)
                        data[i] = NULL;

                for(r2_uint64 i = 0; i <= pq->ncount; ++i)
                        data[i] = pq->data[i]; 
                
                RESIZE = TRUE;
                free(pq->data); 
                pq->data   = data;
                pq->pqsize = size;
        }
        return RESIZE; 
}

/**
 * @brief                       Removes the root.
 * 
 * @param pq                    Priority Queue.
 * @param loc                   Root location.
 * @return struct r2_pq*        Returns priority queue.
 */
struct r2_pq* r2_pq_remove(struct r2_pq *pq, struct r2_locator *loc)
{
        if(r2_pq_empty(pq) != TRUE){
                r2_uint64 root = loc->pos;
                pq->data[root] = pq->data[pq->ncount];
                pq->data[root]->pos  = root;
                r2_free_data(pq->fd, loc);

                pq->data[pq->ncount] = NULL;
                --pq->ncount;
                pq = r2_bubble_down(pq, root);
                if(pq->ncount > PQSIZE && pq->ncount <= (pq->pqsize / 4))
                        r2_pq_resize(pq, pq->pqsize / 2);
        }
        return pq;
}



static void r2_free_data(r2_fd fd, struct r2_locator *l)
{
        if(fd != NULL)
                fd(l->data);
      
        free(l);
}

/**
 * @brief                  Adjusts the priority of an element in the priority queue.   
 * 
 * @param pq               Priority Queue.
 * @param loc              Locator.
 * @param adjust           Adjustment. if adjust == 0 => priority increased, else 1 => priority decreased.
 * @return struct r2_pq*   Returns priority queue.
 */
struct r2_pq* r2_pq_adjust(struct r2_pq *pq, struct r2_locator *loc, r2_uint16 adjust)
{
        return adjust == 0? r2_bubble_up(pq, loc->pos) : r2_bubble_down(pq, loc->pos);
}