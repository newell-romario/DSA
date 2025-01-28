#include "r2_queue.h"
#include <stdlib.h>

/********************File scope functions************************/
static void r2_freenode(struct r2_queuenode *, r2_fd);
/********************File scope functions************************/

/**
 * @brief              Returns an empty queue.
 * 
 * @param  cmp         A comparison callback function.
 * @param  cpy         A callback function to copy values.
 * @param  fd          A callback function that release memory.
 * @return r2_queue*   Returns an empty queue, else NULL.
 */
struct r2_queue*  r2_create_queue(r2_cmp cmp, r2_cpy cpy, r2_fd fd)
{
        struct r2_queue *queue = malloc(sizeof(struct r2_queue));
        if(queue != NULL){
                queue->front = NULL;
                queue->rear  = NULL;
                queue->cmp   = cmp; 
                queue->cpy   = cpy;
                queue->fd    = fd;
                queue->qsize  = 0;
        }
        return queue;
}

/**
 * @brief                    Returns an emtpy node.
 * 
 * @return r2_queuenode*     Returns an emtpy node, else NULL.
 */
struct r2_queuenode* r2_create_queuenode()
{
        struct r2_queuenode *node = malloc(sizeof(struct r2_queuenode));
        if(node != NULL)
        {
                node->data = NULL;
                node->next = NULL;
        }
        return node;
}

/**
 * @brief                       Destroys queue.
 *                             
 * @param queue                 Queue. 
 * @return struct r2_queue*     Returns NULL whenever the queue is successfully destroyed.
 */
struct r2_queue* r2_destroy_queue(struct r2_queue *queue)
{
        struct r2_queuenode *front = r2_queue_front(queue);
        struct r2_queuenode *prev  = NULL; 
        while(front != NULL){
                prev  = front; 
                front = front->next; 
                r2_freenode(prev, queue->fd);    
        }

        free(queue); 
        return NULL;
}

/**
 * @brief                       Enqueues an element.
 *                              
 * @param queue                 Queue.
 * @param data                  Data stored. 
 * @return r2_uint16            Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16 r2_queue_enqueue(struct r2_queue*queue, void *data)
{
        struct r2_queuenode *node = r2_create_queuenode(); 
        r2_uint16 SUCCESS = FALSE;
        if(node != NULL){
                node->data = data; 
                if(r2_queue_empty(queue) == TRUE)
                        queue->front = node;
                else
                        queue->rear->next = node;

                queue->rear = node;
                ++queue->qsize;
                SUCCESS = TRUE;
        }

        return SUCCESS;
}

/**
 * @brief                       Dequeues an element.
 *                              
 * @param queue                 Queue.
 * @return r2_uint16            Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16  r2_queue_dequeue(struct r2_queue *queue)
{
        r2_uint16 SUCCESS = FALSE;
        if(r2_queue_empty(queue) != TRUE){
                struct r2_queuenode *front = r2_queue_front(queue); 
                if(queue->front == queue->rear)
                        queue->rear = NULL;
                
                queue->front = front->next;
                --queue->qsize;
                r2_freenode(front, queue->fd);
                SUCCESS = TRUE;
        }
        return SUCCESS; 
}

/**
 * @brief                       Returns the front of the queue.
 * 
 * @param queue                 Queue.
 * @return struct r2_queuenode* Returns the front of the queue.
 */
struct r2_queuenode* r2_queue_front(const struct r2_queue *queue)
{
        return queue->front; 
}

/**
 * @brief                       Returns the rear of the queue.
 * 
 * @param queue                 Queue.
 * @return struct r2_queuenode* Returns the rear of the queue.
 */
struct r2_queuenode* r2_queue_rear(const struct r2_queue *queue)
{
        return queue->rear;
}
/**
 * @brief                       Checks whether the queue is empty.
 * 
 * @param queue                 Queue. 
 * @return r2_uint16            Returns TRUE for an empty queue, otherwise FALSE. 
 */
r2_uint16 r2_queue_empty(const struct r2_queue *queue)
{
        return queue->front == NULL && queue->rear == NULL && queue->qsize == 0;
}



/**
 * @brief                      Makes a copy of the queue. 
 * 
 * @param source               Source.
 * @return struct r2_queue*    Returns the copy of the queue.
 */
struct r2_queue* r2_queue_copy(const struct r2_queue *source)
{
        struct r2_queue *dest = r2_create_queue(source->cmp, source->cpy, source->fd);
        if(dest != NULL){
                if(r2_queue_empty(source) != TRUE){
                        struct r2_queuenode *front = r2_queue_front(source);
                        struct r2_queuenode **cur  = &dest->front;
                        struct r2_queuenode *temp  = NULL;
                        while(front != NULL){
                                temp = r2_create_queuenode();
                                if(temp != NULL){
                                        if(front->data != NULL && source->cpy != NULL){
                                                temp->data = source->cpy(front->data);
                                                if(temp->data == NULL){
                                                        dest = r2_destroy_queue(dest); 
                                                        break;
                                                }
                                        }else   temp->data = front->data;
                                        *cur = temp;
                                        cur  = &temp->next;
                                        ++dest->qsize;
                                        dest->rear = temp;
                                }else{
                                        dest = r2_destroy_queue(dest); 
                                        break;
                                }
                                front = front->next;
                        }  
                }
        }
        return dest;
}

/**
 * @brief                       Compares two queues.
 * 
 * @param q1                    Queue 1.
 * @param q2                    Queue 2.
 * @return r2_uint16            Returns TRUE or FALSE based on equality.
 */
r2_uint16  r2_queue_compare(const struct r2_queue *q1, const struct r2_queue *q2)
{
        r2_uint16 result = FALSE; 
        if(r2_queue_empty(q1) != TRUE && r2_queue_empty(q2) != TRUE && q2->qsize == q1->qsize){
                struct r2_queuenode *q1_front = r2_queue_front(q1); 
                struct r2_queuenode *q2_front = r2_queue_front(q2);
                while(q1_front != NULL && q2_front != NULL){
                        
                        if(q1->cmp != NULL)
                                result = q1->cmp(q1_front->data, q2_front->data) == 0? TRUE : FALSE;
                        else
                                result = q1_front->data == q2_front->data? TRUE : FALSE;
                        
                        if(result == FALSE)
                                break;
                        
                        q1_front = q1_front->next; 
                        q2_front = q2_front->next; 
                }
        }else if(r2_queue_empty(q1) == TRUE && r2_queue_empty(q2) == TRUE)
                result = TRUE;
        
        return result;
}

/**
 * @brief Helper function to free a stack node. 
 *        
 *        If r2_fd is NULL it doesn't free the data portion of node. 
 * 
 */
static void r2_freenode(struct r2_queuenode *node, r2_fd freedata)
{
        if(freedata != NULL)
                freedata(node->data); 

        free(node);
}