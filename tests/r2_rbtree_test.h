#ifndef RB_TREETEST_H_
#define RB_TREETEST_H_
#include "..\src\r2_rbtree.h"

static void test_r2_rbtree_empty();
static void test_r2_create_rbnode();
static void test_r2_create_rb_tree();
static void test_r2_destroy_rbtree();
static void test_r2_rbnode_successor(); 
static void test_r2_rbnode_predecessor();
static void test_r2_rbnode_min();
static void test_r2_rbnode_max();
static void test_r2_rbnode_insert();
static void test_r2_rbnode_delete();
static void test_r2_rbnode_search();
static void test_r2_rbnode_size();
static void test_r2_rbtree_at(); 
static void test_r2_rbtree_inorder(); 
static void test_r2_rbtree_preorder(); 
static void test_r2_rbtree_postorder();
static void test_r2_rbtree_copy(); 
static void test_r2_rbtree_compare(); 
static void test_r2_rbtree_getkeys();
static void test_r2_rbtree_getvalues();
static r2_uint64 test_r2_rbnode_blackheight(const struct r2_rbnode *);
static void test_r2_r2_rbnode_noconsecreds(const struct r2_rbnode *); 
static void test_r2_rbtree_is_binary_tree(const struct r2_rbnode *, r2_cmp);
static void test_r2_rbtree_certify(const struct r2_rbnode *, r2_cmp);
static void test_r2_rbtree_generate();
void test_r2_rbtree_run();
#endif