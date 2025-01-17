#ifndef BTREE_TEST_H_
#define BTREE_TEST_H_
#include "..\src\r2_btree.h"
static void test_r2_create_btree(); 
static void test_r2_create_page();
static void test_r2_destroy_btree(); 
static void test_r2_btree_insert(); 
static void test_r2_btree_delete();
static void test_r2_btree_search();
static void test_r2_btree_minimum(); 
static void test_r2_btree_maximum();
static void test_r2_page_successor();
static void test_r2_page_predecessor();
static void test_r2_btree_certify(const struct r2_page*, r2_cmp);
static void test_r2_btree_generate();    
static void test_r2_btree_height(const struct r2_page *);                                                                       
void test_r2_btree_run();


#endif