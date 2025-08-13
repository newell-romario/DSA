#include "r2_list.h"
#include <stdlib.h>
#include <assert.h>

/********************File scope functions************************/
static void r2_freenode(struct r2_listnode *, r2_fd);
struct r2_listnode* r2_create_listnode(); 
/********************File scope functions************************/

/**
 * @brief                       Creates a empty node. 
 * 
 * @return struct r2_listnode*  Returns a list node, else NULL.
 */
struct r2_listnode* r2_create_listnode()
{
        struct r2_listnode *node = malloc(sizeof(struct r2_listnode)); 
        if(node != NULL){
                node->data = NULL; 
                node->next = NULL; 
                node->prev = NULL; 
        }

        return node; 
}

/**
 * @brief                       Returns the node at position pos in the list.
 *                              Indexing begins at zero.
 * 
 * @param list                  List. 
 * @param pos                   Position. 
 * @return struct r2_listnode*  Returns node, else NULL.
 */
struct r2_listnode* r2_listnode_at(const struct r2_list *list, r2_uint64 pos)
{
        if(pos >= list->lsize)
                return NULL;

        struct r2_listnode *front = list->front; 
        for(r2_uint64 i = 0 ; i < pos && front != NULL; ++i, front = front->next); 
        
        return front; 
}

/**
 * @brief                               Returns the first node in the list. 
 * 
 * @param list                          List.
 * @return struct r2_listnode*          Returns the first node in the list. 
 */
struct r2_listnode* r2_listnode_first(const struct r2_list *list)
{
        return list->front; 
}

/**
 * @brief                       Returns the last node in the list.              
 * 
 * @param list                  List. 
 * @return struct r2_listnode*  Returns the last node in the list. 
 */
struct r2_listnode* r2_listnode_last(const struct r2_list *list)
{
        return list->rear;
}

/**
 * @brief                       Creates an empty list. 
 * 
 * @param  cmp                  A comparison callback function.
 * @param  cpy                  A callback function to copy values.
 * @param  fd                   A callback function that release memory.
 * @return struct r2_list*      Returns empty list, else NULL. 
 */
struct r2_list* r2_create_list(r2_cmp cmp, r2_cpy cpy, r2_fd fd)
{
        struct r2_list *list = malloc(sizeof(struct r2_list)); 
        if(list != NULL){
                list->front = NULL; 
                list->rear  = NULL;
                list->cmp   = cmp; 
                list->cpy   = cpy; 
                list->fd    = fd;     
                list->lsize  = 0;
        }

        return list; 
}

/**
 * @brief                     Destroys the list.  
 *                            
 * @param list                List. 
 * @return struct r2_list*    Returns NULL whenever the list is successfully destroyed.
 */
struct r2_list* r2_destroy_list(struct r2_list *list)
{
        struct r2_listnode *front = r2_listnode_first(list);
        struct r2_listnode *current  = front;
        while(current != NULL){
                front = current;
                current = current->next;
                r2_freenode(front, list->fd); 
                
        }

        free(list);
        return NULL;  
}


/**
 * @brief                       Inserts an element at the front of the list.                 
 * 
 * @param list                  List. 
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16 r2_list_insert_at_front(struct r2_list *list, void *data)
{
        assert(data != NULL);
        struct r2_listnode *node = r2_create_listnode(); 
        r2_uint16 SUCCESS = FALSE;
        if(node != NULL){
                node->data = data;
                
                if(r2_list_empty(list) == TRUE)
                        list->rear = node; 
                else
                        list->front->prev = node; 
                
                node->next = list->front; 
                list->front = node; 
                ++list->lsize;
                SUCCESS = TRUE;
        }

        return SUCCESS; 
}

/**
 * @brief                       Inserts an element at the end of the list.
 * 
 * @param list                  List. 
 * @param data                  Data. 
 * @return r2_uint16            Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16 r2_list_insert_at_back(struct r2_list *list, void *data)
{
        assert(data != NULL);
        struct r2_listnode *node = r2_create_listnode(); 
        r2_uint16 SUCCESS = FALSE;
        if(node != NULL){
                node->data = data;
                if(r2_list_empty(list) == TRUE)
                        list->front = node; 
                else
                        list->rear->next = node; 

                node->prev = list->rear;
                list->rear = node; 
                ++list->lsize; 
                SUCCESS = TRUE;
        }

        return SUCCESS; 
}

/**
 * @brief                               Inserts an element after position POS.
 * 
 * @param list                          List
 * @param pos                           Position. 
 * @param data                          Data.
 * @return r2_uint16                    Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16 r2_list_insert_after(struct r2_list *list, struct r2_listnode *pos, void *data)
{
        assert(data != NULL);
        struct r2_listnode *rear   = r2_listnode_last(list);
        r2_uint16 SUCCESS = FALSE;
        if(pos == rear)
                return r2_list_insert_at_back(list, data);
        else{
                struct r2_listnode *node = r2_create_listnode(); 
                if(node != NULL){
                        node->data = data;
                        node->prev = pos; 
                        node->next = pos->next; 
                        pos->next->prev = node; 
                        pos->next = node;
                        ++list->lsize; 
                        SUCCESS = TRUE;
                }
        }
        return SUCCESS; 
}

/**
 * @brief                       Inserts an element before position POS. 
 * 
 * @param list                  List.
 * @param pos                   Position.
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon succesful insertion, else FALSE.
 */
r2_uint16 r2_list_insert_before(struct r2_list *list, struct r2_listnode *pos, void *data)
{
        assert(data != NULL);
        struct r2_listnode *front = r2_listnode_first(list); 
        r2_uint16 SUCCESS  = FALSE;
        if(pos == front)
                return r2_list_insert_at_front(list, data);
        else{
                struct r2_listnode *node = r2_create_listnode();
                if(node != NULL){
                        node->data = data;
                        node->next = pos;
                        node->prev = pos->prev;
                        pos->prev->next = node; 
                        pos->prev = node; 
                        ++list->lsize; 
                        SUCCESS = TRUE;
                }
        }
        return SUCCESS; 
}


/**
 * @brief                       Deletes an element at the front of the list. 
 * 
 * @param list                  List.  
 * @return r2_uint16            Returns TRUE upon succesful deletion, else FALSE.
 */
r2_uint16 r2_list_delete_at_front(struct r2_list *list)
{

        r2_uint16 SUCCESS = FALSE;
        if(r2_list_empty(list) != TRUE){
                struct r2_listnode *front = r2_listnode_first(list); 
                if(list->front == list->rear)
                        list->rear = NULL;
                else
                        front->next->prev = NULL; 
                
                list->front = front->next;
                --list->lsize; 
                r2_freenode(front, list->fd);
                SUCCESS = TRUE;
        }

        return SUCCESS; 
}


/**
 * @brief                       Deletes an element at the back of the list.
 * 
 * @param list                  List. 
 * @return r2_uint16            Returns TRUE upon succesful deletion, else FALSE.
 */
r2_uint16 r2_list_delete_at_back(struct r2_list *list)
{
        r2_uint16 SUCCESS = FALSE;
        if(r2_list_empty(list) != TRUE){
                struct r2_listnode *rear = r2_listnode_last(list); 
                if(list->front == list->rear)
                        list->front = NULL;
                else
                        rear->prev->next = NULL;
                
                list->rear = rear->prev;
                --list->lsize;
                r2_freenode(rear, list->fd);
                SUCCESS = TRUE;
        }
        return SUCCESS; 
}

/**
 * @brief                       Removes an element at POS.
 * 
 * @param list                  List. 
 * @param pos                   Position.
 * @return r2_uint16            Returns TRUE upon succesful deletion, else FALSE.
 */
r2_uint16 r2_list_delete(struct r2_list *list, struct r2_listnode *pos)
{
        r2_uint16 SUCCESS = FALSE;
        if(r2_list_empty(list) != TRUE){  
                struct r2_listnode *front = r2_listnode_first(list); 
                struct r2_listnode *rear  = r2_listnode_last(list);
                if(pos == front)
                        return r2_list_delete_at_front(list); 
                else if(pos == rear)
                        return r2_list_delete_at_back(list); 
                else{
                        pos->prev->next = pos->next; 
                        pos->next->prev = pos->prev;
                        r2_freenode(pos, list->fd);       
                        --list->lsize;
                }
                SUCCESS = TRUE;
        }
        return SUCCESS; 
}

/**
 * @brief                       Creates a copy of the list.
 *                              
 *                              This function can do either a shallow or deep copy based on whether 
 *                              cpy was set. If cpy is set for source then it's a deep copy, else shallow copy.
 *                              Fd should be set when cpy is set.
 *                      
 * @param source                Source.
 * @return struct r2_list*      Returns a copy, else NULL.
 */
struct r2_list* r2_list_copy(const struct r2_list *source)
{      
        struct r2_list *copy = r2_create_list(source->cmp, source->cpy, source->fd); 
        if(copy != NULL){
                struct r2_listnode *current = r2_listnode_first(source);
                struct r2_listnode *temp    = NULL;
                struct r2_listnode *prev    = NULL;
                struct r2_listnode **front  = &copy->front;
                
                while(current != NULL){
                        temp = r2_create_listnode(); 
                        if(temp != NULL){
                                if(current->data != NULL && source->cpy != NULL){
                                        temp->data =  source->cpy(current->data);
                                        if(temp->data == NULL){
                                                copy = r2_destroy_list(copy);
                                                break;
                                        }
                                }else 
                                        temp->data =  current->data;
                                temp->prev = prev;
                                *front     = temp; 
                                front      = &temp->next;
                                prev       = temp; 
                                copy->rear = temp;
                                ++copy->lsize; 
                        }
                        current = current->next;
                } 
        }
        return copy; 
}

/**
 * @brief                       Compare lists. 
 *
 *                              This function can do either a shallow or deep comparison based on whether 
 *                              cmp was set. If cmp is set for l1 then it's a deep comparison, else shallow comparison. 
 *                              
 * @param l1                    List 1
 * @param l2                    List 2
 * @return r2_uint16            Returns TRUE if lists are equal, otherwise FALSE.
 */
r2_uint16  r2_list_compare(const struct r2_list *l1, const struct r2_list *l2)
{
        r2_uint16 result = FALSE; 
        if(r2_list_empty(l1) == TRUE && r2_list_empty(l2) == TRUE){
                result = TRUE;
                return result;
        }
                
        if(l1->lsize == l2->lsize){
                struct r2_listnode *l1_front = r2_listnode_first(l1); 
                struct r2_listnode *l2_front = r2_listnode_first(l2);
                while(l1_front != NULL && l2_front != NULL){
                        
                        if(l1->cmp != NULL)
                                result = l1->cmp(l1_front->data, l2_front->data) == 0? TRUE: FALSE;
                        else
                                result = l1_front->data == l2_front->data? TRUE : FALSE;
                        
                        if(result == FALSE)
                                break;

                        l1_front = l1_front->next; 
                        l2_front = l2_front->next;
                }
        }

        return result;
}

/**
 * @brief               Checks whether the list is empty.
 * 
 * @param list          The list that will be checked.
 * @return r2_uint16    Returns TRUE when the list is empty, otherwise FALSE.
 */
r2_uint16 r2_list_empty(const struct r2_list *list)
{
        return list->lsize == 0 && list->front == NULL && list->rear == NULL;
}

/**
 * @brief Helper function to free a stack node. 
 *              
 *        If r2_fd is NULL it doesn't free the data portion of node. 
 * 
 */
static void r2_freenode(struct r2_listnode *node, r2_fd freedata)
{
        if(freedata != NULL)
                if(node != NULL)
                        freedata(node->data); 
        free(node);
}