#include "r2_hash.h"
#include <stdlib.h>
#include <assert.h>
#define WORD sizeof(void * )
#define PSL     4

/********************File scope functions************************/
static r2_uint64 r2_get_tsize(r2_uint64, r2_int16, r2_int16);
static void r2_freenode(struct r2_cnode *,  r2_fk, r2_fd);
static void r2_free_robinentry(struct r2_robinentry *, r2_fk, r2_fd);
static struct r2_entry* r2_create_entry();
static struct r2_cnode* r2_create_cnode();
static void r2_chain_insert(struct r2_chain*, r2_uc*, void *, r2_uint64 ,r2_uint64);
static void r2_chain_remove(struct r2_chain*, struct r2_cnode *, r2_fk, r2_fd, r2_cmp);
static struct r2_cnode* r2_chain_search(const struct r2_chain *, r2_uc *, r2_uint64, r2_cmp);
static struct r2_chaintable* r2_chaintable_resize(struct r2_chaintable *, r2_uint16);
static r2_uint16 r2_robintable_resize(struct r2_robintable*, r2_uint16);
/********************File scope functions************************/


/**
 * @brief                               Creates an empty hash table which uses separate chaining for collision resolution.
 *                                      The hash table size can either be prime or composite number. Ideally if the size is a 
 *                                      composite number it should be a power of two. If you opted to use prime a number for the size of the hash table. 
 *                                      We find the next prime greater than your suggested table size. 
 *                                      We have a prebuilt computed table of primes which we select from. 
 *                                      This means your table size if prime can only be the maximum allowed prime. 
 *                                      N.B Hash table sizes which are prime perform better in this implementation.                          
 * 
 * 
 * @param hf                            Hash function.
 * @param prime                         When prime == 1, hash table size is prime, else hash table size is a power of two.
 * @param tsize                         Hash table size.
 * @param kcmp                          A callback comparison function to compare keys.
 * @param dcmp                          A callback comparison function to compare data.
 * @param kcpy                          A callback function to copy keys.
 * @param dcpy                          A callback function to copy values.
 * @param fk                            A callback function that releases memory used by key.
 * @param fd                            A callback function that releases memory used by data.
 * @return struct r2_chaintable*        Returns an empty hash table, else NULL.
 */
struct r2_chaintable* r2_create_chaintable(r2_int16 hf, r2_int16 prime, r2_uint64 tsize, r2_cmp kcmp, r2_cmp dcmp, r2_cpy kcpy, r2_cpy dcpy,r2_fk fk, r2_fd fd)
{
        r2_hashfunc hfs[] = {
                r2_hash_wee,
                r2_hash_knuth,
                r2_hash_fnv,
                r2_hash_dbj
        }; 

        struct r2_chaintable *table = malloc(sizeof(struct r2_chaintable)); 
        if(table != NULL){
                table->nsize  = 0;
                table->prime  = prime;
                table->tsize  = tsize != 0 && prime != 1? tsize :  r2_get_tsize(tsize, 1, table->prime); 
                table->hf     = hfs[hf];
                table->kcmp   = kcmp; 
                table->dcmp   = dcmp; 
                table->kcpy   = kcpy; 
                table->dcpy   = dcpy;
                table->fk     = fk;
                table->fd     = fd;
                table->chain  = malloc(sizeof(struct r2_chain) * table->tsize);
                if(table->chain != NULL){
                        /*Initializes hash table*/
                        for(r2_uint64 i = 0; i < table->tsize; ++i){
                                table->chain[i].head  = NULL; 
                                table->chain[i].tail  = NULL; 
                                table->chain[i].csize = 0;
                        }
                }else{
                        free(table); 
                        table = NULL;
                }
        }
        return table; 
}

/**
 * @brief                               Destroys hash table. 
 * 
 * @param table                         Hash table.
 * @return struct r2_chaintable*        Returns NULL whenever table is destroyed properly.
 */
struct r2_chaintable* r2_destroy_chaintable(struct r2_chaintable *table)
{
        struct r2_cnode *head  = NULL; 
        struct r2_cnode  *prev = NULL; 
        for(r2_uint64 i = 0; i < table->tsize; ++i){
                if(table->chain[i].csize != 0){
                        head = table->chain[i].head; 
                        while(head != NULL){
                                prev = head;
                                head = head->next; 
                                r2_freenode(prev, table->fk, table->fd);           
                        }
                }
        }
        free(table->chain); 
        free(table); 
        return NULL; 
}

/**
 * @brief                       Creates an entry.
 * 
 * @return struct r2_entry*     Returns an empty entry, else NULL.
 */
static struct r2_entry* r2_create_entry()
{
        struct r2_entry *entry = malloc(sizeof(struct r2_entry));
        if(entry != NULL){
                entry->data     = NULL; 
                entry->key      = NULL; 
                entry->length   = 0;
        }
        return entry;
}


/**
 * @brief                       Creates an empty node for the chain.
 * 
 * @return struct r2_cnode*     Returns an empty node, else NULL.
 */
static struct r2_cnode* r2_create_cnode()
{
        struct r2_cnode* cnode = malloc(sizeof(struct r2_cnode));
        if(cnode != NULL){
                cnode->entry = NULL; 
                cnode->next  = NULL;
                cnode->prev  = NULL;
        }
        return cnode;
}


/**
 * @brief               Determines the next size of the hash table.
 *                     
 *
 * @param tsize         Hash table size.
 * @param op            When op == 1, we grow the table, op == 2  we shrink the table, else nothing.
 * @param prime         When prime == 1, use prime sizes for hash table, else size is a power of two.
 * @return r2_uint64    Returns the new size of the hash table.
 */
static r2_uint64 r2_get_tsize(r2_uint64 tsize, r2_int16 op, r2_int16 prime)
{
        const r2_uint64 PRIMES [] =   {53, 97, 193, 389, 769, 1543,3079, 6151, 12289, 24593,
                                        49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
                                        12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741,
                                        2147483647};
        
        r2_uint64 nsize = tsize; /*new table size*/
        if(prime == 1){
                /**
                 * Find the next prime.
                 */
                        nsize = PRIMES[0];
                        r2_int16 i = 0; 
                        for(;PRIMES[i] < tsize; ++i, nsize = PRIMES[i]);
                        
                        switch(op){
                                case 1: 
                                        if(nsize != PRIMES[26])
                                                nsize = PRIMES[i + 1];    
                                break; 
                                case 2:
                                        if(i > 0)
                                                nsize = PRIMES[i - 1]; 
                                break;
                        }
 
        }else{
                nsize = tsize == 0? 2 : tsize;
                switch(op){
                                case 1: 
                                        nsize = nsize << 1; 
                                        /*checking for overflow*/
                                        if(nsize  < tsize)
                                               nsize = tsize;  
                                break; 
                                case 2:
                                        if(nsize != 2)
                                                nsize = nsize >> 1;  
                                break;
                }           
        } 
        return nsize;
}

/**
 * @brief                               Puts a key with associated data into the table.
 * 
 * @param table                         Hash Table.
 * @param key                           Key.
 * @param data                          Data.
 * @param length                        Key Length.
 * @return struct r2_chaintable*        Returns hash table.
 */
struct r2_chaintable* r2_chaintable_put(struct r2_chaintable *table, r2_uc *key, void *data, r2_uint64 length)
{
        r2_uint64 hash = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        struct r2_cnode *node = r2_chain_search(&table->chain[hash], key, length, table->kcmp); 

        if(node == NULL){
                if((table->nsize/table->tsize) >= .75){
                        table   = r2_chaintable_resize(table, 1);
                        hash    = table->hf(key, length, table->tsize);
                }
                r2_chain_insert(&table->chain[hash], key, data, hash,length);
                table->nsize++;
        }else{
                node->entry->key        = key;
                node->entry->data       = data;
                node->entry->length     = length;
        }

        return table;
}


/**
 * @brief                               Locates key in hash table.
 * 
 * @param table                         Hash Table.
 * @param key                           Key.
 * @param length                        Key length.
 * @param entry                         Stores the entry value found.
 * @return struct r2_chaintable*        Returns hash table.
 */
struct r2_chaintable* r2_chaintable_get(struct r2_chaintable *table, r2_uc *key, r2_uint64 length, struct r2_entry *entry)
{       
        r2_uint64 hash = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        struct r2_cnode *node = r2_chain_search(&table->chain[hash], key, length, table->kcmp); 
        if(node != NULL)
                *entry = *(node->entry);
        
        return table;
}

/**
 * @brief                               Removes key and associated data from the hash table.
 * 
 * @param table                         Hash table.
 * @param key                           Key.
 * @param length                        Key length.
 * @return struct r2_chaintable*        Returns hash table.
 */
struct r2_chaintable* r2_chaintable_del(struct r2_chaintable *table, r2_uc *key, r2_uint64 length)
{
        r2_uint64 hash = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        struct r2_cnode *node = r2_chain_search(&table->chain[hash], key, length, table->kcmp); 

        if(node != NULL){
                r2_chain_remove(&table->chain[hash], 
                                node,
                                table->fk, 
                                table->fd,
                                table->kcmp);
                --table->nsize;
                if(table->nsize > 0 && table->nsize <= (table->tsize / 8))
                        table = r2_chaintable_resize(table, 2);
        }
        
        return table; 
}



/**
 * @brief                               Resizes chain hash table.
 *      
 * @param table                         Hash table.
 * @param op                            op == 1 we grow the table, else op == 2 we shrink the table.
 * @return struct r2_chaintable*        Returns hash table.
 */
static struct r2_chaintable *r2_chaintable_resize(struct r2_chaintable *table, r2_uint16 op)
{
        r2_uint64 tsize = r2_get_tsize(table->tsize, op, table->prime);
        if(tsize != table->tsize){
                struct r2_chain *ntable =  malloc(sizeof(struct r2_chain) * tsize);
                if(ntable != NULL){
                        for(r2_uint64 i = 0; i < tsize; ++i){
                                ntable[i].head  = NULL; 
                                ntable[i].tail  = NULL; 
                                ntable[i].csize = 0;
                        }
                        struct r2_chain chain;
                        struct r2_cnode *node = NULL;
                        r2_uint64 hash = 0;
                        for(r2_uint64 i = 0; i < table->tsize; ++i){
                                chain = table->chain[i]; 
                                node = chain.head; 
                                while(node != NULL){
                                        hash = table->hf(node->entry->key, node->entry->length, tsize);
                                        r2_chain_insert(&ntable[hash], node->entry->key, node->entry->data, hash, node->entry->length);
                                        node   = node->next;
                                }
                        }
                        free(table->chain);
                        table->chain = ntable; 
                        table->tsize = tsize;
                }
        }
        
        return table;
}


/**
 * @brief                               Creates a hash table that uses open addressing for collision resolutions. 
 *                                      It uses robinhood probing to resolve ollisions. For more information on 
 *                                      robinhood hashing see Robin Hood Hashing Pedro Celis.
 *                               
 * 
 * @param hf                            Hash function.
 * @param prime                         When prime == 1, hash table size is prime, else hash table size is a power of two.
 * @param psl                           Probe sequence length.
 * @param tsize                         Hash table size.
 * @param kcmp                          A callback comparison function to compare keys.
 * @param dcmp                          A callback comparison function to compare data.
 * @param kcpy                          A callback function to copy keys.
 * @param dcpy                          A callback function to copy values.
 * @param fk                            A callback function that releases memory used by key.
 * @param fd                            A callback function that releases memory used by data.
 * @return struct r2_robintable*        Returns empty hash table, else NULL.
 */
struct r2_robintable* r2_create_robintable(r2_int16 hf, r2_int16 prime, r2_uint64 psl, r2_uint64 tsize, r2_cmp kcmp, r2_cmp dcmp, r2_cpy kcpy, r2_cpy dcpy, r2_fk fk, r2_fd fd)
{
        r2_hashfunc hfs[] = {
                r2_hash_wee,
                r2_hash_knuth,
                r2_hash_fnv,
                r2_hash_dbj
        }; 

        struct r2_robintable* table  = malloc(sizeof(struct r2_robintable)); 
        if(table != NULL){
                tsize = tsize != 0 && prime != 1? tsize : r2_get_tsize(tsize, 1, prime); 
                table->cells = malloc(sizeof(struct r2_robinentry*) * tsize); 
                if(table->cells != NULL){
                        table->nsize    = 0; 
                        table->tsize    = tsize;
                        table->psl      = psl == 0? PSL: psl; 
                        table->hf       = hfs[hf]; 
                        table->prime    = prime; 
                        table->kcmp     = kcmp; 
                        table->dcmp     = dcmp; 
                        table->kcpy     = kcpy; 
                        table->dcpy     = dcpy; 
                        table->fk       = fk; 
                        table->fd       = fd; 
                        for(r2_uint64 i = 0; i < table->tsize; ++i)
                                table->cells[i] = NULL;   
                }else{
                        free(table); 
                        table = NULL; 
                }
        }

        return table;
}


/**
 * @brief                               Puts a key with associated data into the table.
 * 
 * @param table                         Hash Table.
 * @param key                           Key.
 * @param data                          Data.
 * @param length                        Key Length.
 * @return struct r2_robintable*        Returns hash table.
 */
struct r2_robintable* r2_robintable_put(struct r2_robintable *table, r2_uc *key, void *data, r2_uint64 length)
{
        r2_uint64 hash = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        r2_uint64 psl  = 0; 
        struct r2_key k = {.key = key, .len = length};
        struct r2_key j;
        struct r2_robinentry *pos = NULL;
        struct r2_robinentry *rentry = malloc(sizeof(struct r2_robinentry));

        if(rentry != NULL && table->tsize != table->nsize){
                /**
                 * Key and associated data that will be inserted.
                 * 
                 */
                rentry->entry.key       = key; 
                rentry->entry.data      = data;
                rentry->entry.length    = length; 
                rentry->hash            = hash;
                
                /**
                 * Current position in the table is always the hash + psl. 
                 * If the current pos == NULL, then we can insert else we
                 * continue probing for a record that violates the robin hood 
                 * heuristic. In the robinhood heuristic we're scanning the table 
                 * looking for a record P where its psl is smaller than the current psl.
                 * Once found, record P gives up its spot to rentry and rentry becomes the new
                 * record P. Record P continues probing to find the next appropriate position in the table. 
                 * We continue this displacement until we hit a null which means no more displacement
                 * can happen. We just take the spot of null since no element exists there.
                 * 
                 */
                pos = table->cells[hash + psl]; 
                
                while(pos != NULL){

                        j.key = pos->entry.key; 
                        j.len = pos->entry.length; 
                        
                        /*Handles duplicate*/
                        if(table->kcmp(&k, &j) == 0){
                                pos->entry.key = key; 
                                pos->entry.data = data;
                                free(rentry);
                                return table; 
                        }
                                

                        if(psl > pos->psl){
                                rentry->psl  = psl; 
                                table->cells[(hash + psl) % table->tsize] = rentry; 
                                rentry = pos;
                                psl  = rentry->psl; 
                                hash = rentry->hash;
                        }

                        ++psl;
                        pos = table->cells[(hash + psl) % table->tsize]; 
                }

                rentry->psl  = psl; 
                table->cells[(hash + psl) % table->tsize] = rentry; 
                ++table->nsize;

                if((table->nsize / table->tsize) > .50)
                        r2_robintable_resize(table, 1);
        }else{
                perror("Table full or out of memory");
        }
        
        return table; 
}

/**
 * @brief                               Locates key in hash table.
 * 
 * @param table                         Hash Table.
 * @param key                           Key.
 * @param length                        Key length.
 * @param entry                         Stores the entry value found.
 * @return struct r2_robintable*        Returns hash table.
 */
struct r2_robintable* r2_robintable_get(struct r2_robintable *table, r2_uc *key,  r2_uint64 length, struct r2_entry *entry)
{
        r2_uint64 hash = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        struct r2_key k = {.key = key, .len = length}; 
        struct r2_key j;
        r2_uint64 psl = 0; 
        while(table->cells[hash] != NULL){
                j.key = table->cells[hash]->entry.key;
                j.len = table->cells[hash]->entry.length;
                if(table->kcmp(&k, &j) == 0){
                        *entry = table->cells[hash]->entry;
                        break;
                }

                if(psl > table->cells[hash]->psl)
                        break;
                ++psl;
                hash  = (hash + 1) % table->tsize;
        }


        return table;
}

/**
 * @brief                               Removes key and associated data from the hash table.
 *                                      Uses backward shifting to maintain table.
 * 
 * @param table                         Hash table.
 * @param key                           Key.
 * @param length                        Key length.
 * @return struct r2_robintable*        Returns hash table.
 */
struct r2_robintable* r2_robintable_del(struct r2_robintable *table, r2_uc *key, r2_uint64 length)
{
        r2_uint64 hash  = table->hf(key, length, table->tsize);
        assert(hash < table->tsize);
        r2_uint64 psl   = 0; 
        r2_uint16 found = FALSE;
        struct r2_key k = {.key = key, .len = length}; 
        struct r2_key j;
        struct r2_robinentry *entry = table->cells[hash];
        
        while(entry != NULL){
                j.key = entry->entry.key; 
                j.len = entry->entry.length;
                if(table->kcmp(&k, &j) == 0){
                        found = TRUE;
                        r2_free_robinentry(entry, table->fk, table->fd);
                        table->cells[hash] = NULL;
                        --table->nsize;
                        break;
                }
                        
                if(psl > entry->psl)
                        break;

                hash  = (hash + 1) % table->tsize;
                entry = table->cells[hash];
                ++psl;     
        }
        
        /*Perform backward shifting*/
        if(found == TRUE){
                r2_uint16 RESIZE = FALSE;
                if(table->nsize > 0 && (table->nsize < table->tsize / 8))
                        RESIZE = r2_robintable_resize(table, 2);
                        
                if(RESIZE == FALSE){
                        entry =  table->cells[(hash + 1) % table->tsize]; 
                        while(entry != NULL && entry->psl != 0){
                                --entry->psl;
                                table->cells[hash] = entry; 
                                hash = (hash + 1) % table->tsize;
                                
                                /*Stopping out of bounds error.*/
                                entry = table->cells[(hash + 1) % table->tsize];
                        }

                        table->cells[hash] = NULL;
                }
        }
        
        return table; 
}


/**
 * @brief                               Destroys hash table.
 * 
 * @param table                         Hash table.
 * @return struct r2_robintable*        Returns NULL whenever hash table is destroyed properly.
 */
struct r2_robintable* r2_destroy_robintable(struct r2_robintable *table)
{
        for(r2_uint64 i = 0; i < table->tsize; ++i){
                if(table->cells[i] != NULL)
                        r2_free_robinentry(table->cells[i], table->fk, table->fd);
        }

        free(table->cells);
        free(table); 
        return NULL;
}


/**
 * @brief                               Resizes an hash table.
 *      
 * @param table                         Hash table.
 * @param op                            op == 1 we grow the table, else op == 2 we shrink the table
 * @return r2_uint16                    Returns TRUE if resize was a success, else FALSE.
 */
static r2_uint16  r2_robintable_resize(struct r2_robintable *table, r2_uint16 op)
{
        r2_uint16 SUCCESS = FALSE;
        r2_uint64 tsize = r2_get_tsize(table->tsize, op, table->prime);
        if(table->tsize != tsize){
                struct r2_robinentry **ntable = malloc(sizeof(struct r2_robinentry *) * tsize); 
                struct r2_robinentry **cells  = table->cells;
                r2_uint64 osize = table->tsize;
                if(ntable != NULL){
                        for(r2_uint64 i = 0; i < tsize; ++i)
                                ntable[i] = NULL; 
                        
                        
                        table->cells = ntable;
                        table->nsize = 0;  
                        table->tsize = tsize;
                        for(r2_uint64 i = 0; i < osize; ++i){
                                if(cells[i] != NULL){
                                        table = r2_robintable_put(table, cells[i]->entry.key, cells[i]->entry.data, cells[i]->entry.length);
                                }
                        }  

                        free(cells); 
                        SUCCESS = TRUE; 
                }
        }

        return SUCCESS;
}

/**
 * @brief                  Hashes a string using the common DBJ method.
 * 
 * @param key              Key.
 * @param length           Length.
 * @param tsize            Table size.
 * @return r2_uint64       Returns pos in table.
 */
r2_uint64 r2_hash_dbj(const unsigned char *key, r2_uint64 length, r2_uint64 tsize)
{
        r2_uint64 hash = 0; 
        for(r2_uint64 i = 0; i < length; ++i)
                hash = (101 * hash + key[i]) % tsize; 

        return hash % tsize;
}

/**
 * @brief                Hashes a string using the method in TAOCP.
 * 
 * @param key            Key.
 * @param length         Length.
 * @param tsize          Table size.
 * @return r2_uint64     Return the pos in table.
 */
r2_uint64 r2_hash_knuth(const unsigned char *key, r2_uint64 length, r2_uint64 tsize)
{
        /**
         * Combines DBJ and Knuth multiplicative method along with FNV Primes.
         * Romario Newell hash.
         */
        r2_uint64  W    = 4096;
        r2_uint64  A    = 1099511628211;
        r2_uint64  K    = 0;
        r2_uint64  C    = 0;
        r2_ldbl    P    = (r2_ldbl)18446744073709551557.00;
        for(r2_uint64 i = 0; i < length; ++i){
                C = (((key[i] << 5) | (key[i] >> 3)) << 2) ^ (((key[i] >> 4) | (key[i]) << 3) << 5);
                K = ((K*A/W) + key[i] + C * 16777619);
                K = (K << 7) ^ (K >> 25); 
                K = (K >> 47) | (K << 17);
                K = K % (r2_uint64 )14695981039346656037UL;
        }
               
        
        r2_uint64 B  = (K*A/P);
        r2_ldbl hash = ((K*A/P)-B)*tsize;
        
        return  hash;
}

/**
 * @brief                Hashes a string using the method using FNV hash.
 * 
 * @param key            Key.
 * @param length         Length.
 * @param tsize          Table size.
 * @return r2_uint64     Return the pos in table.
 */
r2_uint64 r2_hash_fnv(const unsigned char *key, r2_uint64 length, r2_uint64 tsize)
{
        r2_uint64 hash  = (r2_uint64)14695981039346656037UL;
        r2_uint64 prime = 1099511628211;

       for(r2_uint64 i = 0; i < length; ++i){
                hash =  hash ^ key[i]; 
                hash =  hash * prime;
        }

        return hash % tsize;
}

/**
 * @brief                Hashes a string using WEE hash found in CLRS.
 * 
 * @param key            Key.
 * @param length         Key length.
 * @param tsize          Table size.
 * @return r2_uint64     Return the pos in table.
 */
r2_uint64 r2_hash_wee(const unsigned char *key, r2_uint64 length, r2_uint64 tsize)
{
        const r2_uint16 NWORD   = WORD * 8;/*word size in bits.*/
        r2_uint64 nbits         = length * 8;/*number of bits in key.*/
        r2_uint64 cbits         = 0;/*bit counter*/
        r2_uc abits[WORD]       = {0};/*groups the bits into groups of WORD bits*/
        r2_uint32 hash32        = ((*key << 5) | (*key >> 3)) ^ ((*key << 7) | (*key >> 25)) * 101 - 1; 
        r2_uint64 hash64        = (r2_uint64 )(((*key << 47) | (*key >> 17)) ^ ((*key << 23) | (*key >> 19)) * 101 -1);
        r2_uint32 A             = 2*nbits + 16777619;
        r2_uint64 pos           = 0;
        r2_uint64 i = 0;
        do{
                cbits += 8;   
                if(i < length)
                        abits[pos++] = key[i++];
                else{
                        for(;(cbits % NWORD) != 0; ++pos, cbits += 8)
                                abits[pos] = 0;               
                }
                                        
                if(cbits % NWORD == 0){
                        switch(NWORD){
                                case 64: 
                                        hash64 = (*(r2_uint64 *)abits) + hash64;
                                        hash64 = (2*hash64*hash64 + A*hash64) % (1 << (NWORD -1));
                                        hash64 = ((hash64) >> (NWORD/2)) + ((hash64) << (NWORD/2));
                                break; 
                                case 32:
                                        hash32 = (*(r2_uint32 *)abits) + hash32;
                                        hash32 = (2*hash32*hash32 + A*hash32) % (1 << (NWORD -1));
                                        hash32 = ((hash32) >> (NWORD/2)) + ((hash32) << (NWORD/2));
                                break;
                        }
                        pos = 0;
                }
        }while(cbits <= nbits);
        
        return (NWORD == 64? hash64 : hash32) % tsize;
}

/**
 * @brief               Inserts a key value pair into our chain.
 * 
 * @param chain         Chain.
 * @param key           Key.
 * @param data          Data.
 * @param hash          Hash.
 * @param length        Key length.
 */
static void r2_chain_insert(struct r2_chain *chain, r2_uc *key,void *data, r2_uint64 hash,r2_uint64 length)
{
        struct r2_entry *entry = NULL; 
        struct r2_cnode *node  = NULL; 

        entry = r2_create_entry(); 
        if(entry != NULL){
                node = r2_create_cnode(); 
                if(node != NULL){
                        entry->key    = key;
                        entry->data   = data; 
                        entry->length = length;
                        node->entry   = entry; 
                        node->hash    = hash;

                        /*Inserting into list*/
                        if(chain->head == NULL && chain->tail == NULL)
                                chain->head = node;
                        else
                                chain->tail->next = node;

                        node->prev  = chain->tail; 
                        chain->tail = node;
                        ++chain->csize;
                }else free(entry); 
        }
}

/**
 * @brief                       Searches for a key in the chain.
 * 
 * @param chain                 Chain.
 * @param key                   Key.
 * @param length                Key length.
 * @param cmp                   A callback comparison function.
 * @return struct r2_cnode*     Returns the node containing the key, else NULL.
 */
static struct r2_cnode *r2_chain_search(const struct r2_chain *chain, r2_uc *key, r2_uint64 length, r2_cmp cmp)
{
        struct r2_cnode *head   = chain->head; 
        struct r2_key k = {.key = key, .len = length}; 
        struct r2_key j = {.key = NULL, .len = 0};
        while(head != NULL){
                j.key = head->entry->key; 
                j.len = head->entry->length;
                if(cmp(&k, &j) == 0)
                        break;
                head = head->next; 
        }
        return head;
}


/**
 * @brief               Removes a key from the data.
 * 
 * @param chain         Chain.
 * @param node          Node.
 * @param freekey       A callback function that releases memory used by the key.
 * @param freedata      A callback function that releases memory used by data.
 * @param r2_cmp        A callback function used to compare keys.
 */
static void r2_chain_remove(struct r2_chain *chain, struct r2_cnode *node, r2_fk freekey, r2_fd freedata, r2_cmp cmp)
{
        if(node != NULL){
                if(node == chain->head && node == chain->tail){
                        chain->head = NULL; 
                        chain->tail = NULL; 
                }else if(chain->head == node){
                        chain->head = node->next; 
                        chain->head->prev = NULL;
                }else if(chain->tail == node){
                        chain->tail = node->prev; 
                        chain->tail->next = NULL; 
                }else{
                        node->prev->next = node->next; 
                        node->next->prev = node->prev;
                }
                --chain->csize;
                r2_freenode(node, freekey, freedata); 
        }
}

/**
 * @brief               Free memory used by an entry
 * 
 * @param node          Node.
 * @param freekey       A callback function that releases memory used by the key.
 * @param freedata      A callback function that releases memory used by data.
 */
static void r2_freenode(struct r2_cnode *node,  r2_fk freekey, r2_fd freedata)
{
        if(freekey != NULL)
                freekey(node->entry->key);

        if(freedata != NULL)
                freedata(node->entry->data);
        
        free(node->entry);
        free(node);
}

static void r2_free_robinentry(struct r2_robinentry *entry, r2_fk fk, r2_fd fd)
{
        if(fk != NULL)
                fk(entry->entry.key); 

        if(fd != NULL)
                fd(entry->entry.data); 


        free(entry);
}