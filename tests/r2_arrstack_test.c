#include <stdlib.h>
#include <assert.h>
#include "r2_arrstack_test.h"
#define SIZE 4096
static int arr[SIZE]; 

/**
 * @brief       Initializes the test data. 
 * 
 */
static void test_r2_arrstack_init_data()
{
        for(r2_uint64 i = 0; i < SIZE; ++i)
                arr[i]  = rand() % (SIZE * 2) + 1; 
}


/**
 * @brief      Tests the create functionality of the stack.
 * 
 */
static void test_r2_create_arrstack()
{
        struct r2_arrstack *stack = NULL; 
        /**
         * @brief Creates stack of different sizes.
         * 
         */
        for(int i = 32; i < 4096; i <<= 2){
                stack = r2_create_arrstack(i, NULL, NULL, NULL); 
                assert(r2_arrstack_empty(stack) == TRUE); 
                assert(stack->top == 0);
                assert(stack->cmp == NULL); 
                assert(stack->cpy == NULL); 
                assert(stack->fd  ==  NULL);
                assert(stack->ssize == i);
                assert(stack->data != NULL);
                r2_destroy_arrstack(stack);
        }
}

/**
 * @brief       Test the destroy functionality of the stack.
 * 
 */
static void test_r2_destroy_arrstack()
{
        struct r2_arrstack  *stack = r2_create_arrstack(SIZE, NULL, NULL, NULL); 
        assert(r2_destroy_arrstack(stack) == NULL);        
}

/**
 * @brief Test push functionality of the stack.
 * 
 */
static void test_r2_arrstack_push()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL); 
        for(int i = 0; i < SIZE; ++i){
                r2_arrstack_push(stack, &arr[i]);
                assert(r2_arrstack_top(stack) == &arr[i]); 
                assert(stack->ncount == (i + 1));
        }
        r2_destroy_arrstack(stack);
}

/**
 * @brief       Test the pop functionality of the stack.
 * 
 */
static void test_r2_arrstack_pop()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                r2_arrstack_push(stack, &arr[i]);
        
        void *data = NULL;
        for(int i = 1; i < SIZE; i += 2){
                data  = r2_arrstack_top(stack);
                assert(&arr[SIZE - i] == data);
                r2_arrstack_pop(stack);
                data  = r2_arrstack_top(stack);
                assert(&arr[SIZE - i - 1] == data);
                r2_arrstack_pop(stack);
        }
        assert(r2_arrstack_empty(stack) == TRUE);
        r2_destroy_arrstack(stack);
}

/**
 * @brief       Tests the top functionality of the stack.
 * 
 */
static void test_r2_arrstack_top()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        for(int i = 0; i < SIZE; ++i){
                r2_arrstack_push(stack, &arr[i]);
                assert(r2_arrstack_top(stack) == &arr[i]);
        }
        r2_destroy_arrstack(stack);
}

/**
 * @brief       Tests the empty functionality of the stack.
 * 
 */
static void test_r2_arrstack_empty()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        assert(r2_arrstack_empty(stack) == TRUE); 

        for(int i = 0 ; i < SIZE; ++i)
                r2_arrstack_push(stack, &arr[i]);

        assert(r2_arrstack_empty(stack) != TRUE); 
        r2_destroy_arrstack(stack);
}

/**
 * @brief       Tests the full functionality of the stack.
 * 
 */
static void test_r2_arrstack_full()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                r2_arrstack_push(stack, &arr[i]);
        
        assert(r2_arrstack_full(stack) == TRUE); 
        r2_destroy_arrstack(stack);
}

static void *cpy(const void *data)
{
        int *copy = malloc(sizeof(int));
        *copy = *((int *)data);
        return copy;  
}


static r2_int16 cmp(const void *d1, const void *d2)
{
        if(*((int *)d1) == *((int *)d2))
                return 0; 
        else if(*((int *)d1) < *((int *)d2))
                return -1; 
        else    return 1;
}

/**
 * @brief       Tests the comparison functionality of the stack.
 * 
 */
static void test_r2_arrstack_compare()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        
        /*Shallow comparison*/
        assert(r2_arrstack_compare(stack, stack) == TRUE);
        
        /*Deep comparison*/
        stack->cmp  = cmp;
        assert(r2_arrstack_compare(stack, stack) == TRUE);
        
        for(int i = 0; i < SIZE; ++i)
                r2_arrstack_push(stack, &arr[i]);
        
        /*Shallow comparison*/
        stack->cmp = NULL;
        assert(r2_arrstack_compare(stack, stack) == TRUE);
        
        /*Deep comparison*/
        stack->cmp  = cmp;
        assert(r2_arrstack_compare(stack, stack) == TRUE);
        
        /*Shallow comparison*/
        stack->cmp = NULL;
        stack->cpy = cpy;
        struct r2_arrstack *copy = r2_arrstack_copy(stack);
        assert(r2_arrstack_compare(stack, copy) != TRUE);

        /*Deep Comparison*/
        stack->cmp = cmp;
        copy->cmp  = cmp;
        assert(r2_arrstack_compare(stack, copy) == TRUE);

        r2_destroy_arrstack(stack);
        r2_destroy_arrstack(copy);
}




/**
 * @brief       Tests the copy functionality of a stack.
 * 
 */
static void test_r2_arrstack_copy()
{
        struct r2_arrstack *stack = r2_create_arrstack(0, NULL, NULL, NULL);
        struct r2_arrstack *dest  = r2_arrstack_copy(stack); 
        
        /*Shallow comparison.*/
        stack->cmp = NULL;
        assert(r2_arrstack_compare(stack, dest) == TRUE);

        /*Deep comparison*/
        stack->cmp = cmp;
        assert(r2_arrstack_compare(stack, dest) == TRUE);
        r2_destroy_arrstack(dest);
        
        for(int i = 0; i < SIZE; ++i)
                r2_arrstack_push(stack, &arr[i]);
        
        
        /*Shallow comparison after shallow copy*/
        dest  = r2_arrstack_copy(stack);
        assert(r2_arrstack_compare(stack, dest) == TRUE);
        
        /*Deep comparison after shallow copy*/
        stack->cmp = cmp;
        dest->cmp  = cmp;
        assert(r2_arrstack_compare(stack, dest) == TRUE);
        r2_destroy_arrstack(dest);
        
        /*Shallow comparison after deep copy*/
        stack->cpy = cpy;
        stack->cmp = NULL;
        dest  = r2_arrstack_copy(stack);
        assert(r2_arrstack_compare(stack, dest) != TRUE);
        
        /*Deep comparison after deep copy*/
        stack->cmp = cmp;
        dest->cmp  = cmp;
        assert(r2_arrstack_compare(stack, dest) == TRUE);
        
        r2_destroy_arrstack(stack);
        r2_destroy_arrstack(dest);
}



/**
 * @brief       Run all stack tests
 * 
 */
void test_r2_arrstack_run()
{
        test_r2_arrstack_init_data();
        test_r2_create_arrstack();
        test_r2_destroy_arrstack();
        test_r2_arrstack_push();
        test_r2_arrstack_pop();
        test_r2_arrstack_top();
        test_r2_arrstack_empty(); 
        test_r2_arrstack_full(); 
        test_r2_arrstack_compare(); 
        test_r2_arrstack_copy(); 
}