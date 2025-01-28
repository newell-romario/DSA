#include "r2_heap_test.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static r2_int16 mincmp(const void *, const void *);
static r2_int16 maxcmp(const void *, const void *);

/**
 * @brief  Tests create functionality.
 * 
 */
static void test_r2_create_priority_queue()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 1, NULL, NULL,NULL);
        assert(pq != NULL); 
        assert(pq->ncount == 0); 
        assert(pq->pqsize == (64)); 
        assert(pq->type == 1);
        assert(pq->kcmp == NULL);
        assert(pq->fd == NULL);
        r2_destroy_priority_queue(pq);
}
/**
 * @brief   Tests destroy functionality.
 * 
 */
static void test_r2_destroy_priority_queue()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 1, NULL, NULL, NULL);
        assert(r2_destroy_priority_queue(pq)  == NULL);
}

/**
 * @brief  Tests insert functionality.
 * 
 */
static void test_r2_pq_insert()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 0, mincmp, NULL, NULL);
        r2_uint64 values[] = {10,9,8,7,6,5,4,3,2,1,0};
        for(r2_uint64 i = 0; i < 11; ++i)
                r2_pq_insert(pq, &values[i]);

        assert(r2_pq_first(pq)->data == &values[10]);
        r2_destroy_priority_queue(pq);
}

/**
 * @brief Tests first functionality.
 * 
 */
static void test_r2_pq_first()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 0, mincmp, NULL, NULL);
        r2_uint64 values[] = {10,9,8,7,6,5,4,3,2,1,0};
        for(r2_uint64 i = 0; i < 11; ++i){
                r2_pq_insert(pq, &values[i]);
                assert(r2_pq_first(pq)->data == &values[i]);
        }
        r2_destroy_priority_queue(pq);
}

/**
 * @brief  Remove the smallest element.
 * 
 */
static void test_r2_pq_remove_root()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 0, mincmp, NULL, NULL);
        r2_uint64 values[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        for(r2_uint64 i = 0; i < 11; ++i)
                r2_pq_insert(pq, &values[i]);
       
        r2_int64 j = 9;
        for(r2_uint64 i = 0; i < 11; ++i,--j){
                pq = r2_pq_remove(pq, r2_pq_first(pq));
                if(j >= 1)
                        assert(r2_pq_first(pq)->data == &values[j]);
        }
        r2_destroy_priority_queue(pq);
}

/**
 * @brief Test the increase or decrease of a priority for an element.
 * 
 */
static void test_r2_pq_adjust()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 0, mincmp, NULL, NULL);
        r2_int64 values[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        for(r2_uint64 i = 0; i < 11; ++i)
                r2_pq_insert(pq, &values[i]);

        struct r2_locator * l = r2_pq_first(pq);
        r2_int64 *v = l->data;
        *v = *v +  20;
        pq = r2_pq_adjust(pq, l, 1);
        assert(*(r2_int64 *)r2_pq_first(pq)->data == values[9]);

        *v = *v - 50;
        pq = r2_pq_adjust(pq, l, 0);
        assert(*(r2_int64 *)r2_pq_first(pq)->data == *v);
        r2_destroy_priority_queue(pq);
}

/**
 * @brief Test empty priority queue.
 * 
 */
static void test_r2_pq_empty()
{
        struct r2_pq *pq = r2_create_priority_queue(64, 0, mincmp, NULL, NULL);
        assert(r2_pq_empty(pq) == TRUE);
        r2_uint64 values[] = {10,9,8,7,6,5,4,3,2,1,0};
        for(r2_uint64 i = 0; i < 11; ++i)
                r2_pq_insert(pq, &values[i]);
        
        assert(r2_pq_empty(pq) != TRUE);
        r2_destroy_priority_queue(pq);

}


static r2_int16 mincmp(const void *a, const void *b)
{
        const r2_int64 *c = a; 
        const r2_int64 *d = b; 
        if(*c <= *d)
                return 0;
        
        return 1;
}

static r2_int16 maxcmp(const void *a, const void *b)
{
        const r2_int64 *c = a; 
        const r2_int64 *d = b; 
        if(*c <= *d)
                return 0;
        
        return 1;
}

/**
 * @brief Run all tests.
 * 
 */
void test_r2_pq_run()
{
        test_r2_create_priority_queue();
        test_r2_destroy_priority_queue();
        test_r2_pq_insert();
        test_r2_pq_first();
        test_r2_pq_remove_root();
        test_r2_pq_empty();
        test_r2_pq_adjust();
}