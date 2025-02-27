#include "r2_stack_test.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define SIZE 100

static int arr[SIZE]; 

/**
 * @brief Initializes the test data which will be used in our tests. 
 * 
 */
static void test_init_data()
{
        for(int i = 0; i < SIZE; ++i)
                arr[i] = rand() % 100;
}

/**
 * @brief Tests the correctness of the create function for a stack.
 * 
 */
static void test_r2_create_stack(){
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);
        assert(stack->cmp == NULL); 
        assert(stack->cpy == NULL); 
        assert(stack->fd  == NULL);
        assert(r2_stack_empty(stack) == TRUE);
        r2_destroy_stack(stack);
}

/**
 * @brief Tests the correctness of create function for a node.
 * 
 */
static void test_r2_create_stacknode()
{
        struct r2_stacknode *node = r2_create_stacknode();
        assert(node != NULL);
        assert(node->data == NULL);
        assert(node->next == NULL);
        free(node);
}

/**
 * @brief Tests the correctness of the destroy function for the stack.
 * 
 */
static void test_r2_destroy_stack()
{
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);
        assert(r2_destroy_stack(stack) == NULL); 
}

/**
 * @brief Tests the push functionality of the stack.
 * 
 */
static void test_r2_stack_push()
{
        struct r2_stack     *stack = r2_create_stack(NULL, NULL, NULL);
        struct r2_stacknode *top   = NULL;

        for(int i = 0; i < SIZE; ++i){
                
                 r2_stack_push(stack, &arr[i]);
                top   = r2_stack_peek(stack);  
                assert(top->data == &arr[i]);
        }

        
        assert(stack->ssize == SIZE);
        r2_destroy_stack(stack); 
}

/**
 * @brief Tests the pop functionality of the stack.
 * 
 *
 */
static void test_r2_stack_pop()
{
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);

        for(int i = 0; i < SIZE; ++i)
                r2_stack_push(stack, &arr[i]);
        
        struct r2_stacknode *top = NULL;
        for(int i = SIZE; i >= 2; i-= 2){

                
                top = r2_stack_peek(stack);
                assert(top->data == &arr[i - 1]);
                r2_stack_pop(stack);
                top = r2_stack_peek(stack);
                assert(top->data == &arr[i - 2]);
                r2_stack_pop(stack);

        }

        assert(r2_stack_empty(stack) == TRUE);
        r2_destroy_stack(stack); 
}

/**
 * @brief Tests the correctness of the peek function the for the stack.
 * 
 */
static void test_r2_stack_peek()
{
        struct r2_stack     *stack = r2_create_stack(NULL, NULL, NULL);
        struct r2_stacknode *top   = NULL; 

        for(int i = 0; i < SIZE; ++i){
                r2_stack_push(stack, &arr[i]);
                top   = r2_stack_peek(stack);
                assert(top->data == &arr[i]);
        }

        r2_destroy_stack(stack); 
}


static void* cpy(const void *data)
{
        int *copy = malloc(sizeof(int));
        *copy = *(int *)data;
        return copy;  
}


static r2_int16 cmp(const void *s1, const void *s2)
{
        if((*(int *)s1) == (*(int *)s2))
                return 0; 
        else if((*(int *)s1) < (*(int *)s2)) 
                return -1;
        else 
                return 1;
}

/**
 * @brief Tests the deep copy functionality of a stack.
 * 
 * 
 */
static void test_r2_stack_copy(){
        
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);

        /**
         *  Shallow copy test on epmpty stack.
         * 
         */
        struct r2_stack *copy  = r2_stack_copy(stack);
        assert(r2_stack_compare(stack, copy) == TRUE);
        assert(r2_stack_compare(stack, copy) == TRUE);
        r2_destroy_stack(copy); 


        /**
         *  Deep copy test on empty stack.
         */
        stack->cpy = cpy;
        copy  = r2_stack_copy(stack);
        
        /*Shallow comparison*/
        assert(r2_stack_compare(stack, copy) == TRUE);
        
        /*Deep comparison*/
        stack->cmp = cmp;
        assert(r2_stack_compare(stack, copy) == TRUE);
        r2_destroy_stack(copy); 


        for(int i = 0; i < SIZE;++i)
                r2_stack_push(stack, &arr[i]);
        

        /**
         * Shallow copy test on stack.
         */
        stack->cpy = NULL;
        stack->cmp = NULL;
        copy  = r2_stack_copy(stack);

         /*Shallow comparison*/
        assert(r2_stack_compare(stack, copy) == TRUE);

        /*Deep comparison*/
        stack->cmp = cmp;
        copy->cmp  = cmp;
        assert(r2_stack_compare(stack, copy) == TRUE);
        r2_destroy_stack(copy); 

        /**
         * @brief Deep copy test on stack
         * 
         */
        stack->cpy = cpy;
        copy  = r2_stack_copy(stack);
        /*Deep comparison*/
        assert(r2_stack_compare(stack, copy) == TRUE);

        /*Shallow comparison*/
        stack->cmp = NULL;
        stack->cmp = NULL;
        assert(r2_stack_compare(stack, copy) != TRUE);

        r2_destroy_stack(stack); 
        r2_destroy_stack(copy); 
}

/**
 * @brief       Tests the empty functionality of the stack.
 * 
 */
static void test_r2_stack_empty()
{               
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);
        assert(r2_stack_empty(stack) == TRUE);
        for(int i = 0; i < SIZE;++i)
                r2_stack_push(stack, &arr[i]);
        

        assert(r2_stack_empty(stack) != TRUE);
        r2_destroy_stack(stack);
}



/**
 * @brief Test comparing two stacks. 
 * 
 */
void static test_r2_stack_compare(){
        
        struct r2_stack *stack = r2_create_stack(NULL, NULL, NULL);
        
        /**
         * @brief Empty stack comparison which should be automatically true.
         * 
         */
        assert(r2_stack_compare(stack, stack) == TRUE);
        assert(r2_stack_compare(stack, stack) == TRUE);

        for(int i = 0; i < SIZE;++i)
                r2_stack_push(stack, &arr[i]);
        

        /**
         * @brief Compares a stack against itself which should be true.
         *        
         * 
         */
        stack->cmp = cmp; 
        stack->cpy = cpy;
        assert(r2_stack_compare(stack, stack) == TRUE);
        assert(r2_stack_compare(stack, stack) == TRUE);
        
        /**
         * @brief Does a comparion after a deep copy. 
         * 
         */
        struct r2_stack *copy  = r2_stack_copy(stack);
        /*Deep comparison*/
        assert(r2_stack_compare(stack, copy) == TRUE);
        /*Shallow comparison*/
        stack->cmp = NULL; 
        assert(r2_stack_compare(stack, copy) != TRUE);

        r2_destroy_stack(stack); 
        r2_destroy_stack(copy); 
}

/**
 * @brief Runs all test functions relating to the stack data structure. 
 * 
 */
void test_r2_stack_run()
{
        test_init_data();
        test_r2_create_stack();
        test_r2_create_stacknode();
        test_r2_destroy_stack();
        test_r2_stack_push();
        test_r2_stack_peek();
        test_r2_stack_pop();
        test_r2_stack_copy();  
        test_r2_stack_empty();
        test_r2_stack_compare();
}