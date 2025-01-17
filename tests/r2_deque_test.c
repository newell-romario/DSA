#include "r2_deque_test.h"
#include <assert.h>
#include <stdlib.h>
#define SIZE 100
static int arr[SIZE];

/**
 * @brief       Tests the create deque functionality.
 * 
 */
static void test_r2_create_deque()
{
        struct r2_deque *deque = r2_create_deque(NULL, NULL, NULL);

        /*Deque should be empty when just created.*/
        assert(r2_deque_empty(deque) == TRUE);
        assert(deque->cpy == NULL); 
        assert(deque->cmp == NULL); 
        assert(deque->fd == NULL);
        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the create deque functionality.
 * 
 */
static void test_r2_create_dequenode()
{
        struct r2_dequenode *node  = r2_create_dequenode();
        /*Node should be empty when just created.*/ 
        assert(node->data == NULL); 
        assert(node->next == NULL);
        free(node);
}

/**
 * @brief       Tests the destroy functionality of the deque.
 * 
 */
static void test_r2_destroy_deque()
{
        struct r2_deque *deque = r2_create_deque(NULL, NULL, NULL);
        assert(r2_destroy_deque(deque) == NULL);
}

/**
 * @brief       Tests the empty functionality of the deque.
 * 
 */
static void test_r2_deque_empty()
{
        struct r2_deque *deque = r2_create_deque(NULL, NULL, NULL);
        assert(r2_deque_empty(deque) == TRUE);

        for(r2_uint64 i = 0; i < SIZE;++i)
               deque = r2_deque_insert_at_front(deque, &arr[i]); 
        
        assert(r2_deque_empty(deque) != TRUE);
        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the insert at front functionality of the deque.
 * 
 */
static void test_r2_deque_insert_at_front()
{

        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *front = NULL;
        struct r2_dequenode *rear  = NULL;

        for(int i = 0; i < SIZE;++i){
               deque = r2_deque_insert_at_front(deque, &arr[i]);
               front = r2_deque_front(deque); 
               rear  = r2_deque_rear(deque);

               assert(rear->data == &arr[0]); 
               assert(front->data == &arr[i]); 
        }
        
        front = r2_deque_front(deque); 
        rear  = r2_deque_rear(deque);

        assert(deque->dsize == SIZE); 
        assert(rear->data   == &arr[0]); 
        assert(front->data  == &arr[SIZE - 1]);

        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the insert at back functionality.
 * 
 */
static void test_r2_deque_insert_at_back()
{
        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *front = NULL;
        struct r2_dequenode *rear  = NULL;

        for(int i = 0; i < SIZE;++i){
               deque = r2_deque_insert_at_back(deque, &arr[i]);
               rear  = r2_deque_rear(deque); 
               front = r2_deque_front(deque);
               assert(front->data == &arr[0]);
               assert(rear->data  == &arr[i]); 
        }
        
        front = r2_deque_front(deque); 
        rear  = r2_deque_rear(deque);

        assert(deque->dsize == SIZE); 
        assert(rear->data  == &arr[SIZE - 1]); 
        assert(front->data == &arr[0]);

        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the delete functionality at the front of the deque. 
 * 
 */
static void test_r2_deque_delete_at_front()
{
        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *front = NULL;

        for(int i = 0; i < SIZE;++i)
               deque = r2_deque_insert_at_back(deque, &arr[i]);
        
        for(int i = 0; i <= SIZE - 2; i += 2){
                front = r2_deque_front(deque); 
                assert(front->data == &arr[i]);
                deque = r2_deque_delete_at_front(deque);
                front = r2_deque_front(deque);
                assert(front->data == &arr[i + 1]);
                deque = r2_deque_delete_at_front(deque); 
        }

        assert(r2_deque_empty(deque) == TRUE); 
        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the delete functionality at the back of the deque.
 * 
 */
static void test_r2_deque_delete_at_back()
{
        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *rear  = NULL;

        for(int i = 0; i < SIZE;++i)
               deque = r2_deque_insert_at_back(deque, &arr[i]);
        

        for(int i = SIZE - 1; i >= 1; i -= 2){
                rear = r2_deque_rear(deque); 
                assert(rear->data == &arr[i]);
                deque = r2_deque_delete_at_back(deque);
                rear = r2_deque_rear(deque); 
                assert(rear->data == &arr[i - 1]);
                deque = r2_deque_delete_at_back(deque);
        }
        
        assert(r2_deque_empty(deque) == TRUE); 
        r2_destroy_deque(deque);
}

/**
 * @brief       Tests the front functionality of the deque.
 * 
 */
static void test_r2_deque_front()
{
        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *front = NULL;

        for(int i = 0; i < SIZE;++i){
               deque = r2_deque_insert_at_back(deque, &arr[i]); 
               front = r2_deque_front(deque); 
               assert(front->data == &arr[0]);
        }

        r2_destroy_deque(deque);
}


/**
 * @brief       Tests the insert rear functionality of the deque.
 * 
 */
static void test_r2_deque_rear()
{
        struct r2_deque *deque     = r2_create_deque(NULL, NULL, NULL);
        struct r2_dequenode *rear  = NULL;

        for(int i = 0; i < SIZE;++i){
               deque = r2_deque_insert_at_back(deque, &arr[i]); 
               rear  = r2_deque_rear(deque); 
               assert(rear->data == &arr[i]);
        }

        r2_destroy_deque(deque);
}

static void* cpy(const void *data)
{
        int *copy = malloc(sizeof(int));
        *copy = *((int *)data);
        return copy; 
}

static r2_int16 cmp(const void *s1, const void *s2)
{
        return (*(int *)s1) == (*(int *)s2); 
}

/**
 * @brief Tests the copy functionality of the deque.
 * 
 */
static void test_r2_deque_copy()
{

        struct r2_deque *source = r2_create_deque(NULL, NULL, NULL);
        struct r2_deque *dest   = r2_deque_copy(source); 

        /**
         * @brief Compares two empty deque.
         * 
         */
        
        /*Shallow comparison*/
        assert(r2_deque_compare(source, dest) == TRUE);

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, dest) == TRUE); 
        r2_destroy_deque(dest);
        
        /*Deep copy*/
        source->cpy = cpy;
        dest   = r2_deque_copy(source);  /*Copies an empty deque using the copy function passed.*/
        /*Shallow comparison*/
        source->cmp = NULL;
        assert(r2_deque_compare(source, dest) == TRUE); /*Compares an empty deque.*/
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, dest) == TRUE);
        r2_destroy_deque(dest);
     

        for(r2_uint64 i = 0; i < SIZE; ++i)
                source = r2_deque_insert_at_back(source, &arr[i]);
        

        /*Does a shallow copy and shallow comparison.*/
        source->cpy = NULL;
        source->cmp = NULL;
        dest        = r2_deque_copy(source);
        assert(r2_deque_compare(source, dest) == TRUE);
        
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, dest) == TRUE); 
        r2_destroy_deque(dest);
        
        /*Does a deep copy and shallow comparison.*/
        source->cpy = cpy;
        source->cmp = NULL;
        dest   = r2_deque_copy(source);
        assert(r2_deque_compare(source, dest) != TRUE);
        
        /*Deep Comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, dest) == TRUE); 
        
        r2_destroy_deque(source); 
        r2_destroy_deque(dest);
}

/**
 * @brief       Tests compare functionality of the deque.
 * 
 */
static void test_r2_deque_compare()
{
        /*Compares an empty deque*/
        struct r2_deque *source = r2_create_deque(NULL, NULL, NULL);
        assert(r2_deque_compare(source, source) == TRUE);
        assert(r2_deque_compare(source, source) == TRUE); 


        for(int i = 0; i < SIZE; ++i)
                source = r2_deque_insert_at_back(source, &arr[i]);
        
        /*Compares a deque against itself*/

        /*Shallow comparison*/
        assert(r2_deque_compare(source, source) == TRUE);

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, source) == TRUE); 

        /*Compares a deque against itself after deep copy*/
        source->cpy = cpy;
        source->cmp = NULL;
        struct r2_deque *dest   = r2_deque_copy(source); 
        assert(r2_deque_compare(source, dest) != TRUE);
        
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_deque_compare(source, dest) == TRUE); 

        r2_destroy_deque(source); 
        r2_destroy_deque(dest);
}

/**
 * @brief       Initialize the test data
 * 
 */
static void test_r2_deque_init_data()
{
        for(int i = 0; i < SIZE;++i){
                arr[i] = rand() % SIZE + 1;
        }
}

/**
 * @brief       Runs all the test
 * 
 */
void test_r2_deque_run()
{
        test_r2_deque_init_data();
        test_r2_create_deque();
        test_r2_create_dequenode();
        test_r2_destroy_deque();
        test_r2_deque_empty();
        test_r2_deque_insert_at_front();
        test_r2_deque_insert_at_back();
        test_r2_deque_delete_at_front();
        test_r2_deque_delete_at_back();
        test_r2_deque_front();
        test_r2_deque_rear();
        test_r2_deque_copy(); 
        test_r2_deque_compare();
}