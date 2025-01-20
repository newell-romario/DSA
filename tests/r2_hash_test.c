#include "r2_hash_test.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strings [] = {
"Romario", 
"Newell", 
"Computer Science", 
"Software",
"Developer",
"programming",
"1997",
"10",
"20",
"interesting",
"hashing",
"hash",
"hashes",
"ashes",
"unique",
"come closer",
"double up"
};

static r2_int16 cmp(const void *a, const void *b);
static r2_int16 cmp2(const void *a, const void *b);


/**
 * @brief       Tests create functionality for hash table.
 * 
 */
static void test_r2_create_chaintable()
{
        struct r2_chaintable * table  = r2_create_chaintable(0, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        assert(table != NULL);
        assert(table->chain != NULL); 
        assert(table->hf == r2_hash_wee); 
        assert(table->kcmp == cmp); 
        assert(table->dcmp == cmp); 
        assert(table->nsize == 0); 
        assert(table->tsize == 256 || table->tsize == 251 || table->tsize == 97); 
        assert(table->fk == NULL); 
        assert(table->fd == NULL);
        assert(table->dcpy == NULL); 
        assert(table->kcpy == NULL);
        assert(table->prime == 1);
        r2_destroy_chaintable(table);
}

/**
 * @brief       Tests destroy functionality for hash table.
 * 
 */
static void test_r2_destroy_chaintable()
{
        struct r2_chaintable * table  = r2_create_chaintable(3, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        assert(r2_destroy_chaintable(table) == NULL); 
        table  = r2_create_chaintable(0, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0 ; i < 17; ++i)
                table = r2_chaintable_put(table, strings[i], strings[i], strlen(strings[i]));
        
        assert(r2_destroy_chaintable(table) == NULL); 
}

/**
 * @brief       Tests the hash table put functionality.
 * 
 */
static void test_r2_chaintable_put()
{
        struct r2_chaintable * table  = r2_create_chaintable(0, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0 ; i < 17; ++i)
                table = r2_chaintable_put(table, strings[i], strings[i], strlen(strings[i]));
        
        assert(table->nsize == 17); 
        struct r2_entry entry = {.data = NULL, .key = NULL, .length = 0}; 
        table = r2_chaintable_get(table, strings[0],strlen(strings[0]), &entry);
        assert(strcmp(entry.key, strings[0]) == 0); 
        assert(strlen(entry.key) ==  strlen(strings[0])); 
        assert(strcmp(entry.data, strings[0]) == 0);
        assert(strlen(entry.data) ==  strlen(strings[0])); 
        r2_destroy_chaintable(table);
}

/**
 * @brief       Tests the hash table get functionality.
 * 
 */
static void test_r2_chaintable_get()
{
        struct r2_chaintable * table  = r2_create_chaintable(0, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        struct r2_entry entry;
        for(r2_uint64 i = 0 ; i < 17; ++i){
                entry.key  = 0;
                entry.data = NULL; 
                entry.length = 0;
                table = r2_chaintable_put(table, strings[i], strings[i], strlen(strings[i]));
                table = r2_chaintable_get(table, strings[i],strlen(strings[i]), &entry);
                assert(strcmp(entry.key, strings[i]) == 0); 
                assert(strlen(entry.key) ==  strlen(strings[i])); 
                assert(strcmp(entry.data, strings[i]) == 0);
                assert(strlen(entry.data) ==  strlen(strings[i])); 
        }
                
        r2_destroy_chaintable(table);
}

/**
 * @brief       Tests the del functionality of hash table.
 * 
 */
static void test_r2_chaintable_del()
{
        struct r2_chaintable * table  = r2_create_chaintable(0, 1, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0 ; i < 17; ++i)
                table = r2_chaintable_put(table, strings[i], strings[i], strlen(strings[i]));

        table = r2_chaintable_del(table, strings[0], strlen(strings[0])); 
        assert(table->nsize == 16); 
        struct r2_entry entry = {.data = NULL, .key = NULL, .length = 0}; 
        table = r2_chaintable_get(table, strings[0], strlen(strings[0]), &entry);
        assert(entry.data == NULL); 
        assert(entry.key == NULL); 
        assert(entry.length == 0);
        for(r2_uint64 i = 0 ; i < 17; ++i)
                table = r2_chaintable_del(table, strings[i], strlen(strings[i]));

        r2_destroy_chaintable(table);
}

/**
 * @brief       Tests the create functionality for a hash table
 * 
 */
static void test_r2_create_robintable()
{
        struct r2_robintable *table = r2_create_robintable(0, 1, 0, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        assert(table->cells != NULL); 
        assert(table->hf == r2_hash_wee); 
        assert(table->prime == 1); 
        assert(table->psl == 4); 
        assert(table->nsize == 0);
        assert(table->tsize == 97); 
        assert(table->kcmp  == cmp);
        assert(table->dcmp  == cmp); 
        assert(table->kcpy  == NULL);
        assert(table->dcpy  == NULL); 
        assert(table->fk    == NULL); 
        assert(table->fd    == NULL);
      
        r2_destroy_robintable(table);
}

/**
 * @brief       Tests the put function for robinhood table
 * 
 */
static void test_r2_robintable_put()
{
        struct r2_robintable *table = r2_create_robintable(0, 1, 0, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 17; ++i)
                table = r2_robintable_put(table, strings[i], strings[i], strlen(strings[i]));
        
        assert(table->nsize == 17);
        struct r2_entry entry;
        entry.key  = NULL; 
        entry.data = NULL;
        entry.length = 0; 
        table = r2_robintable_get(table, strings[0], strlen(strings[0]), &entry);
        assert(strcmp(entry.key, strings[0]) == 0); 
        assert(strlen(entry.key) == strlen(strings[0]));
        r2_destroy_robintable(table);
}

/**
 * @brief       Tests the get functionality for robintable
 * 
 */
static void test_r2_robintable_get()
{
        struct r2_robintable *table = r2_create_robintable(0, 1, 0, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        struct r2_entry entry;
        entry.key  = NULL; 
        entry.data = NULL;
        entry.length = 0; 
        for(r2_uint64 i = 0; i < 17; ++i){
                table = r2_robintable_put(table, strings[i], strings[i], strlen(strings[i]));
                table = r2_robintable_get(table, strings[i], strlen(strings[i]), &entry);
                assert(strcmp(entry.key, strings[i]) == 0); 
                assert(strlen(entry.key) == strlen(strings[i]));
        }
               
        entry.key  = NULL; 
        entry.data = NULL;
        entry.length = 0; 
        table = r2_robintable_get(table, "Softwares", strlen("Softwares"), &entry);
        assert(entry.key == NULL); 
        assert(entry.data == NULL); 
        assert(entry.length == 0);

        r2_destroy_robintable(table);
}

/**
 * @brief Tests the delete functionality of robinhood hash table.
 * 
 */
static void test_r2_robintable_del()
{
        struct r2_robintable *table = r2_create_robintable(0, 1, 0, 0, cmp, cmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 17; ++i)
                table = r2_robintable_put(table, strings[i], strings[i], strlen(strings[i]));    

        for(r2_uint64 i = 0; i < 17; ++i)
                table = r2_robintable_del(table, strings[i], strlen(strings[i]));

       r2_destroy_robintable(table);
}
/**
 * @brief       Dump probe sequence length of each record to csv
 * 
 */
static void test_r2_robintable_psl(struct r2_robintable *table)
{
        FILE *results = fopen("robintable.csv", "w");
        char key[50];
        FILE *fp = fopen("../tests/words.txt", "r");
        r2_uint64 line = 0; 
        struct r2_key j;
        struct r2_key k;
        r2_uint64 length;
        r2_uint64 hash ;
        r2_uint64 psl;

        while(TRUE){
                if(fscanf(fp, "%s", key) != 1)
                        break;
    
                length =  strlen(key);
                hash  = table->hf(key, length, table->tsize);
                k.key = key; 
                k.len = length;
              
                psl = 0; 
                while(table->cells[hash] != NULL){
                        j.key = table->cells[hash]->entry.key;
                        j.len = table->cells[hash]->entry.length;
                        if(table->kcmp(&k, &j) == 0){
                               fprintf(results,"\n%lld", psl);
                                break;
                        }

                        if(psl > table->cells[hash]->psl)
                                break;
                        ++psl;
                        hash  = (hash + 1) % table->tsize;
                }
        }

        fclose(results); 
        fclose(fp);

}

static r2_int16 cmp2(const void *a, const void *b)
{
        const long long int *c = ((struct r2_key *)a)->key; 
        const long long int *d = ((struct r2_key *)b)->key; 

       if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else 
                return 1;
}


/**
 * @brief       Prints every element in a chain.
 * 
 * @param c     Chain.
 */
static void test_r2_print_chain(const struct r2_chain *c)
{
        struct r2_cnode *head = NULL; 

        if(c != NULL){
                head = c->head; 
                while(head != NULL)
                {
                        printf("%s ", head->entry->key); 
                        head = head->next;
                }
        }
}

/**
 * @brief       Prints every element in a chain.
 * 
 * @param c     Chain.
 */
static void test_r2_print_chain_int(const struct r2_chain *c)
{
        struct r2_cnode *head = NULL; 

        if(c != NULL){
                head = c->head; 
                while(head != NULL)
                {
                        printf("%lld ", *((r2_uint64 *)head->entry->key)); 
                        head = head->next;
                }
        }
}


/**
 * @brief                       Gets the longest chain in the hash table.
 * 
 * @param table                 Hash table
 * @return struct r2_chain*     Returns longest chain.
 */
static struct r2_chain* test_r2_longest_chain(struct r2_chaintable *table)
{
        r2_uint64 max   = 0; 
        r2_uint64 csize = 0;
        for(r2_uint64 i = 0; i < table->tsize; ++i){
                if(table->chain[i].csize > csize){
                        csize = table->chain[i].csize;
                        max = i;
                }
                        
        }

      return  &table->chain[max];
}


/**
 * @brief       Inserts 400k words into hash table.
 * 
 */
static void test_r2_chaintable_generate()
{
        printf("\n---------------------------------Hashing with separate chaining----------------------------------");
        struct r2_chaintable *table = r2_create_chaintable(1, 1, 0, cmp, cmp, NULL, NULL, free, NULL);
        char *key  = NULL; 
        r2_uint64 l = 70; 
        FILE *fp = fopen("../tests/words.txt", "r");
        r2_uint64 line = 0; 
        while(TRUE){
                key = malloc(sizeof(char) *l); 
                if(fscanf(fp, "%s", key) != 1)
                        break;
    
                printf("\n%d)(Key = %s,val = %s)", ++line, key, key);
                table = r2_chaintable_put(table, key, key, strlen(key));
        }
        r2_uint64 ncells = 0;
        r2_uint64 nentries = 0;
        for(r2_uint64 i = 0; i < table->tsize; ++i){
                if(table->chain[i].csize != 0){
                        ncells++; 
                        nentries += table->chain[i].csize;
                }
        }

        printf("\nAverage chain length: %lf", (double)nentries/ ncells);
        struct r2_chain *chain  = test_r2_longest_chain(table);
        printf("\nLongest chain: %d", chain->csize); 
        printf("\nLongest chain: ");
        test_r2_print_chain(test_r2_longest_chain(table));
        printf("\nFirst Index: %d", test_r2_chaintable_first_index(table));
        printf("\nLast Index: %d", test_r2_chaintable_last_index(table));
        test_r2_chaintable_print(table);

        struct r2_entry entry = {.data= NULL, .key = NULL, .length = 0};
        char del[70]; 
        rewind(fp);
        line = 0; 
        printf("\n---------------------------------------Deleting-------------------------------------------");
        while(table->nsize != 0){
                
                if(fscanf(fp, "%s", del) == 1){
                        entry.key = entry.data = NULL;
                        printf("\n%d)(Key = %s,val = %s)", ++line, del, del);
                        r2_chaintable_get(table, del, strlen(del), &entry);
                        assert(entry.key != NULL);
                        assert(strcmp(entry.key, del) == 0);
                        table = r2_chaintable_del(table, del, strlen(del));
                        entry.key = entry.data = NULL;
                        r2_chaintable_get(table, del, strlen(del), &entry);
                        assert(entry.key == NULL);
                }
                        
                
                
        }
        fclose(fp);
}

/**
 * @brief  Inserts 400k words into hash table
 * 
 */
static void test_r2_robintable_generate()
{
        printf("\n---------------------------------Hashing with open addressing(Robinhood)---------------------------------");
        struct r2_robintable *table = r2_create_robintable(2, 1, 0, 0, cmp, cmp, NULL, NULL, free,  NULL);
        char *key  = NULL; 
        r2_uint64 l = 70; 
        FILE * fp = fopen("../tests/words.txt", "r");
        r2_uint64 line = 0; 
        while(TRUE){
                key = malloc(sizeof(char) *l); 
                if(fscanf(fp, "%s", key) != 1)
                        break;

                printf("\n%d)(Key = %s,val = %s)", ++line, key, key);
                table = r2_robintable_put(table, key, key, strlen(key));
        }

        test_r2_robintable_psl(table);


        struct r2_entry entry = {.data= NULL, .key = NULL, .length = 0};
        char del[70]; 
        rewind(fp);
        while(table->nsize != 0){
                fscanf(fp, "%s", del);
                entry.key  = entry.data = NULL;
                r2_robintable_get(table, del, strlen(del), &entry); 
                assert(entry.key != NULL);
                assert(strcmp(entry.key, del) == 0);
                table = r2_robintable_del(table, del, strlen(del));
                entry.key  = entry.data = NULL;
                r2_robintable_get(table, del, strlen(del), &entry); 
                assert(entry.key == NULL);
        }
        fclose(fp);
}

/**
 * @brief               Sends the chain length to a file in cwd.
 * 
 * @param table         Hash table
 */
static void test_r2_chaintable_print(const struct r2_chaintable *table)
{
        FILE *fp = fopen("chaintable.csv", "w");
        struct r2_cnode *head = NULL;
        for(r2_uint64 i = 0; i < table->tsize; ++i){
                if(table->chain[i].csize != 0){
                        fprintf(fp, "\n%ld", table->chain[i].csize);
                }
        }

        fclose(fp);
}


static r2_uint64 test_r2_chaintable_first_index(const struct r2_chaintable *table)
{
  
        for(r2_uint64 i = 0; i < table->tsize; ++i)
                if(table->chain[i].csize != 0)
                        return i;
}

static  r2_uint64 test_r2_chaintable_last_index(const struct r2_chaintable *table)
{
        r2_uint64 max = 0;
        for(r2_uint64 i = 0; i < table->tsize; ++i)
                if(table->chain[i].csize != 0)
                        max = i; 
                
        return max;
}

static r2_int16 cmp(const void *a, const void *b)
{
        const struct r2_key *c = (struct r2_key *)a; 
        const struct r2_key *d = (struct r2_key *)b;
        return strcmp(c->key, d->key);
}







void test_r2_chaintable_run()
{
       
        test_r2_create_chaintable();
        test_r2_destroy_chaintable();
        test_r2_chaintable_put();
        test_r2_chaintable_get();
        test_r2_chaintable_del();
        test_r2_create_robintable();
        test_r2_robintable_put();
        test_r2_robintable_get();
        test_r2_robintable_del();
        test_r2_chaintable_generate();
        test_r2_robintable_generate();
}