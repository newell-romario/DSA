#include <stdlib.h>
#include "r2_ring.h"
#include <assert.h>
/**
 * @brief                       Creates an empty ring buffer.
 * 
 * @param size                  Size.
 * @param cmp                   A callback comparison function.
 * @param cpy                   A callback copy function.
 * @param fd                    A callback function to free memory used by data.
 * @return struct r2_ring*      Returns an empty ring buffer, else NULL.
 */
struct r2_ring* r2_create_ring(r2_uint64 rsize, r2_cmp cmp, r2_cpy cpy, r2_fd fd)
{
        struct r2_ring *ring = malloc(sizeof(struct r2_ring));
        if(ring != NULL){
                ring->data = malloc(sizeof(void *) * rsize); 
                if(ring->data == NULL){
                        free(ring); 
                        ring = NULL;
                }else{
                        ring->cmp       = cmp;
                        ring->cpy       = cpy; 
                        ring->fd        = fd;
                        ring->front     = 0; 
                        ring->rear      = 0;
                        ring->ncount    = 0;
                        ring->rsize     = rsize;
                }
        }
        return ring; 
}
/**
 * @brief                       Destroys a ring buffer.
 * 
 * @param ring                  Ring.
 * @return struct r2_ring*      Returns NULL whenever ring is destroyed properly.
 */
struct r2_ring* r2_destroy_ring(struct r2_ring *ring)
{
        if(ring->fd != NULL){
                for(r2_uint64 i = ring->front, j = 0; j < ring->ncount; i = (i+1)%ring->rsize,  ++j)
                        ring->fd(ring->data[i]);          
        }

        free(ring->data);
        free(ring);
        return NULL;
}

/**
 * @brief                     Inserts an element into the ring buffer.  
 *                            Overwrites the oldest element if the ring becomes full.
 *           
 *                              
 * @param ring                Ring. 
 * @param data                Data.
 */
void r2_ring_insert(struct r2_ring *ring, void *data)
{
        assert(data != NULL);
        if(ring->ncount < ring->rsize){
                ring->data[ring->rear] = data;
                ring->rear = (ring->rear + 1) % ring->rsize;
                ++ring->ncount;
        }else{
                ring->data[ring->front] = data;
                ring->front = (ring->front  + 1) % ring->rsize;
                ring->rear = ring->front;
        }
}


/**
 * @brief                       Deletes an element from the ring buffer.
 * 
 * @param ring                  Ring.
 */
void r2_ring_delete(struct r2_ring *ring)
{
        if(r2_ring_empty(ring) != TRUE){
                if(ring->fd != NULL)
                        ring->fd(ring->data[ring->front]);
                ring->front = (ring->front + 1) % ring->rsize;
                --ring->ncount;
        }   
}

/**
 * @brief               Gets the first element in the ring buffer.
 * 
 * @param ring          The ring.
 * @return void*        Returns the first element in the ring buffer.
 */
void* r2_ring_front(const struct r2_ring *ring)
{
        return ring->data[ring->front];
}

/**
 * @brief               Returns the data at a position pos in the ring.
 *                      Indexing starts at zero.
 * 
 * @param pos           Position.         
 * @return void*        Returns the data at a posiiton else NULL.
 */
void* r2_ring_at(const struct r2_ring *ring, r2_uint64 pos)
{
        void *data = NULL; 
        
        if(pos < ring->ncount){
                pos = (ring->front + pos) % ring->rsize;
                data = ring->data[pos];
        }
        return data;
}


/**
 * @brief               Checks to see if the ring buffer is empty.
 * 
 * @param ring          Ring.
 * @return r2_uint16    Returns TRUE when empty, otherwise FALSE.
 */
r2_uint16 r2_ring_empty(const struct r2_ring *ring)
{
        return ring->ncount == 0;
}

/**
 * @brief                       Creates a copy of a ring.
 *                           
 *                              This function can do either a shallow or deep copy based on whether 
 *                              cpy was set. If cpy is set then it's a deep copy, else shallow copy. 
 *                              Fd should be set when cpy is set.
 * 
 * @param ring                  Ring.
 * @return struct r2_ring*      Returns ring.
 */
struct r2_ring* r2_ring_copy(const struct r2_ring *ring)
{
        struct r2_ring *copy = r2_create_ring(ring->rsize, ring->cmp, ring->cpy, ring->fd); 
        if(copy != NULL){
                copy->front  = ring->front; 
                copy->rear   = ring->rear;
                for(r2_uint64 i = 0, j = ring->front; i < ring->ncount; ++i, j = (j + 1) % ring->rsize){
                        if(ring->cpy != NULL){
                                copy->data[j] = ring->cpy(ring->data[j]);
                                if(copy->data[j] == NULL){
                                        copy = r2_destroy_ring(copy);
                                        break; 
                                }
                        }else
                                copy->data[j] = ring->data[j];
                        copy->ncount++;
                }
        }

        return copy; 
}

/**
 * @brief                       Compares two rings.
 *                              This function can do either a shallow or deep comparison based on whether 
 *                              cmp was set. If cmp is set for r1 then it's a deep comparison, else shallow comparison.    
 *                              
 * @param r1                    Ring 1
 * @param r2                    Ring 2
 * @return r2_uint16            Returns TRUE if both rings are equal, otherwise FALSE.
 */
r2_uint16 r2_ring_compare(const struct r2_ring *r1, const struct r2_ring *r2)
{               
                r2_uint16 result = FALSE;

                if(r2_ring_empty(r1) == TRUE && r2_ring_empty(r2) == TRUE)
                        result = TRUE;
                else if(r1->ncount == r2->ncount){
                        for(r2_uint64 i = 0, j = r1->front, k = r2->front; i < r1->ncount; ++i, j = (j + 1) % r1->rsize, k= (k + 1) % r2->rsize){
                                if(r1->cmp != NULL)
                                        result = r1->cmp(r1->data[j], r2->data[k]) == 0? TRUE : FALSE; 
                                else
                                        result = r1->data[j] == r2->data[k]? TRUE : FALSE;
                                
                                if(result == FALSE)
                                        break;
                        }
                }
        return result;
}