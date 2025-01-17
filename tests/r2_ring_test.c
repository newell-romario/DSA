#include "r2_ring_test.h"
#include <stdlib.h>
#include <assert.h>
#define  SIZE 100

static int arr[SIZE]; 

/**
 * @brief Initializes test data.
 * 
 */
static void test_r2_ring_init_data()
{
        for(int i = 0; i < SIZE; ++i)
                arr[i] = rand() % SIZE;
}


/**
 * @brief       Tests the at functionality of the ring.
 * 
 */
static void test_r2_ring_at()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < SIZE; ++i){
                ring = r2_ring_insert(ring, &arr[i]); 
                assert(r2_ring_at(ring, i) == &arr[i]);
        }
        r2_destroy_ring(ring);
}

/**
 * @brief Tests the create functionality of the ring.
 * 
 */
static void test_r2_ring_create_ring()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        assert(ring != NULL);
        assert(ring->rsize == SIZE); 
        assert(ring->data != NULL); 
        assert(ring->cpy == NULL); 
        assert(ring->cmp == NULL); 
        assert(ring->fd  == NULL);
        assert(ring->ncount == 0);
        assert(ring->front == 0); 
        assert(ring->rear == 0);
        r2_destroy_ring(ring);
}


/**
 * @brief  Tests the destroy functionality of the ring.
 * 
 */
static void test_r2_ring_destroy_ring()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        assert(r2_destroy_ring(ring) == NULL);
}

/**
 * @brief Tests the insert functionality of the ring.
 * 
 */
static void test_r2_ring_insert()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);   
        for(r2_uint64 i = 0; i < SIZE; ++i){
                ring = r2_ring_insert(ring, &arr[i]); 
                assert(ring->ncount == (i + 1)); 
                assert(r2_ring_at(ring, i) == &arr[i]);
        }

        ring = r2_ring_insert(ring, &arr[0]); 
        r2_destroy_ring(ring);
}

/**
 * @brief Tests the delete functionality of the ring.
 * 
 */
static void test_r2_ring_delete()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < SIZE; ++i)
                ring  = r2_ring_insert(ring, &arr[i]); 

        void *data = NULL;        
        for(r2_uint64 i = 0;i <= SIZE - 2;i += 2){
                data = r2_ring_front(ring);
                assert(data == &arr[i]);
                ring = r2_ring_delete(ring);
                data = r2_ring_front(ring);
                assert(data == &arr[i + 1]);
                ring = r2_ring_delete(ring);
        }

        ring = r2_ring_delete(ring);
        r2_destroy_ring(ring);
}

/**
 * @brief Tests the empty functionality of the ring.
 * 
 */
static void test_r2_ring_empty()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        assert(r2_ring_empty(ring) == TRUE);
        
        for(r2_uint64 i = 0; i < SIZE; ++i)
                ring = r2_ring_insert(ring, &arr[i]); 
        
        assert(r2_ring_empty(ring) != TRUE);
        r2_destroy_ring(ring);
}

/**
 * @brief Tests the front functionality of the ring.
 * 
 */
static void test_r2_ring_front()
{
        struct r2_ring  *ring = r2_create_ring(SIZE, NULL, NULL, NULL);
        for(int i = 0; i < SIZE; ++i){
                ring = r2_ring_insert(ring, &arr[i]); 
                assert(r2_ring_front(ring) == &arr[0]);
        }

        r2_destroy_ring(ring);        
}

static void *cpy(const void *data)
{
        int *copy = malloc(sizeof(int)); 
        *copy = *((int *)data); 
        return copy;
}
static r2_int16 cmp(const void *d1, const void *d2)
{
        return *((int *)d1) == *((int *)d2);
}

/**
 * @brief    Tests the ring copy function
 * 
 */
static void test_r2_ring_copy()
{
        struct r2_ring *ring = r2_create_ring(SIZE, NULL, NULL, NULL);

        /*Shallow copy.*/
        struct r2_ring *copy = r2_ring_copy(ring);
        
        /*Shallow comparison*/
        assert(r2_ring_compare(ring, copy) == TRUE);
        
        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, copy) == TRUE);
        r2_destroy_ring(copy);


        /*Deep copy.*/
        ring->cpy = cpy;
        copy = r2_ring_copy(ring);
        
        /*Shallow comparison*/
        ring->cmp = NULL;
        assert(r2_ring_compare(ring, copy) == TRUE);
        
        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, copy) == TRUE);
        r2_destroy_ring(copy);

        for(int i = 0; i < SIZE; ++i)
                ring = r2_ring_insert(ring, &arr[i]); 
               
        
        /*Shallow copy.*/
        ring->cpy = NULL;
        copy = r2_ring_copy(ring);
        
        /*Shallow comparison*/
        ring->cmp = NULL; 
        assert(r2_ring_compare(ring, copy) == TRUE);
        
        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, copy) == TRUE);
        r2_destroy_ring(copy);

        /*Deep copy.*/
        ring->cpy = cpy;
        copy = r2_ring_copy(ring);
        
        /*Shallow comparison*/
        ring->cmp = NULL; 
        assert(r2_ring_compare(ring, copy) != TRUE);
        
        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, copy) == TRUE);
        
        r2_destroy_ring(copy);
        r2_destroy_ring(ring);
}

/**
 * @brief  Test the compare functionality.
 * 
 */
static void test_r2_ring_compare()
{
        struct r2_ring *ring  = r2_create_ring(SIZE, NULL, NULL, NULL);
        
        /*Shallow comparison*/
        assert(r2_ring_compare(ring, ring) == TRUE); 
        
        /*Deep Comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, ring) == TRUE);

        for(int i = 0; i < SIZE; ++i)
                ring = r2_ring_insert(ring, &arr[i]); 
        
        /*Shallow comparison*/
        ring->cmp = NULL;
        assert(r2_ring_compare(ring, ring) == TRUE); 

        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, ring) == TRUE);

        ring->cpy = cpy;
        struct r2_ring *copy = r2_ring_copy(ring);

        /*Shallow comparison*/
        ring->cmp = NULL;
        assert(r2_ring_compare(ring, copy) != TRUE); 

        /*Deep comparison*/
        ring->cmp = cmp;
        assert(r2_ring_compare(ring, copy) == TRUE);

        r2_destroy_ring(copy);
        r2_destroy_ring(ring);
}

/**
 * @brief     Run all ring tests.  
 * 
 */
void test_r2_ring_run()
{
        test_r2_ring_init_data(); 
        test_r2_ring_create_ring();
        test_r2_ring_destroy_ring();
        test_r2_ring_insert();
        test_r2_ring_delete();
        test_r2_ring_empty();
        test_r2_ring_front();
        test_r2_ring_copy();
        test_r2_ring_at();
        test_r2_ring_compare();
}
