#include "r2_stack.h"
#include <stdlib.h>
#include <stdio.h>

/********************File scope functions************************/
static void r2_freenode(struct r2_stacknode *, r2_fd);
/********************File scope functions************************/



/**
 * @brief                This function creates an empty stack. 
 * 
 * @param cmp            A callback comparison function.
 * @param cpy            A callback copy function.
 * @param fd             A callback function that releases memory.
 * @return r2_stack*     Returns an empty stack, else NULL.
 */
struct r2_stack* r2_create_stack(r2_cmp cmp, r2_cpy cpy, r2_fd fd)
{
        struct r2_stack *stack = malloc(sizeof(struct r2_stack)); 
        if(stack != NULL){
                stack->ssize    = 0; 
                stack->top      = NULL; 
                stack->cmp      = cmp; 
                stack->fd       = fd; 
                stack->cpy      = cpy;
        }

        return stack; 
}

/**
 * @brief                    This function creates an empty node and returns it to the caller.
 * 
 * @return r2_stacknode*     Returns an empty node, else NULL. 
 */
struct r2_stacknode* r2_create_stacknode()
{
        struct r2_stacknode *node = malloc(sizeof(struct r2_stacknode));
        if(node != NULL){
                node->data = NULL;
                node->next = NULL;
        }

        return node; 
}

/**
 * @brief                       Destroys stack.
 *                              
 * @param stack                 Stack.                    
 * @return struct r2_stack*     Returns NULL when the stack is destroyed properly.
 */
struct r2_stack* r2_destroy_stack(struct r2_stack *stack)
{
        
        struct r2_stacknode *cur = NULL;
        struct r2_stacknode *top  = r2_stack_peek(stack);

        while(top != NULL){
                cur = top; 
                top  = top->next;
                r2_freenode(cur, stack->fd);
        }

        free(stack);
        return NULL; 
}

/**
 * @brief                       Pushes an element onto stack.
 * 
 * @param stack                 Stack.
 * @param data                  Data.
 * @return struct r2_stack*     Returns stack. 
 */
struct r2_stack* r2_stack_push(struct r2_stack *stack, void *data)
{
        struct r2_stacknode *node = r2_create_stacknode(); 
        if(node != NULL){
                node->data = data;
                node->next = stack->top;
                ++stack->ssize;
                stack->top = node; 
        }

        return stack;
}

/**
 * @brief                       Pops an element from stack.
 * 
 * @param stack                 Stack.
 * @return struct r2_stack*     Returns stack. 
 */
struct r2_stack* r2_stack_pop(struct r2_stack *stack)
{
        if(r2_stack_empty(stack) != TRUE){
                struct r2_stacknode *top = r2_stack_peek(stack); 
                stack->top = top->next;
                --stack->ssize;
                r2_freenode(top, stack->fd);
        }

        return stack; 
}


/**
 * @brief                       Gets the top of the stack.
 * 
 * @param stack                 Stack.
 * @return struct r2_stacknode* Returns the top of the stack.
 */
struct r2_stacknode* r2_stack_peek(const struct r2_stack *stack)
{
        return stack->top; 
}


/**
 * @brief                       Checks whether a stack is empty.
 * 
 * @param stack                 Stack.
 * @return                      Returns TRUE when the stack is empty, FALSE otherwise.
 */
r2_int16 r2_stack_empty(const struct r2_stack *stack)
{
    return stack->top == NULL && stack->ssize == 0;
}



/**
 * @brief                       Makes a copy of stack.
 * 
 * @param source                Stack. 
 * @return struct r2_stack*     Returns the copy of the stack.
 */
struct r2_stack* r2_stack_copy(const struct r2_stack *source)
{
        struct r2_stack *new_stack = r2_create_stack(source->cmp, source->cpy, source->fd);
        if(new_stack != NULL){
                struct r2_stacknode *top        = r2_stack_peek(source);
                struct r2_stacknode *temp       = NULL;
                struct r2_stacknode **next      = &new_stack->top;
                while(top != NULL){
                        temp = r2_create_stacknode();
                        if(temp != NULL){

                                temp->data =  source->cpy != NULL? source->cpy(top->data) : top->data;
                                *next      = temp; 
                                next       = &(temp->next);
                                ++new_stack->ssize;
                        }
                        top = top->next; 
                }
        }
        return new_stack; 
}

/**
 * @brief              Compares two stack.
 * 
 * @param s1           Stack 1.
 * @param s2           Stack 2.

 * @return r2_uint16   Returns TRUE if both stack are equal, else FALSE.
 */
r2_uint16  r2_stack_compare(const struct r2_stack *s1, const struct r2_stack *s2)
{
        r2_uint16 result = FALSE;
        if(s1->ssize != s2->ssize)
                return result; 
        
        

        if(r2_stack_empty(s1) != TRUE && r2_stack_empty(s2) != TRUE && s1->ssize == s2->ssize){
                
                struct r2_stacknode *s1_top = r2_stack_peek(s1); 
                struct r2_stacknode *s2_top = r2_stack_peek(s2);

                while(s1_top != NULL && s2_top != NULL){
                               
                        if(s1->cmp != NULL)
                                result = s1->cmp(s1_top->data, s2_top->data) == 0? TRUE: FALSE;
                        else
                                result = s1_top->data == s2_top->data? TRUE : FALSE;
                        if(!result)
                                break;
                        s1_top = s1_top->next; 
                        s2_top = s2_top->next;
                }
        }else
                if(r2_stack_empty(s1) == TRUE && r2_stack_empty(s2) == TRUE)
                        result = TRUE;
        
        return result;
}

/**
 * @brief Helper function to free a stack node. 
 *        
 *       
 *        If r2_freedata is NULL it doesn't free the data portion of node. 
 * 
 */
static void r2_freenode(struct r2_stacknode *node, r2_fd freedata)
{
        if(freedata != NULL)
                freedata(node->data); 
        free(node);
}