#include "..\src\r2_trie.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void test_r2_create_trie()
{
        struct r2_trie *trie = r2_create_trie(NULL, NULL);
        assert(trie != NULL); 
        assert(trie->fk == NULL); 
        assert(trie->fd == NULL); 
        assert(trie->root != NULL); 
        r2_destroy_trie(trie);
}

static void test_r2_trie_insert()
{
        struct r2_trie *trie = r2_create_trie(NULL, NULL);
        const char *str[] = {"Romario", "Newell"}; 
        assert(r2_trie_insert(trie, (r2_uc *)str[0], strlen(str[0]), (r2_uc *)str[1]) == TRUE); 
        assert(r2_trie_insert(trie, (r2_uc *)str[1], strlen(str[1]), (r2_uc *)str[0]) == TRUE); 
        r2_destroy_trie(trie);
}

static void test_r2_trie_search()
{
        struct r2_trie *trie = r2_create_trie(NULL, NULL);
        const char *str[] = {"Romario", "Newell", "First"}; 
        r2_trie_insert(trie, (r2_uc *)str[0], strlen(str[0]), (r2_uc *)str[1]);
        r2_trie_insert(trie, (r2_uc *)str[1], strlen(str[1]), (r2_uc *)str[0]);
        assert(r2_trie_search(trie, (r2_uc *)str[0], strlen(str[0])) == str[1]); 
        assert(r2_trie_search(trie, (r2_uc *)str[1], strlen(str[1])) == str[0]);
        assert(r2_trie_search(trie, (r2_uc *)str[2], strlen(str[2])) == NULL);
        r2_trie_search(trie, "Rom", 3);
        r2_destroy_trie(trie);  
}

static void test_r2_trie_delete()
{
        struct r2_trie *trie = r2_create_trie(NULL, NULL);
        const char *str[] = {"Romario", "Newell", "First", "Romarios"}; 
        assert(r2_trie_insert(trie, (r2_uc *)str[0], strlen(str[0]), (r2_uc *)str[1]) == TRUE);
        assert(r2_trie_insert(trie, (r2_uc *)str[1], strlen(str[1]), (r2_uc *)str[0]) == TRUE);  
        assert(r2_trie_insert(trie, (r2_uc *)str[3], strlen(str[3]), (r2_uc *)str[3]) == TRUE);  
        assert(r2_trie_search(trie, (r2_uc *)str[0], strlen(str[0])) == str[1]);  
        assert(r2_trie_search(trie, (r2_uc *)str[1], strlen(str[1])) == str[0]); 
        assert(r2_trie_delete(trie, (r2_uc *)str[0], strlen(str[0])) == TRUE);
        assert(r2_trie_delete(trie, (r2_uc *)str[1], strlen(str[1])) == TRUE);
        assert(r2_trie_delete(trie, (r2_uc *)str[2], strlen(str[2])) == FALSE);
        assert(r2_trie_search(trie, (r2_uc *)str[0], strlen(str[0])) == NULL);  
        assert(r2_trie_search(trie, (r2_uc *)str[1], strlen(str[1])) == NULL); 
        r2_destroy_trie(trie);
}

static void test_r2_trie_longgest_prefix()
{
        struct r2_trie *trie = r2_create_trie(NULL, NULL);
        const char *str[] = {"Romario", "Newell", "First", "Romarios"}; 
        assert(r2_trie_insert(trie, (r2_uc *)str[0], strlen(str[0]), (r2_uc *)str[1]) == TRUE);
        assert(r2_trie_insert(trie, (r2_uc *)str[1], strlen(str[1]), (r2_uc *)str[0]) == TRUE);  
        assert(r2_trie_insert(trie, (r2_uc *)str[3], strlen(str[3]), (r2_uc *)str[3]) == TRUE);  
        assert(r2_trie_longest_prefix(trie, (r2_uc *)str[1], strlen(str[1])) == NULL);
        assert(strcmp(r2_trie_longest_prefix(trie, (r2_uc *)str[3], strlen(str[3])), str[0]) == 0);
        r2_destroy_trie(trie);
}
void test_r2_trie_run()
{
        test_r2_create_trie(); 
        test_r2_trie_insert();
        test_r2_trie_search();
        test_r2_trie_delete();
        test_r2_trie_longgest_prefix();
}