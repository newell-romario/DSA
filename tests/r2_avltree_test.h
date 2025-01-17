#ifndef AVL_TREE_TEST_H_
#define AVL_TREE_TEST_H_
#include "..\src\r2_avltree.h"


static void test_r2_create_avlnode(); 
static void test_r2_avlnode_successor(); 
static void test_r2_avlnode_predecessor();
static void test_r2_avlnode_min(); 
static void test_r2_avlnode_max();
static void test_r2_create_avltree(); 
static void test_r2_destroy_avltree();
static void test_r2_avltree_insert(); 
static void test_r2_avltree_delete(); 
static void test_r2_avltree_search(); 
static void test_r2_avltree_height();
static void test_r2_avltree_level(); 
static void test_r2_avltree_size();
static void test_r2_avltree_empty();
static void test_r2_avltree_at();
static void test_r2_avltree_inorder();
static void test_r2_avltree_postorder(); 
static void test_r2_avltree_preorder();
static void test_r2_avltree_getkeys();
static void test_r2_avltree_getvalues();
static void test_r2_avltree_rangequery();
static void test_r2_avltree_compare();
static void test_r2_avltree_copy();
static void test_r2_avltree_certify(const struct r2_avlnode *root, r2_cmp cmp);
static void test_r2_avltree_is_avltree(const struct r2_avlnode *root);
static void test_r2_avltree_is_binary_tree(const struct r2_avlnode *root, r2_cmp);
static void test_r2_avltree_generate();
void test_r2_avltree_run();

/*TO DO a certification function*/
#endif