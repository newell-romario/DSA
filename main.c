#include "tests/r2_stack_test.h"
#include "tests/r2_queue_test.h"
#include "tests/r2_deque_test.h"
#include "tests/r2_arrstack_test.h"
#include "tests/r2_list_test.h"
#include "tests/r2_ring_test.h"
#include "tests/r2_avltree_test.h"
#include "tests/r2_rbtree_test.h"
#include "tests/r2_wavltree_test.h"
#include "tests/r2_btree_test.h"
#include "tests/r2_graph_test.h"
#include "tests/r2_hash_test.h"
#include "tests/r2_heap_test.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static r2_int16 cmp(const void *, const void *);
static void benchmark_tree();


int main()
{
        test_r2_stack_run();
        test_r2_queue_run();
        test_r2_deque_run();
        test_r2_arrstack_run();
        test_r2_list_run();
        test_r2_ring_run();
        test_r2_pq_run();
        //test_r2_avltree_run();
        //test_r2_rbtree_run();
        //test_r2_wavltree_run(); 
        //test_r2_btree_run(); 
        //test_r2_chaintable_run();

        //r2_graph_run();
        return 0;
}
