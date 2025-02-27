#ifndef WAVLTREE_TEST_H_
#define WAVLTREE_TEST_H_
#include "../src/r2_wavltree.h"
static void test_r2_create_wavlnode(); 
static void test_r2_create_wavltree();
static void test_r2_wavltree_empty();
static void test_r2_destroy_wavltree();
static void test_r2_wavlnode_successor();
static void test_r2_wavlnode_predecessor();
static void test_r2_wavlnode_min();
static void test_r2_wavlnode_max();
static void test_r2_wavltree_search(); 
static void test_r2_wavtlree_insert(); 
static void test_r2_wavltree_delete(); 
static void test_r2_wavltree_at(); 
static void test_r2_wavltree_get_keys(); 
static void test_r2_wavltree_get_values();
static void test_r2_wavltree_certify(const struct r2_wavlnode *);
static void test_r2_wavltree_compare();
static void test_r2_wavltree_copy();
static void test_r2_wavltree_inorder();
static void test_r2_wavltree_postorder(); 
static void test_r2_wavltree_preorder();
static void test_r2_wavltree_range_query(); 
static void test_r2_wavltree_generate();
static void test_r2_wavltree_stats();
void test_r2_wavltree_run();
#endif 