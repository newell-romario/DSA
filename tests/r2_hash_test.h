#ifndef R2_HASH_TEST_H_
#define R2_HASH_TEST_H_
#include "../src/r2_hash.h"
static struct r2_chain* test_r2_longest_chain(struct r2_chaintable *);
static void test_r2_print_chain(const struct r2_chain*);
static void test_r2_chaintable_generate();
static void test_r2_robintable_generate();
static void test_r2_chaintable_print(const struct r2_chaintable *);
static r2_uint64 test_r2_chaintable_first_index(const struct r2_chaintable *); 
static r2_uint64 test_r2_chaintable_last_index(const struct r2_chaintable *);
static void test_r2_create_chaintable(); 
static void test_r2_destroy_chaintable();
static void test_r2_chaintable_put(); 
static void test_r2_chaintable_get();
static void test_r2_chaintable_del();
static void test_r2_create_robintable(); 
static void test_r2_robintable_put();
static void test_r2_robintable_get();
static void test_r2_robintable_del();
static void test_r2_robintable_psl(struct r2_robintable *);
void test_r2_chaintable_run();
#endif