#include "r2_queue_test.h"
#include <stdlib.h>
#include <assert.h>

#define SIZE 100
static int arr[SIZE];

/**
 * @brief Initializes test data.
 * 
 */
static void test_init_data()
{
        for(int i = 0; i < SIZE;++i)
                arr[i] = rand() % SIZE + 1;
        
}

/**
 * @brief Tests the create functionality for a queue.
 * 
 */
static void test_r2_create_queue()
{
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        assert(r2_queue_empty(queue) == TRUE);
        assert(queue->front == NULL); 
        assert(queue->rear  == NULL);
        assert(queue->cmp   == NULL);
        assert(queue->cpy   == NULL);
        assert(queue->fd    == NULL);
        assert(queue->qsize == 0); 
        r2_destroy_queue(queue);
}


/**
 * @brief Tests the destroy functionality.
 * 
 */
static void test_r2_destroy_queue()
{
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        assert(r2_destroy_queue(queue) == NULL);
}


/**
 * @brief Tests the enqueue functionality of the queue. 
 * 
 */
static void test_r2_queue_enqueue()
{
        struct r2_queue         *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_queuenode     *rear  = NULL;
        struct r2_queuenode     *front = NULL; 
        for(int i = 0; i < SIZE; ++i){
                r2_queue_enqueue(queue, &arr[i]);
                front = r2_queue_front(queue);
                rear  = r2_queue_rear(queue);
                assert(front->data == &arr[0]);
                assert(rear->data  == &arr[i]);
        }

        assert(queue->qsize == SIZE);
        r2_destroy_queue(queue);
}

/**
 * @brief Tests the dequeue functionality.
 * 
 */
static void test_r2_queue_dequeue()
{
        
        struct r2_queue         *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_queuenode     *front = NULL; 
        for(int i = 0; i < SIZE; ++i)
                r2_queue_enqueue(queue, &arr[i]);
        

        for(int i = 0; i < SIZE; i += 2){

                front = r2_queue_front(queue);
                assert(front->data == &arr[i]);
                r2_queue_dequeue(queue);
                front = r2_queue_front(queue);
                assert(front->data == &arr[i + 1]);
                r2_queue_dequeue(queue);
        }

        

        assert(r2_queue_empty(queue) == TRUE);
        r2_destroy_queue(queue);
}

/**
 * @brief Test the front functionality of the queue.
 * 
 */
static void test_r2_queue_front()
{
        struct r2_queue         *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_queuenode     *front = NULL; 
        for(int i = 0; i < SIZE; ++i){
                r2_queue_enqueue(queue, &arr[i]);
                front = r2_queue_front(queue);
                assert(front->data == &arr[0]);
        }

        r2_destroy_queue(queue);
}


/**
 * @brief This function tests the rear functionality of the queue.
 * 
 */
static void test_r2_queue_rear()
{
        struct r2_queue     *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_queuenode *rear  = NULL; 
        for(int i = 0; i < SIZE; ++i){
                r2_queue_enqueue(queue, &arr[i]);
                rear  = r2_queue_rear(queue);
                assert(rear->data == &arr[i]);
        }

        r2_destroy_queue(queue);
}

/**
 * @brief This function tests the empty functionality of a queue
 * 
 */
static void test_r2_queue_empty()
{
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        assert(r2_queue_empty(queue) == TRUE);
        r2_destroy_queue(queue);
}

/**
 * @brief               Copy function to be used with deep copy.
 * 
 * @param data          Data to be copied.
 * @return void*        New copy of the data.
 */
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
 * @brief Tests the deep copy functionality.
 * 
 */
static void test_r2_queue_copy()
{
        struct r2_queue *source = r2_create_queue(NULL, cpy, NULL);
        struct r2_queue *dest   = r2_queue_copy(source); 

        /*Testing the deep copy functionality*/
        
        /*Shallow comparison*/
        assert(r2_queue_compare(source, dest) == TRUE);
        
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, dest) == TRUE); 
        r2_destroy_queue(dest);

        /* Shallow copy*/
        source->cpy = NULL;
        dest = r2_queue_copy(source);

        /*Shallow comparison*/
        source->cmp = NULL;
        assert(r2_queue_compare(source, dest) == TRUE);
        
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, dest) == TRUE);
        r2_destroy_queue(dest);


        for(int i = 0; i < SIZE;++i)
                r2_queue_enqueue(source, &arr[i]);
        

        /* Shallow copy.*/
        source->cpy = NULL;
        dest = r2_queue_copy(source);
        
        /*Shallow comparison*/
        source->cmp = NULL;
        assert(r2_queue_compare(source, dest) == TRUE);

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, dest) == TRUE);
        r2_destroy_queue(dest);

        /*Deep copy.*/
        source->cpy = cpy;
        dest = r2_queue_copy(source);

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, dest) == TRUE);

        /*Shallow comparison*/
        source->cmp = NULL;
        assert(r2_queue_compare(source, dest) != TRUE);

        r2_destroy_queue(source);
        r2_destroy_queue(dest);
}

/**
 * @brief Tests the comparison function for a queue.
 * 
 */
static void test_r2_queue_compare()
{
        struct r2_queue *source = r2_create_queue(NULL, NULL, NULL);
        /**
         * Compares queue against itself when empty.
         * 
         */
        /*shallow comparison*/
        assert(r2_queue_compare(source, source) == TRUE);
        
        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, source) == TRUE);
        
        for(int i = 0; i < SIZE;++i)
                r2_queue_enqueue(source, &arr[i]);
        

        /**
         * Compares queue against itself.
         * 
         */

        /*shallow comparison*/
        source->cmp = NULL;
        assert(r2_queue_compare(source, source) == TRUE);

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, source) == TRUE);

        /**
         * Compares queue against deep copy.
         * 
         */
        source->cpy = cpy;
        struct r2_queue *copy = r2_queue_copy(source); 

        /*Deep comparison*/
        source->cmp = cmp;
        assert(r2_queue_compare(source, copy) == TRUE);
        /*Shallow comparison*/
        source->cmp = NULL;
        assert(r2_queue_compare(source, copy) != TRUE);

        r2_destroy_queue(copy);
        r2_destroy_queue(source);
}

/**
 * @brief       Runs all the test.
 * 
 */
void test_r2_queue_run()
{
        test_init_data();
        test_r2_create_queue();
        //test_r2_create_queuenode();
        test_r2_destroy_queue();
        test_r2_queue_compare();
        test_r2_queue_dequeue();
        test_r2_queue_enqueue();
        test_r2_queue_front();
        test_r2_queue_rear();
        test_r2_queue_copy();
        test_r2_queue_empty();
}