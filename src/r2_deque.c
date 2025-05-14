#include "r2_deque.h"
#include <stdlib.h>
#include <assert.h>
/********************File scope functions************************/
static void r2_freenode(struct r2_dequenode *, r2_fd);
static struct r2_dequenode*    r2_create_dequenode();
/********************File scope functions************************/


/**
 * @brief                      Creates an empty deque.
 * 
 * @param cmp                  A comparison callback function. 
 * @param cpy                  A callback function to copy values.
 * @param fd                   A callback function that release memory.  
 * @return  struct r2_deque    Returns an empty deque, else NULL.
 */
struct r2_deque*  r2_create_deque(r2_cmp cmp, r2_cpy cpy, r2_fd fd)
{
        struct r2_deque *deque = malloc(sizeof(struct r2_deque)); 
        if(deque != NULL){
                deque->front = NULL;
                deque->rear  = NULL;
                deque->cmp   = cmp;
                deque->cpy   = cpy; 
                deque->fd    = fd;
                deque->dsize = 0;
        }

        return deque;
}


/**
 * @brief                           Creates an empty node.
 * 
 * @return struct r2_dequenode*     Returns empty node, else NULL.
 */
static struct r2_dequenode* r2_create_dequenode()
{
        struct r2_dequenode *node = malloc(sizeof(struct r2_dequenode));
        if(node != NULL){
                node->data = NULL;
                node->next = NULL;
        }

        return node; 
}

/**
 * @brief                       Destroys the deque.

 * @param deque                 Deque.
 * @return struct r2_deque*     Returns NULL when the deque is destroyed properly. 
 */
struct r2_deque*  r2_destroy_deque(struct r2_deque *deque)
{
        struct r2_dequenode *front = deque->front;
        struct r2_dequenode *prev  = NULL;
        while(front != NULL){
                prev   = front;
                front  = front->next;
                r2_freenode(prev, deque->fd);
        }

        free(deque); 
        return NULL;
}


/**
 * @brief               Checks if the deque is empty. 
 * 
 * @param  deque        Deque.
 * @return r2_uint16    Returns TRUE if the deque is empty,  otherwise FALSE.
 */
r2_uint16 r2_deque_empty(const struct r2_deque *deque)
{
        return deque->front == NULL && deque->rear == NULL && deque->dsize == 0;
}


/**
 * @brief                       Inserts an element at the front of the deque.
 * 
 * @param deque                 Deque. 
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE. 
 */
r2_uint16 r2_deque_insert_at_front(struct r2_deque *deque, void *data)
{
        assert(data != NULL);
        struct r2_dequenode *node =  r2_create_dequenode();
        r2_uint16 SUCCESS = FALSE;
        if(node != NULL){
                node->data = data;
                if(r2_deque_empty(deque) == TRUE)
                        deque->rear = node;
                
                node->next   = deque->front;
                deque->front = node; 
                ++deque->dsize;
                SUCCESS = TRUE;
        }
        return SUCCESS;
}

/**
 * @brief                       Inserts an element at the back of the deque. 
 * 
 * @param deque                 Deque. 
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE.  
 */
r2_uint16 r2_deque_insert_at_back(struct r2_deque *deque, void *data)
{
        assert(data != NULL);
        struct r2_dequenode *node = r2_create_dequenode(); 
        r2_uint16 SUCCESS = FALSE;
        if(node != NULL){
                node->data = data;
                if(r2_deque_empty(deque) == TRUE)
                        deque->front = node;
                else
                        deque->rear->next = node;
                deque->rear = node;
                ++deque->dsize; 
                SUCCESS = TRUE;
        }
        return SUCCESS;
}


/**
 * @brief                               Removes the last node in the deque.
 * 
 * @param deque                         Deque.
 * @return r2_uint16                    Returns TRUE upon successful deletion, else FALSE.            
 */
r2_uint16 r2_deque_delete_at_back(struct r2_deque *deque)
{
        r2_uint16 SUCCESS = FALSE;
        if(r2_deque_empty(deque) != TRUE){
                struct r2_dequenode *front   = r2_deque_front(deque);
                struct r2_dequenode *cur     = r2_deque_rear(deque);
                
                if(front != cur){
                        /*Iterating through deque to find the node before rear*/
                        while(front->next != NULL){
                                cur = front; 
                                front = front->next; 
                        }
                        cur->next  = NULL;
                        deque->rear = cur;
                        cur = front; 
                }else{
                        deque->front = NULL; 
                        deque->rear  = NULL;
                }
                
                r2_freenode(cur, deque->fd); 
                --deque->dsize;
                SUCCESS = TRUE;
        }
        
        return SUCCESS; 
}

/**
 * @brief                               Removes the first node in the deque.
 * 
 * @param deque                         Deque.
 * @return r2_uint16                    Returns TRUE upon successful deletion, else FALSE. 
 */
r2_uint16  r2_deque_delete_at_front(struct r2_deque *deque)
{
        r2_uint16 SUCCESS  = FALSE;
        if(r2_deque_empty(deque) != TRUE){
                struct r2_dequenode *front = r2_deque_front(deque); 
                struct r2_dequenode *rear  = r2_deque_rear(deque);
                
                if(front == rear)
                        deque->rear  = NULL; 
                
                deque->front = front->next;
                r2_freenode(front, deque->fd); 
                --deque->dsize;
                SUCCESS = TRUE;
        }     
        return SUCCESS;
}

/**
 * @brief                               Returns the first node in the deque.
 * 
 * @param deque                         Deque. 
 * @return struct r2_dequenode*         Returns node.
 */
struct r2_dequenode* r2_deque_front(const struct r2_deque *deque)
{
        return deque->front;
}


/**
 * @brief                               Returns the last node in the deque.
 * 
 * @param deque                         Deque.
 * @return struct r2_dequenode*         Returns node. 
 */
struct r2_dequenode* r2_deque_rear(const struct r2_deque *deque)
{
        return deque->rear;
}

/**
 * @brief                               Creates a copy of the deque.
 *                                      
 *                                      This function can do either a shallow or deep copy based on whether 
 *                                      cpy was set. If cpy is set for source then it's a deep copy, else shallow copy. 
 *                                      Fd should be set when cpy is set.
 *  
 * @param source                        Deque.                                
 * @return struct r2_deque*             Returns copy.
 */
struct r2_deque* r2_deque_copy(const struct r2_deque *source)
{       
        struct r2_deque *dest = r2_create_deque(source->cmp, source->cpy, source->fd); 
        if(dest != NULL){
                struct r2_dequenode *temp  = NULL; 
                struct r2_dequenode *front = source->front;
                struct r2_dequenode **cur  = &dest->front; 
                while(front != NULL){
                        temp = r2_create_dequenode(); 
                        if(temp != NULL){
                                if(source->cpy != NULL && front->data != NULL){
                                        temp->data = source->cpy(front->data);
                                        if(temp->data == NULL){
                                                dest = r2_destroy_deque(dest);
                                                break;
                                        }
                                }     
                                else
                                        temp->data = front->data;

                                *cur = temp; 
                                cur = &temp->next;
                                dest->rear = temp; 
                                ++dest->dsize;
                        }else{
                                dest = r2_destroy_deque(dest);
                                break;
                        }
                        front = front->next;
                }
        
        }
        return dest;
}

/**
 * @brief                       Compares two deque.
 *                              
 *                              This function can do either a shallow or deep comparison based on whether 
 *                              cmp was set. If cmp is set for d1 then it's a deep comparison, else shallow comparison. 
 * 
 * @param d1                    Deque 1
 * @param d2                    Deque 2
 * @return  r2_uint16           Returns TRUE if both deque are equal, otherwise FALSE.
 */
r2_uint16  r2_deque_compare(const struct r2_deque *d1, const struct r2_deque *d2)
{
        r2_uint16 result = FALSE;
        if(r2_deque_empty(d1) == TRUE && r2_deque_empty(d2) == TRUE){
                result = TRUE;
                return TRUE;
        }

        if(d1->dsize == d2->dsize){
                struct r2_dequenode *d1_front = d1->front; 
                struct r2_dequenode *d2_front = d2->front;
                while(d1_front != NULL && d2_front != NULL){

                        if(d1->cmp != NULL)
                                result = d1->cmp(d1_front->data, d2_front->data) == 0? TRUE: FALSE;
                        else    
                                result = d1_front->data == d2_front->data? TRUE : FALSE;
                        
                        if(result == FALSE)
                                break;
                        d1_front = d1_front->next; 
                        d2_front = d2_front->next;
                } 

        }
                
        
        return result; 
}

/**
 * @brief Helper function to free a deque node. 
 *        If r2_freedata is NULL it doesn't free the data portion of node. 
 */
static void r2_freenode(struct r2_dequenode *node, r2_fd freedata)
{
        if(freedata != NULL)
                freedata(node->data); 
        free(node);
}