#include "r2_arrstack.h"
#include <stdlib.h>
#include <assert.h>
#define DEFAULT_SIZE  2 
/********************File scope functions************************/
static r2_uint16 r2_arrstack_resize(struct r2_arrstack *, r2_uint64);
/********************File scope functions************************/

/**
 * @brief                       Creates an empty stack. 
 * 
 * @param size                  Size.
 * @param fd                    A callback function that frees each item on the stack.
 * @param cpy                   A callback function that copies each item on the stack.
 * @param cmp                   A callback comparison function compares item on the stack.
 * @return struct r2_arrstack*  Returns an empty stack, otherwise NULL.
 */
struct r2_arrstack* r2_arrstack_create_stack(r2_uint64 size, r2_fd fd, r2_cpy cpy, r2_cmp cmp)
{
        struct r2_arrstack *stack  = malloc(sizeof(struct r2_arrstack));
        if(stack != NULL){
                size        = size > DEFAULT_SIZE? size : DEFAULT_SIZE; 
                stack->data = malloc(sizeof(void *) * size);
                if(stack->data != NULL){
                        stack->top              = 0; 
                        stack->ncount           = 0; 
                        stack->ssize            = size;
                        stack->fd               = fd;
                        stack->cpy              = cpy;
                        stack->cmp              = cmp;
                }else{
                        free(stack); 
                        stack  = NULL;
                }
        }

        return stack;
}

/**
 * @brief                       Destroys stack.
 * 
 * @param stack                 Stack.
 * @return struct r2_arrstack*  Returns NULL upon successfully destruction.
 */
struct r2_arrstack*  r2_arrstack_destroy_stack(struct r2_arrstack *stack)
{
        if(stack->fd != NULL){
                for(r2_int64 i = 0; i < stack->ncount; ++i)
                       stack->fd(stack->data[i]);
        }
        
        free(stack->data);
        free(stack); 
        return NULL;         
}

/**
 * @brief                       Pushes an element onto stack.
 * 
 * @param stack                 Stack. 
 * @param data                  Data. 
 * @return struct r2_arrstack*  Returns stack. 
 */
struct r2_arrstack* r2_arrstack_push(struct r2_arrstack *stack, void *data)
{
        if(r2_arrstack_full(stack) == TRUE){
                r2_uint64 size = stack->ssize * 2;      
                /*Resize failed*/
                if(r2_arrstack_resize(stack, size) != TRUE)
                        goto FINAL;

        }

        stack->data[stack->top] = data; 
        ++stack->top; 
        ++stack->ncount;
        FINAL:
                return stack;
}

/**
 * @brief                       Pops an element from stack. 
 * 
 * @param stack                 Stack. 
 * @return struct r2_arrstack*  Returns stack. 
 */
struct r2_arrstack* r2_arrstack_pop(struct r2_arrstack *stack)
{
        if(r2_arrstack_empty(stack) != TRUE){
                --stack->top;
                if(stack->fd != NULL)
                        stack->fd(stack->data[stack->top]);

                stack->data[stack->top] = NULL;
                --stack->ncount;
                if(stack->ncount > 0 && stack->ncount <= (stack->ssize / 4)){
                        r2_int64 size = stack->ssize / 2; 
                        r2_arrstack_resize(stack, size); 
                }
        }
        return stack;
}

/**
 * @brief               Returns the top of the stack.
 *                       
 *                      A NULL return value can mean the stack is empty or 
 *                      a NULL value is actually at the top of the stack.
 * 
 * @param stack         Stack.
 * @return void*        Returns the data, else NULL.
 */
void* r2_arrstack_top(const struct r2_arrstack *stack)
{
        return r2_arrstack_empty(stack) != TRUE? stack->data[stack->top - 1] : NULL;
}


/**
 * @brief               Checks whether the stack is empty.
 * 
 * @param stack         Stack. 
 * @return r2_uint16    Returns TRUE if stack empty, otherwise FALSE.
 */
r2_uint16 r2_arrstack_empty(const struct r2_arrstack *stack)
{
        return stack->ncount == 0;
}

/**
 * @brief               Checks if the stack is full. 
 * 
 * @param stack         Stack. 
 * @return r2_uint16    Returns TRUE if stack is full, otherwise FALSE.
 */
r2_uint16 r2_arrstack_full(const struct r2_arrstack*stack)
{
        return stack->ssize == stack->ncount;
}
/**
 * @brief                       Grows or shrinks stack.                        
 * 
 * @param  stack                Stack. 
 * @return r2_uint16            Returns TRUE if stack was successfully grown, else FALSE. 
 */
static r2_uint16 r2_arrstack_resize(struct r2_arrstack *stack, r2_uint64 size)
{
        r2_uint16 SUCCESS = FALSE;
        size        = size > DEFAULT_SIZE? size : DEFAULT_SIZE; 
        void **data = malloc(sizeof(void *) * size);
        if(data != NULL){
                /* Copies data items from the old stack to the new stack.*/
                for(int i = 0; i < stack->ncount; ++i)
                        data[i] = stack->data[i]; 

                free(stack->data); 
                stack->data = data;
                stack->ssize = size;
                SUCCESS= TRUE;
        }                
        return SUCCESS;    
}


/**
 * @brief               Compares two stack.
 * 
 * @param s1            Stack 1
 * @param s2            Stack 2
 * @return r2_uint16    Returns TRUE if both stack are equal, otherwise FALSE.
 */
r2_uint16 r2_arrstack_compare(const struct r2_arrstack *s1, const struct r2_arrstack *s2)
{
        r2_uint16 result = FALSE; 
        if(s1->ncount == s2->ncount){
                if(r2_arrstack_empty(s1) != TRUE && r2_arrstack_empty(s2) != TRUE){
                        for(int i = 0; i < s1->ncount; ++i){
                                if(s1->cmp != NULL)
                                        result = s1->cmp(s1->data[i], s2->data[i]);
                                else    result = s1->data[i] == s2->data[i]? TRUE : FALSE; 

                                if(result == FALSE)
                                        break;
                        }
                }else
                        result = TRUE;
        }

        return result;
}

/**
 * @brief                       Retuns a copy of the stack. 
 * 
 * @param source                Stack.
 * @return struct r2_arrstack*  Returns a copy of the stack. 
 */
struct r2_arrstack* r2_arrstack_copy(const struct r2_arrstack *source)
{
        struct r2_arrstack *dest = r2_arrstack_create_stack(source->ssize, source->fd, source->cpy, source->cmp);
        if(dest != NULL){
                for(int i = 0; i < source->ncount; ++i){
                        if(source->cpy != NULL)
                                dest->data[i] = source->cpy(source->data[i]);
                        else
                                dest->data[i] = source->data[i];

                }
                dest->top    = source->top;
                dest->ncount = source->ncount;
        }
        return dest;
}