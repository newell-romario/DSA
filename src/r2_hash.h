#ifndef R2_HASHTABLE_H_
#define R2_HASHTABLE_H_
#include "r2_types.h"

/*
 * A hash table is a fundamental data structure which offers O(1) time for insertion, search and deletion in its
 * best case while O(n) in its worst case. Conceptually a hash table is really just an array that uses a 
 * special function to map keys to different locations in the array. This special function is called
 * a hash function. An important role of a hash function is to uniformly disperse the keys in the array; however, 
 * in an imperfect world this doesn't happen. We have mutiple keys mapping to the same location. Whenever this happens 
 * it is termed as a collision. A bad hash function has many collisions which drastically reduces the performance 
 * of a hash table. 
 * 
 * Another important part of a hash table implementation is collision resolution. How can we handle key collisions?
 * Key collisions can be handled in a few ways such as chaining, probing and double hashing. We discuss this more below. 
 * 
 * ____________________________________________________________
 * Hash Function
 * ____________________________________________________________
 * 
 * The hash function is of utmost importance when creating a hash table because if we choose the incorrect hash function 
 * then we have a poorly peforming hash table with many collisions.  According to Robert Sedgewick a good hash function aims to maximize
 * these three conditions: 
 * 
 *      1) It should be consistent.
 *      2) It should be efficient to compute.
 *      3) It should uniformly distribute the keys.
 * 
 * By consistent we mean if k1 == k2 then hash(k1) == hash(k2). A hash function is efficient to compute when 
 * the time taken to compute hash is minimal. Lastly, a hash function that uniformly distribute the keys minimizes 
 * collision by ensuring each location has the same probability of being hashed to. 
 * Hash functions that achieve these condition come in the forms of either multiplicative or division. 
 * 
 * Hash functions that use the division method are of this form h(k) = k mod M where M is the size of the
 * table. Examples of these type of hash functions can be found in Algorithms and Data Structures Applications by Goodrich, 
 * Algorithms by Robert Segdewick etc.
 * 
 * The multiplicative hash functions can be found in TOACP and https://opendatastructures.org/newhtml/ods/latex/hashing.html#tex2htm-61, and CLRS.
 * 
 * ______________________________________________________________
 * Collision Resolution
 * ______________________________________________________________
 * It doesn't matter how good a hash function is there will always be collisions. Since collisions are certain we need a way of resolving these collisions. 
 * In practice there are two main ways of resolving collisions which are separate chaining or open addressing. 
 * 
 * Separate chaining views the hash table as an array of lists (chain); when a collision occurs at a position we insert into the list at that position.
 * This means a cell in the array can contain numerous elements; a bad hash function would hash to one cell where all the keys would be inserted 
 * into that one list which means it takes linear time to perform any action.  
 * 
 * Open addressing views the hash table as simply an array where we use creative schemes such as linear probing, cuckoo hashing, hop scotch hashing, and quadratic probing
 * to resolve collisions. 
 * 
 * 
 * 
 */


/**
 * Enum representing all the different hash functions that can be used.
 * 
 */
enum hashfunc{
        WEE = 0,
        KNUTH,
        FNV,
        DBJ
};

typedef r2_uint64 (*r2_hashfunc)(const r2_uc *, r2_uint64, r2_uint64);

/**************************************Hash Functions*************************************/
r2_uint64 r2_hash_wee(const r2_uc *, r2_uint64, r2_uint64);
r2_uint64 r2_hash_knuth(const r2_uc*, r2_uint64, r2_uint64);
r2_uint64 r2_hash_fnv(const r2_uc*, r2_uint64, r2_uint64);
r2_uint64 r2_hash_dbj(const r2_uc*, r2_uint64, r2_uint64);
/**************************************Hash Functions*************************************/


/**
 * Struture containing key and length.
 */
struct r2_key{
        r2_uc *key; /*key*/
        r2_uint64 len; /*length*/
};


/**
 * An entry represents the key value pair. 
 * Entry is shared between all hash table implementation.
 * 
 */
struct r2_entry{
        r2_uc *key; /*key*/
        void  *data;/*data associated with key*/
        r2_uint64 length; /*length of key*/
};


/**
 * Represents node in the separate chain.
 * 
 */
struct r2_cnode{
        struct r2_entry *entry;/*entry*/
        r2_uint64 hash; /*hash*/
        struct r2_cnode *next;/*link to next entry in chain*/
        struct r2_cnode *prev;/*link to prev entry in chain*/
};

/**
 * Separate chain.
 * 
 */
struct r2_chain{
        r2_uint64 csize;/*number of entries*/
        struct r2_cnode *head;/*first entry in chain*/
        struct r2_cnode *tail;/*last entry in the chain*/
};

/**
 * Hash table using separate chaining.
 * 
 */
struct r2_chaintable{
        r2_uint64 nsize;/*Number of entries in the table*/
        r2_uint64 tsize;/*Number of buckets*/
        r2_ldbl lf;/*load factor*/
        struct r2_chain *chain;/*Buckets*/
        r2_hashfunc hf;/*Hash function*/
        r2_int16 prime;/*boolean representing if our hash table use prime number versus powers of 2*/
        r2_cmp kcmp;/*A callback comparison function for key*/
        r2_cmp dcmp;/*A callback comparison function for data*/
        r2_cpy kcpy;/*A callback function to copy keys*/
        r2_cpy dcpy;/*A callback function to copy values*/
        r2_fk fk;/*A callback function that release memory used by key*/
        r2_fd fd;/*A callback function that release memory used by data*/
        struct r2_chain *first;/*first entry*/
        struct r2_chain *end;/*last entry*/
};


struct r2_chaintable* r2_create_chaintable(r2_int16, r2_int16, r2_uint64, r2_ldbl, r2_cmp, r2_cmp, r2_cpy, r2_cpy,r2_fk, r2_fd); 
r2_uint16 r2_chaintable_put(struct r2_chaintable  *, r2_uc *, void *, r2_uint64);
r2_uint16 r2_chaintable_del(struct r2_chaintable*, r2_uc *, r2_uint64);
void* r2_chaintable_get(struct r2_chaintable *,  r2_uc *,  r2_uint64);
struct r2_chaintable* r2_destroy_chaintable(struct r2_chaintable *);


struct r2_robinentry{
        struct r2_entry entry;/*entry*/
        r2_uint64 hash; /*original position*/
        r2_uint64 psl; /*probe sequence length*/
};


struct r2_robintable{
        struct r2_robinentry **cells;/*cells in hash table*/
        r2_uint64 nsize;/*Number of entries in the table*/
        r2_uint64 tsize;/*Number of buckets*/
        r2_ldbl lf;/*load factor*/
        r2_uint64 psl; /*Maximum allowed length for a probe sequence*/
        r2_hashfunc hf;/*Hash function*/
        r2_int16 prime;/*Boolean representing if our hash table use prime number versus powers of 2*/
        r2_cmp kcmp;/*A callback comparison function for key*/
        r2_cmp dcmp;/*A callback comparison function for data*/
        r2_cpy kcpy;/*A callback function to copy keys*/
        r2_cpy dcpy;/*A callback function to copy values*/
        r2_fk fk;/*A callback function that release memory used by key*/
        r2_fd fd;/*A callback function that release memory used by data*/
};

struct r2_robintable* r2_create_robintable(r2_int16, r2_int16, r2_uint64, r2_uint64, r2_ldbl, r2_cmp, r2_cmp, r2_cpy, r2_cpy, r2_fk, r2_fd); 
struct r2_robintable* r2_robintable_put(struct r2_robintable *, r2_uc *, void *, r2_uint64);
struct r2_robintable* r2_robintable_get(struct r2_robintable *, r2_uc *,  r2_uint64, struct r2_entry *);
struct r2_robintable* r2_robintable_del(struct r2_robintable *, r2_uc *, r2_uint64); 
struct r2_robintable* r2_destroy_robintable(struct r2_robintable *);

#endif