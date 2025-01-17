#include "r2_btree.h"
#include "../src/r2_stack.h"
#include <stdlib.h>

/**********************File scope functions************************/
static r2_int64 r2_bsearch(void **, void *,r2_int64, r2_int64, r2_cmp); 
static void     r2_split_page(struct r2_btree*, struct r2_page*, r2_cmp);
static void     r2_page_insert_key(struct r2_page *, r2_int64, void *);
static r2_int64 r2_page_calc_size(const struct r2_page *);
static void     r2_page_delete_rebalance(struct r2_btree *, struct r2_page *);
static struct r2_page * r2_page_get_sibling(struct r2_page*, r2_cmp);
static struct r2_page*  r2_page_catenate(struct r2_btree *, struct r2_page *, struct r2_page *);
static void r2_page_underflow(struct r2_page *, struct r2_page *, r2_cmp);
static void r2_freepage(struct r2_page*,  r2_fk);
/**********************File scope functions************************/


/**
 * @brief                       Creates a B Tree of order M.
 *        
 * 
 * @param  order                Order.
 * @param  kcmp                 A comparison callback function.
 * @param  fk                   A callback function that release memory used by key.
 * @return struct r2_btree*     Returns a B Tree of order M, else NULL.
 */
struct r2_btree* r2_create_btree(r2_int64 order, r2_cmp kcmp, r2_fk fk)
{
        struct r2_btree *btree = NULL;
        if(order >= 2 && (order % 2 == 0)){
                btree = malloc(sizeof(struct r2_btree)); 
                if(btree != NULL){
                        btree->order    = order; 
                        btree->root     = NULL; 
                        btree->kcmp     = kcmp; 
                        btree->fk       = fk;
                        btree->ncount   = 0; 
                }       
        }  

        return btree;     
}

/**
 * @brief                       Creates a page.
 *                             
 * @param order                 Order.
 * @return struct r2_page*      Returns an empty page, else NULL.
 */
struct r2_page* r2_create_page(r2_int64 order)
{
        struct r2_page *page = malloc(sizeof(struct r2_page));
        if(page != NULL){
                page->indexes   = malloc(sizeof(void *) * (order + 1)); /*Extra key to help when splitting root*/
                page->children  = malloc(sizeof(void *) * (order + 2)); /*Extra child to help when splitting root*/
                if(page->indexes != NULL && page->children != NULL){
                        page->leaf      = TRUE; 
                        page->ncount    = 1; 
                        page->mkeys     = order;
                        page->nkeys     = 0;
                        page->parent    = NULL;
                        for(r2_int64 i = 0; i < page->mkeys + 1; ++i)
                                page->indexes[i] = NULL;  
                        
                        for(r2_int64 i = 0; i < page->mkeys + 2; ++i)
                                page->children[i] = NULL;
                }else{
                        free(page->children); 
                        free(page->indexes);
                        free(page);
                        page = NULL; 
                }       
        }

        return page;
}


/**
 * @brief                       Destroys a B Tree.
 * 
 * @param btree                 B Tree.
 * @return struct r2_btree*     Returns NULL if B Tree was destroyed properly.
 */
struct r2_btree* r2_destroy_btree(struct r2_btree *btree)
{
        struct r2_stack *stack = r2_create_stack(btree->kcmp, NULL, NULL);

        if(btree->root != NULL)
                stack = r2_stack_push(stack, btree->root); 

        struct r2_page * root = NULL;
        struct r2_stacknode *top = NULL; 
        while(r2_stack_empty(stack) != TRUE){
                top   = r2_stack_peek(stack); 
                root  = top->data;
                stack = r2_stack_pop(stack); 

                for(int i = 0; i < root->nkeys + 1; ++i)
                        if(root->children[i] != NULL)
                                stack = r2_stack_push(stack, root->children[i]);
                
                r2_freepage(root, btree->fk);
        }

        free(btree);
        return NULL;
}

/**
 * @brief                       Searches for a key in a B Tree.
 * 
 * @param btree                 B Tree.
 * @param key                   Key.
 * @return struct r2_page*      Returns the page containing the key, else NULL.
 */
struct r2_page* r2_btree_search(const struct r2_btree *btree, void *key)
{
        r2_int16 result = 0; 
        struct r2_page *page = btree->root; 
        while(page != NULL){
                result = r2_bsearch(page->indexes, key, 0, page->nkeys -1, btree->kcmp);
                if(result < page->nkeys && btree->kcmp(key, page->indexes[result]) == 0)
                        break;
                else 
                        page = page->children[result];
        }

        return page;
}


/**
 * @brief               Peforms a binary search on an array indexes.
 * 
 * @param indexes       An array of pointers to indexes.
 * @param key           Lookup key.
 * @param start         Start of array.
 * @param end           End of array.
 * @param cmp           Comparison function.
 * @return r2_int64     Returns the index the binary search ended at.
 */
static r2_int64 r2_bsearch(void **indexes, void *key,r2_int64 start, r2_int64 end, r2_cmp cmp)
{
        r2_int64 middle  = 0;
        r2_int16 result  = 0;  
        while(start <= end){
                middle = start + (end - start)/2;
                result = cmp(key, indexes[middle]);
                if(result == 0)
                        return middle;
                else if(result > 0)
                        start = middle + 1; 
                else 
                        end = middle - 1;
        }

        return end + 1;
}

/**
 * @brief                       Splits a page.  
 * 
 * @param page                  Page.
 */
static void r2_split_page(struct r2_btree *btree, struct r2_page *page, r2_cmp cmp)
{
        struct r2_page* parent  = page->parent;
        struct r2_page* sibling = r2_create_page(page->mkeys);

        if(sibling != NULL){
                r2_int64 median  = page->nkeys / 2;
                void *median_key = page->indexes[median];
                
                /**
                 * A sibling is either a leaf or internal node based on 
                 * the node it's split from.
                 */
                sibling->leaf = page->leaf;

                /**
                 * Copying keys from page to sibling.
                 */
                for(r2_int64 i = median + 1, j = 0; i < page->nkeys; ++i, ++j){
                        sibling->indexes[j] = page->indexes[i];
                        page->indexes[i]  = NULL;
                }

                /**
                 * Copying children as well.
                 */
                if(page->leaf != TRUE){
                         for(r2_int64 i = median + 1, j = 0; i < page->nkeys + 1; ++i, ++j){
                                sibling->children[j] = page->children[i]; 
                                page->children[i]->parent = sibling;
                                page->children[i] = NULL;       
                        }
                }
               

                page->indexes[median] = NULL;
                page->nkeys    = median;
                sibling->nkeys = median;
                 
                if(parent == NULL){
                        parent        = r2_create_page(page->mkeys);
                        btree->root   = parent;
                        page->parent  = parent;
                }

                if(parent != NULL){
                        /**
                         * Find the position where the median should be inserted in parent.
                         * 
                         */
                        r2_int64 index  = r2_bsearch(parent->indexes, median_key, 0, parent->nkeys -1, btree->kcmp);
                        r2_page_insert_key(parent, index, median_key);

                        /**
                         * Shift children to make room for sibling.
                         * 
                         */
                        for(r2_int64 i = parent->nkeys; i > index; --i)
                                parent->children[i] = parent->children[i -1];

                        parent->children[index + 1] = sibling; 
                        parent->children[index] = page;
                        parent->leaf = FALSE;

                }
                sibling->parent   = parent; 
                page->ncount      = r2_page_calc_size(page);
                sibling->ncount   = r2_page_calc_size(sibling);
        }
}

/**
 * @brief                       Inserts a key into a B Tree.

 * @param btree                 B Tree.
 * @param key                   Key.
 * @return struct r2_btree*     Returns B Tree.
 */
struct r2_btree* r2_btree_insert(struct r2_btree *btree, void *key)
{
        struct r2_page *page   = btree->root; 
        struct r2_page *parent = NULL;  
        r2_int64 index  = 0; 
        while(page != NULL){
                parent = page;
                index  = r2_bsearch(page->indexes, key, 0, page->nkeys - 1, btree->kcmp);
                if(index < page->nkeys && btree->kcmp(key, page->indexes[index]) == 0)
                        return btree; 
                        
                page = page->children[index];  
        }
        
        /**
         * Whenever parent is null this means the B Tree
         * is empty. We need to create the root of the B Tree.
         */
        if(parent == NULL){
                parent = r2_create_page(btree->order); 
                btree->root = parent;
        }

        r2_page_insert_key(parent, index, key);
        while(parent != NULL){
                 /*Page overflowed and needs to be corrected*/
                if(parent->nkeys > parent->mkeys){
                        r2_split_page(btree, parent, btree->kcmp);
                }
                parent->ncount = r2_page_calc_size(parent);
                parent = parent->parent; 
        }
        btree->ncount = r2_page_calc_size(btree->root);
        return btree; 
}

/**
 * @brief                       Deletes a key from a B Tree
 *                      
 * @param btree                 B Tree.
 * @param key                   Key.
 * @return struct r2_btree*     Returns B Tree.
 */
struct r2_btree* r2_btree_delete(struct r2_btree *btree, void *key)
{
        struct r2_page *page = r2_btree_search(btree, key); 
        if(page != NULL){
                struct r2_page *minimum = NULL; 
                r2_int64 index = 0;

                if(page->leaf != TRUE){
                        index   = r2_bsearch(page->indexes, key, 0, page->nkeys -1, btree->kcmp); 
                        minimum = r2_page_maximum(page->children[index]);
                        page->indexes[index] = minimum->indexes[minimum->nkeys -1];
                        page = minimum;
                        key  = minimum->indexes[minimum->nkeys -1];
                }

                index   = r2_bsearch(page->indexes, key, 0, page->nkeys -1, btree->kcmp); 
                /**
                 *     Remove key from page which means we're shifting down the keys.
                 *     Example page->index[i] = pages->indexes[i + 1]
                 */
                page->indexes[index] = NULL;
                for(; index < page->nkeys -1; ++index){
                        page->indexes[index] = page->indexes[index + 1];
                        page->indexes[index + 1] = NULL;
                }

                --page->nkeys;
            
                /*Page underflow? */
                r2_page_delete_rebalance(btree, page);

                    if(btree->fk != NULL)
                        btree->fk(key);
        }

        return btree;
}


/**
 * @brief               Rebalances a B Tree after deletion.
 * 
 * @param btree         B Tree.
 * @param page          Page.
 */
static void r2_page_delete_rebalance(struct r2_btree *btree, struct r2_page *page)
{
        struct r2_page *sibling = NULL;
        while(page != NULL){
                sibling = r2_page_get_sibling(page , btree->kcmp);
                if(page->nkeys < (page->mkeys / 2) && page != btree->root){
                                if((sibling->nkeys + page->nkeys) < page->mkeys)
                                       page = r2_page_catenate(btree, page, sibling);
                                else
                                        r2_page_underflow(page, sibling, btree->kcmp);
                }else if(page == btree->root && page->nkeys == 0){
                        r2_freepage(page, NULL);
                        btree->root = NULL;
                        break;
                }
                page->ncount = r2_page_calc_size(page);
                page = page->parent;
        }

        btree->ncount = r2_page_calc_size(btree->root);       
}


/**
 * @brief                       Returns the minimum page with the smallest key.
 * 
 * @param page                  Page.
 * @return struct r2_page*      Returns the minimum page, else NULL.
 */
struct r2_page* r2_page_minimum(struct r2_page *page)
{
        while(page != NULL && page->children[0] != NULL)
                page = page->children[0];

        return page;
}

/**
 * @brief                       Returns the maximum page with the largest key.
 * 
 * @param page                  Page.
 * @return struct r2_page*      Returns the maximum page, else NULL.
 */
struct r2_page* r2_page_maximum(struct r2_page *page)
{
        while(page != NULL && page->children[page->nkeys] != NULL)
                page = page->children[page->nkeys];

        return page;  

}

/**
 * @brief                    Finds the successor of a key in a specific page.
 * 
 * @param page               Page the key is on.
 * @param key                Key.
 * @param cmp                A comparison callback function.
 * @return struct r2_page*   Returns the page the successor is located on, else null.
 */
struct r2_page* r2_page_successor(struct r2_page *page, void *key, r2_cmp cmp)
{
        /**
         * @brief We search for the page our key is on.
         *        That page is our potential successor (psuccessor).
         */
        r2_int64  index = 0;
        struct r2_page *psuccessor = NULL; 
        while(page != NULL){
                index      = r2_bsearch(page->indexes, key, 0, page->nkeys -1, cmp);
                psuccessor = page;
                if(index < psuccessor->nkeys && cmp(key, psuccessor->indexes[index]) == 0)
                        break;

                page       = page->children[index];
        }

        /**
         * @brief Once we found the psuccessor our key is located on. We check if our key is the greatest on this page. 
         *        If our key isn't the greatest key then there's a key on psuccessor greater than our key which makes this page
         *        the acutal successor. We return right away in this case.   
         */
        if(index < (psuccessor->nkeys -1))
                return psuccessor;
        /**
         * @brief If we're here then it's possible the successor is further down in the tree or further up the tree.
         *        If psuccuessor is an internal node we continue down the true finding the minimum node 
         *        in psuccessor->children[index + 1], else we walk back up tree.     
         */ 
        else if(psuccessor->leaf != TRUE)
                psuccessor = r2_page_minimum(psuccessor->children[index + 1]); 
        else{
                psuccessor = psuccessor->parent; 
                while(psuccessor != NULL && r2_bsearch(psuccessor->indexes, key, 0,psuccessor->nkeys -1, cmp) == psuccessor->nkeys){
                        psuccessor = psuccessor->parent; 
                }
        }
        
        return psuccessor;    
}

/**
 * @brief                    Finds the predecessor of a key in a specific page.
 * 
 * @param page               Page.
 * @param key                Key.
 * @param cmp                A comparison callback function.
 * @return struct r2_page*   Returns the page the predecessor is located on, else null.
 */
struct r2_page*  r2_page_predecessor(struct r2_page *page, void *key, r2_cmp cmp)
{
        /**
         * @brief We search for the page our key is on.
         *        That page is our potential predecessor (ppredecessor).
         */
        r2_int64  index = 0;
        struct r2_page *ppredecessor = NULL; 
        while(page != NULL){
                index      = r2_bsearch(page->indexes, key, 0, page->nkeys -1, cmp);
                ppredecessor = page;
                if(index < ppredecessor->nkeys && cmp(key, ppredecessor->indexes[index]) == 0)
                        break;

                page       = page->children[index];
        }


        /**
         * @brief If we're here then it's possible the ppredecessor is further down in the tree or further up the tree.
         *        If ppredecessor is an internal node we continue down the true finding the maximum node 
         *        in ppredecessor->children[index], else we walk back up tree.     
         */ 
        if(ppredecessor->leaf != TRUE)
                ppredecessor= r2_page_maximum(ppredecessor->children[index]); 
        else{
        /**
         * @brief Once we found the ppredecessor our key is located on. We check if our key is the smallest on this page. 
         *        If our key isn't the smallest key then there's a key on ppredecessor smaller than our key which makes this page
         *        the acutal ppredecessor. We return right away in this case.   
         */
                if(index > 0)
                        return ppredecessor;

                ppredecessor = ppredecessor->parent; 
                while(ppredecessor != NULL &&  r2_bsearch(ppredecessor->indexes, key, 0,ppredecessor->nkeys -1, cmp) == 0){
                       ppredecessor = ppredecessor->parent; 
                }
        }
        
        return ppredecessor;    
} 

/**
 * @brief               Checks whether a B Tree is empty.
 * 
 * @param btree         B Tree
 * @return r2_int16     Returns TRUE when B Tree is empty, else FALSE.
 */
r2_int16 r2_btree_empty(const struct r2_btree *btree)
{
        return btree->root == NULL && btree->ncount == 0;
}




/**
 * @brief               Calculates the height of a B Tree.
 * 
 * @param page          Page.
 * @return r2_int64     Returns height.
 */
r2_int64 r2_page_height(const struct r2_page *page)
{
        r2_int64 height = -1; 
        if(page == NULL)
                return height; 

        r2_int64 max_height = 0; 
        for(int i = 0; i < (page->nkeys + 1); ++i){
                
                height = r2_page_height(page->children[i]) + 1; 
                max_height = height > max_height? height : max_height;
        }

        return max_height;
}


/**
 * @brief               Calculates the size of page i.e. number of children at the page.
 *                   
 * @param page          Page
 * @return r2_int64     Returns the size of the page.
 */
static r2_int64 r2_page_calc_size(const struct r2_page *page)
{     
        r2_int64 size = 0; 
        if(page != NULL){
                for(r2_int64 i = 0; page->children[i] != NULL;++i){
                        size += page->children[i]->ncount;   
                }        
                ++size;
        }
        return size;
}

/**
 * @brief               Inserts key into page at position pos.
 * 
 * @param page          Page.
 * @param pos           Position.
 * @param key           Key.
 */
static void r2_page_insert_key(struct r2_page *page, r2_int64 pos, void *key)
{
        /*Shifting elements to make room.*/
        if(page != NULL){
                if(pos < page->nkeys)
                        for(r2_int64 i = page->nkeys - 1; i >= pos; --i)
                                page->indexes[i + 1] = page->indexes[i];

                page->indexes[pos] = key; 
                ++page->nkeys;
        }

}

/**
 * @brief                       Gets sibling of a page.
 * 
 * @param page                  Page.
 * @param cmp                   A comparison callback function.
 * @return struct r2_page*      Returns the sibling of a page.
 */
static struct r2_page* r2_page_get_sibling(struct r2_page *page, r2_cmp cmp)
{
        struct r2_page *sibling = NULL; 
        struct r2_page *parent  = page->parent;

        if(parent != NULL){
                r2_int64 index = r2_bsearch(parent->indexes, page->indexes[0], 0, parent->nkeys -1, cmp);
                sibling        = parent->children[index + 1] != NULL? parent->children[index + 1] : parent->children[index - 1];
        }

        return sibling;
}

/**
 * @brief               Merges sibling to page.
 * 
 * @param page          Page.
 * @param sibling       Sibling.
 */
static struct r2_page* r2_page_catenate(struct r2_btree *btree,struct r2_page *page, struct r2_page *sibling)
{
        struct r2_page *parent  = page->parent;
        r2_int64 sibling_pos    = r2_bsearch(parent->indexes, sibling->indexes[0], 0, parent->nkeys -1, btree->kcmp); ; 
        r2_int64 page_pos       = r2_bsearch(parent->indexes, page->indexes[0], 0, parent->nkeys -1, btree->kcmp); 
        r2_int64 parent_pos     = (page_pos + sibling_pos) / 2;

        /*Always dump sibling keys and children into page. To achieve this we may have to swap sibling and page*/
        if(page_pos > sibling_pos){
                r2_int64 tpos = page_pos; 
                struct r2_page *tpage = page;
                page_pos        = sibling_pos; 
                page            = sibling;
                sibling_pos     = tpos;
                sibling         = tpage; 
        }

        /*
         * Dumping sibling keys into page.
         */
        for(int i = 0, j = page->nkeys; i < sibling->nkeys; ++i, ++j)
                page->indexes[j] = sibling->indexes[i];

        /**
         * Dumping sibling children into page
         */
        for(int i = 0, j = page->nkeys + 1; i < sibling->nkeys + 1; ++i, ++j){
                page->children[j] = sibling->children[i];
                if(page->children[j] != NULL)
                        page->children[j]->parent = page;
        }

        page->nkeys = page->nkeys + sibling->nkeys;
        
        /**
         * Insert median into in page.
         */
        void *median = parent->indexes[parent_pos];
        r2_page_insert_key(page, r2_bsearch(page->indexes, median, 0, page->nkeys -1, btree->kcmp), median);

        /**
         * Shift keys in parent starting at parent_pos
         */
        parent->indexes[parent_pos] = NULL;
        for(int i = parent_pos; i < parent->nkeys - 1; ++i){
                parent->indexes[i]  = parent->indexes[i + 1]; 
                parent->indexes[i + 1] = NULL;
        }
        
        /**
         * Shift children in parent starting at sibling_pos
         */
        parent->children[sibling_pos] = NULL; 
        for(int i = sibling_pos; i < parent->nkeys; ++i){
                parent->children[i] = parent->children[i + 1];
                parent->children[i + 1] = NULL;
        }

        --parent->nkeys;
        /**
         * B Tree shrunk and got a new root
         * 
         */
        if(parent->nkeys == 0){
                btree->root = page; 
                page->parent = NULL;
                r2_freepage(parent, NULL);
        }

        /*Remove sibling because it was merged into page.*/
        r2_freepage(sibling, NULL); 
        return page;
}


/**
 * @brief               Performs the underflow operation on a B Tree.
 * 
 * @param page          Page.
 * @param sibling       Sibling.
 * @param cmp           A comparison callback function.
 */
static void r2_page_underflow(struct r2_page *page, struct r2_page *sibling, r2_cmp cmp)
{
        struct r2_page *parent          = page->parent;
        r2_int64 page_pos               = r2_bsearch(parent->indexes, page->indexes[0], 0, parent->nkeys -1, cmp); 
        r2_int64 sibling_pos            = r2_bsearch(parent->indexes, sibling->indexes[0], 0, parent->nkeys -1, cmp); 
        r2_int64 parent_pos             = (page_pos + sibling_pos) / 2;        
        
        /**
         * 
         *Conceptually we're borrowing a key from sibling. 
         *The key that we borrow can either be the largest or smallest key in sibling.
         *If we take the largest key, then we take the largest child and the other case is symmetric.
         */
        void *bkey      = NULL; /*Borrowed key*/
        struct r2_page *bchild    = NULL; /*Borrowd child*/
        r2_int64 key_pos   = 0;    /*Borrowed key pos */
        r2_int64 child_pos = 0;    /*Borrowed child pos*/
        r2_int64 index = 0; /*Location where the borrowed key will be inserted*/
        r2_int64 ichild = 0; /*Location where the borrowed child will be inserted in page->children*/
        if(page_pos < sibling_pos){
                bkey    = sibling->indexes[key_pos];
                bchild  = sibling->children[child_pos]; 
                index   = page->nkeys;
                ichild  = index + 1;
        }else{
                key_pos   = sibling->nkeys -1; 
                child_pos = sibling->nkeys;
                bkey      = sibling->indexes[key_pos];
                bchild    = sibling->children[child_pos]; 
        }

        
        /**
         * Fix sibling keys
         * 
         */
        for(;key_pos < (sibling->nkeys - 1); ++key_pos)
                sibling->indexes[key_pos] = sibling->indexes[key_pos + 1];
        
        sibling->indexes[key_pos] = NULL; 
        --sibling->nkeys;
        /**
         * Fixing sibling children
         * 
         */
        for(;child_pos < sibling->nkeys + 1; ++child_pos)
                sibling->children[child_pos] = sibling->children[child_pos + 1]; 

        sibling->children[child_pos] = NULL;

       

        /*Add key to page*/
        r2_page_insert_key(page, index, bkey);

        /*Add child to page*/
        for(int i = page->nkeys; i > ichild;  --i)
                page->children[i] = page->children[i -1];

        page->children[ichild] = bchild;
        if(bchild != NULL)
                bchild->parent = page;
        /**
         * Swap median with bchild and we should be done
         * 
         */
        page->indexes[index] = parent->indexes[parent_pos];
        parent->indexes[parent_pos] = bkey;     
}

/**
 * @brief               Releases all memory associated with a page
 * 
 * @param page          Page
 * @param freekey       A callback function used to free key.
 */
static void r2_freepage(struct r2_page *page,  r2_fk freekey)
{
        if(freekey != NULL){
                for(int i = 0; i < page->nkeys; ++i)
                        freekey(page->indexes[i]);
        }

        free(page->children); 
        free(page->indexes);
        free(page);
}