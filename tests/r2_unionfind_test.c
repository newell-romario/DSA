#include "r2_unionfind_test.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static r2_int16 cmp(const void *, const void *);

/**
 * @brief Test create universe functionality.
 * 
 */
static void test_r2_create_universe()
{
        struct r2_universe *u = r2_create_universe(NULL, NULL); 
        assert(u != NULL); 
        assert(u->nsets == 0);
        assert(u->sets != NULL);
        r2_destroy_universe(u);
}

/**
 * @brief Test destroy universe functionality.
 * 
 */
static void test_r2_destroy_universe()
{
        struct r2_universe *u = r2_create_universe(NULL, NULL); 
        assert(r2_destroy_universe(u) == NULL);
}

/**
 * @brief Test make set.
 * 
 */
static void test_r2_makeset()
{
        struct r2_universe *u = r2_create_universe(cmp, NULL);
        r2_uint64 A[3] = {1, 2, 3};
        for(r2_uint16 i = 0; i < 3; ++i)
                assert(r2_makeset(u, (r2_uc *)&A[i], sizeof(r2_uint64)) == TRUE);
        
        assert(u->nsets == 3);

        for(r2_uint16 i = 0; i < 3; ++i)
                assert(r2_findset(u, (r2_uc *)&A[i], sizeof(r2_uint64)) == (r2_uc *)&A[i]);

        r2_destroy_universe(u);
}

/**
 * @brief Test union set
 * 
 */
static void test_r2_unionset()
{
        struct r2_universe *u = r2_create_universe(cmp, NULL);
        r2_uint64 A[3]= {1, 4, 7};
        r2_uint16 B[4] = {2, 3, 6, 9};
        for(r2_uint16 i = 0; i < 3; ++i)
                assert(r2_makeset(u, (r2_uc *)&A[i], sizeof(r2_uint64)) == TRUE);

        for(r2_uint16 i = 0; i < 4; ++i)
                assert(r2_makeset(u, (r2_uc *)&B[i], sizeof(r2_uint64)) == TRUE);

        assert(r2_unionset(u, (r2_uc *)&A[0], sizeof(r2_uint64), (r2_uc *)&A[1], sizeof(r2_uint64)) == TRUE);
        assert(r2_unionset(u, (r2_uc *)&A[1], sizeof(r2_uint64),(r2_uc *)&A[2], sizeof(r2_uint64)) == TRUE);
        assert(r2_findset(u, (r2_uc *)&A[0], sizeof(r2_uint64)) == r2_findset(u, (r2_uc *)&A[1], sizeof(r2_uint64)) && 
        r2_findset(u, (r2_uc *)&A[1], sizeof(r2_uint64)) == r2_findset(u, (r2_uc *)&A[2], sizeof(r2_uint64)));
        
        assert(r2_unionset(u, (r2_uc *)&B[0], sizeof(r2_uint64), (r2_uc *)&B[1], sizeof(r2_uint64)) == TRUE);
        assert(r2_unionset(u, (r2_uc *)&B[1], sizeof(r2_uint64), (r2_uc *)&B[2], sizeof(r2_uint64)) == TRUE);
        assert(r2_unionset(u, (r2_uc *)&B[2], sizeof(r2_uint64), (r2_uc *)&B[3], sizeof(r2_uint64)) == TRUE);

        assert(r2_findset(u, (r2_uc *)&B[0], sizeof(r2_uint64)) == r2_findset(u, (r2_uc *)&B[1], sizeof(r2_uint64)) && 
        r2_findset(u, (r2_uc *)&B[1], sizeof(r2_uint64)) == r2_findset(u, (r2_uc *)&B[2], sizeof(r2_uint64)) && 
        r2_findset(u, (r2_uc *)&B[2], sizeof(r2_uint64)) == r2_findset(u, (r2_uc *)&B[3], sizeof(r2_uint64)));

        assert(r2_findset(u, (r2_uc *)&B[0], sizeof(r2_uint64)) != r2_findset(u,(r2_uc *)&A[0], sizeof(r2_uint64)));
        r2_destroy_universe(u);
}

/**
 * @brief Test same set functionality.
 * 
 */
static void test_r2_sameset()
{
        struct r2_universe *u = r2_create_universe(cmp, NULL);
        r2_uint64 A[3]= {1, 4, 7};
        r2_uint16 B[4] = {2, 3, 6, 9};

        for(r2_uint16 i = 0; i < 3; ++i)
                assert(r2_makeset(u, (r2_uc *)&A[i], sizeof(r2_uint64)) == TRUE);

        for(r2_uint16 i = 0; i < 4; ++i)
                assert(r2_makeset(u, (r2_uc *)&B[i], sizeof(r2_uint64)) == TRUE);
        
        r2_unionset(u, (r2_uc *)&A[0], sizeof(r2_uint64), (r2_uc *)&A[1], sizeof(r2_uint64));
        assert(r2_sameset(u, (r2_uc *)&A[0], sizeof(r2_uint64), (r2_uc *)&A[1], sizeof(r2_uint64)) == TRUE);
        assert(r2_sameset(u, (r2_uc *)&A[1], sizeof(r2_uint64), (r2_uc *)&A[2], sizeof(r2_uint64)) != TRUE);

        r2_destroy_universe(u);
}



/**
 * @brief Run test
 * 
 */
void test_r2_unionfind_run()
{
        test_r2_create_universe();
        test_r2_destroy_universe();
        test_r2_makeset();
        test_r2_unionset(); 
        test_r2_sameset();
}

static r2_int16 cmp(const void *a, const void *b)
{
        const  r2_uint64 *c = (const  r2_uint64 *)((const struct r2_key *)a)->key; 
        const  r2_uint64 *d = (const  r2_uint64 *)((const struct r2_key *)b)->key;
        return *c - *d;
}