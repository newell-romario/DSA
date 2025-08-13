#ifndef R2_TRIE_H_
#define R2_TRIE_H_
#include "r2_types.h"
#define ALPHABET 256

struct r2_trienode{
        r2_uint64 tcount;/*number of keys*/
        struct r2_trienode *parent;/*parent of current node*/
        struct r2_trienode *keys[ALPHABET];/*number of keys stored*/
        void *data;
};

struct r2_trie{
        struct r2_trienode *root;/*root*/
        r2_uint64 nelems;/*number of elements*/
        r2_fk fk;/*a callback function to free key*/
        r2_fd fd;/*a callback function to free data*/
}; 

struct r2_trie* r2_create_trie(r2_fk, r2_fd); 
struct r2_trie* r2_destroy_trie(const struct r2_trie *);
r2_uint16 r2_trie_insert(struct r2_trie *, r2_uc*, r2_uint64, void *);
r2_uint16 r2_trie_delete(struct r2_trie *, r2_uc *, r2_uint64);
void* r2_trie_search(struct r2_trie *, r2_uc *, r2_uint64);
char* r2_trie_longest_prefix(struct r2_trie *, r2_uc *, r2_uint64);
#endif