#include <stdlib.h>
#include <assert.h>
#include "r2_trie.h"
#include <string.h>
static struct r2_trienode* r2_create_trienode();

/**
 * @brief                       Creates and returns an emtpy trie.
 * 
 * @param fk                    A callback function to release memory occupied by key.              
 * @param fd                    A callback function to release memory occupied by data.
 * @return struct r2_trie*      Returns an empty trie, else NULL.      
 */
struct r2_trie* r2_create_trie(r2_fk fk, r2_fd fd)
{
        struct r2_trie *trie = malloc(sizeof(struct r2_trie));
        if(trie != NULL){
                trie->fk = fk; 
                trie->fd = fd; 
                trie->root = r2_create_trienode(); 
                trie->nelems = 0;
                if(trie->root == NULL){
                        free(trie);
                        trie = NULL;
                }
        }
        return trie;
}

/**
 * @brief                       Destroys a trie.
 *                              
 *                              We only free the data associated with each key.                             
 * 
 * @param trie                  Trie.
 * @return struct r2_trie*      Returns NULL when successfully.
 */
struct r2_trie* r2_destroy_trie(const struct r2_trie *trie)
{
        assert(trie != NULL);
        r2_int64 pos = 0;
        struct r2_trienode *root   = trie->root; 
        struct r2_trienode *parent = NULL; 
        root->tcount = 0;
        for(;;){
                for(pos = root->tcount; pos < ALPHABET; ++pos){
                        if(root->keys[pos] != NULL){
                                root->tcount = pos;/*setting parent position*/ 
                                parent = root;/*move down to a valid child*/
                                root = root->keys[pos];
                                break;
                        }
                }
                /**
                 * We have deleted all children. 
                 * It's now time to delete root and move back up to parent.
                 */
                if(pos == ALPHABET){
                        /*Edge case*/
                        if(root == trie->root)
                               break;
                        
                        if(trie->fd != NULL)
                                trie->fd(root->data); 
                        free(root); 
                        parent->keys[parent->tcount] = NULL;
                        root   = parent;
                        parent = root->parent; 
                }else root->tcount = 0;
        }

        free(trie->root); 
        free((struct r2_trie *)trie);
        return NULL;
}


/**
 * @brief                       Inserts a key along with data into trie.
 * 
 * @param trie                  Trie. 
 * @param key                   Key.
 * @param len                   Length. 
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon successfully insertion, else FALSE.
 */
r2_uint16 r2_trie_insert(struct r2_trie *trie, r2_uc *key, r2_uint64 len, void *data)
{
        assert(trie != NULL &&  key  != NULL &&  data != NULL && len > 0); 
        r2_uint16 SUCCESS = TRUE;
        struct r2_trienode *root     = trie->root;
        struct r2_trienode *parent   = NULL;
        r2_int64 pos = 0;
        
        /*Find the position in the tree to insert*/
        for(;root->keys[key[pos]] != NULL && pos < len; ++pos)
                root = root->keys[key[pos]];

        /*Remember this position in the trie in case we have to back up.*/
        r2_int64 start = pos;

        /*Insert key into trie*/
        for(;pos != len; ++pos){
                root->keys[key[pos]] = r2_create_trienode();
                if(root->keys[key[pos]] == NULL){
                        SUCCESS = FALSE;
                        break;
                }
                ++root->tcount;
                parent = root;       
                root   = root->keys[key[pos]]; 
                root->parent = parent;
        }

        /*Updates number of elements in the trie*/
        if(root->data == NULL)
                ++trie->nelems;
        
        root->data = data;
        if(SUCCESS == FALSE){
                /**
                 * We failed to insert the key into the trie and 
                 * need to walk back up the trie removing nodes we inserted. 
                 * We stop walking the trie once we reach back to our starting position. 
                 * 
                 */
                --trie->nelems;
                for(r2_int64 i = pos-1; i >= start; --i, parent = parent->parent)
                        free(parent->keys[key[pos]]);
        }
        return SUCCESS; 
}

 
/**
 * @brief               Finds a key in a trie.
 * 
 * @param trie          Trie. 
 * @param key           Key.
 * @param len           Length. 
 * @return void*        Returns the data item associated with key, else NULL.
 */
void* r2_trie_search(struct r2_trie *trie, r2_uc *key, r2_uint64 len)
{
        assert(trie != NULL && key != NULL && len > 0); 
        struct r2_trienode *root = trie->root;
        for(r2_uint64 pos = 0; root != NULL && pos != len; ++pos)
                root = root->keys[key[pos]];

       return root == NULL? root : root->data; 
}

/**
 * @brief               Deletes a key from the trie.
 * 
 * @param trie          Trie. 
 * @param key           Key.
 * @param len           Len.
 * @return r2_uint16    Returns TRUE if we successfully deleted the key, else FALSE. 
 */
r2_uint16 r2_trie_delete(struct r2_trie *trie, r2_uc *key, r2_uint64 len)
{
        r2_uint16  SUCCESS = FALSE;
        assert(trie != NULL && key != NULL && len > 0); 
        struct r2_trienode *root = trie->root;
        r2_int64 pos = 0;
        for(;root != NULL && pos != len; ++pos)
                root = root->keys[key[pos]];
        
        if(root != NULL && root->data != NULL){   
                if(trie->fd != NULL)
                        trie->fd(root->data);

                root->data = NULL;
                /*Walking up the trie to our starting position*/
                struct r2_trienode *parent = root->parent;
                --pos;
                for(;pos >= 0 && root->tcount == 0; --pos, parent = parent->parent){  
                        free(parent->keys[key[pos]]);
                        parent->keys[key[pos]] = NULL;
                        --parent->tcount;
                        root = parent;     
                }
                        
                if(trie->fk != NULL)
                        trie->fk(key); 
                --trie->nelems;
                SUCCESS = TRUE;
        }
        return SUCCESS;
}

/**
 * @brief               Finds the longest proper prefix of key.
 *                      N.B Caller is responsible for freeing memory.
 * @param trie          Trie.
 * @param key           Key. 
 * @param len           Length.
 * @return char*        Returns longest prefix of key, else NULL.
 */
char* r2_trie_longest_prefix(struct r2_trie *trie, r2_uc *key, r2_uint64 len)
{
        r2_uc *prefix = NULL; 
        assert(trie != NULL && key != NULL && len > 0); 
        struct r2_trienode *root = trie->root;

        r2_int64 pos;
        for(pos = 0; root != NULL && pos != len; ++pos)
                root = root->keys[key[pos]];
        
        if(root != NULL && root->data != NULL){
                struct r2_trienode *parent = NULL;
                do{
                        --pos;
                        parent = root->parent;
                        root   = parent;
                }while(pos != 0 && parent->data == NULL);

                if(pos != 0){
                        prefix = malloc(sizeof(r2_uc) *pos);
                        if(prefix != NULL){
                                strncpy(prefix, key, pos);
                                prefix[pos] = '\0';
                        }
                }
        }

        return prefix; 
}

static struct r2_trienode* r2_create_trienode()
{
        struct r2_trienode *node = calloc(1, sizeof(struct r2_trienode));
        return node;
}