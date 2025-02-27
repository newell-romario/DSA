#ifndef R2_BTREE_H_
#define R2_BTREE_H_
#include "r2_types.h"
/*
 * B Trees are a part of the balanced search tree data structure family which means
 * insertion, search and deletion happen in O(log n) time. This makes it an efficient data
 * structure like it's family members. However, where normal balanced binary search trees
 * only have 1 key, a B Tree has numerous keys. Having numerous keys is advantageous i.e.
 * the height of search tree is reduced making searches quicker. 
 * 
 * A B tree shines in terms peformance when it comes storing a bulk of data in memory which reduces
 * the number of secondary/tertiary memory access that's needed to retrieve data. 
 * 
 * Formally for a tree to be considered as a B Tree it has to meet the following conditions: 
 *      1) Every leaf node has the same depth. 
 *      2) Every node except the root has a minimum of (k + 1) children and maximum (2k + 1) children. 
 *         The root is a leaf or has at least two children. 
 * 
 *      3) Every node except the root has minimum k keys and 2k keys maximum while root has at least holds between 1 and 2k keys inclusive.
 * 
 *      
 * In B Trees nodes are called pages and pages are of a certain size. Sometimes pages have the same size as
 * a page in the OS or an arbitrary size. According to Robert Sedgwick a page size should always be even. 
 *  A good description of all algorithms related to B Trees can be found in the original paper by R. Bayer and E.McCreight. 
 * 
 * Let's a take at look the operations a B Tree can do: 
 * -----------------------------------------------------------
 * Search Algorithm
 * -----------------------------------------------------------
 * Like the search algorithm for BST, the search algorithm for a B Tree is no different. 
 * We start at the root and proceed as normal. As we proceed down the tree we check
 * all the keys on the current page. If the page contains the key then we're done else
 * we check if the key would be in one of the children and proceed to the child. If the key is not in
 * one of the page or any children then key isn't found. 
 *
 * ---------------------------------------------------------------
 * Insertion Algorithm
 * ---------------------------------------------------------------
 * The B Tree algorithm is similar to it's other family members. However, it has a few differences. 
 * One difference is insertion only happens at leaves in B Trees unlike BST where insertion can happen 
 * at an unary node or leaf. The tree grows bottom up for B Trees unlike BST which is top down.
 * The height of a B Tree is increased by spliting the root of the tree which increases the height by one. With those 
 * key differences out of the way we can discuss the insertion algorithm for B Tree. 
 * 
 * 1)Search for key k in the B Tree.
 * 2)If key k is found replace its value then done. 
 * 3)If key k is not found, let s be the page k would be inserted on. 
 * 4)Is S null? Create root page with key k and done. 
 * 4)Is S full? If is not full, insert key k into S and done. Else split S and insert key k into the appropriate child.
 * 5)Proceed up the tree spliting pages which became overflowed because of our earlier split. 
 * 
 * 
 * ----------------------------------------------------------------------
 * Deletion Algorithm
 * ----------------------------------------------------------------------
 * The B Tree deletion algorithm is a little bit more complicated than the insertion algorithm but boils down to one question. 
 * Has the deletion caused an underflow on the current page? If yes, then we have violated one of the B Tree properties
 * which is every page except the root must have minimum t keys and maximum 2t keys. 
 * 
 * Whenever an underflow happens we can  fix this by either catenation or underflow operations described in the original paper. 
 * Both the catenation and underflow operations are mainly concerned with three pages which are the current page which has the undeflow which we'll call u
 * , the parent of u and the sibling s of u which is also a child of p. The sum of the number of keys in u and s determines if a catenation versus an undeflow
 * operation will happen. 
 * 
 * Catenation
 * The catenation operation only occurs if the sum of the number of keys in s and u < 2t. When this case holds true, we can merge both s and u forming a new page
 * n. We then delete the appropriate key k from p and insert it into the new page n. This solves our underflow issue at the current page but may have caused it at
 * the parent. We continue up the tree at the parent. 
 * 
 * Underflow
 * The underflow operation only occurs if the sum of the number of keys is greater than or equal to 2t. In this case we merge s and u and then split  
 * which gives us two pages with same number of keys which we are done. 
 */
struct r2_page{
        r2_int16 leaf;/*Boolean that stores whether page is leaf.*/
        r2_int64 ncount;/*Size of subtree.*/ 
        r2_int64 mkeys;/*Number of keys the page can hold.*/
        r2_int64 nkeys;/*Number of keys currently held in the page.*/
        void **indexes;/*Array storing keys in the page.*/
        struct r2_page *parent; /*Parent of this page.*/
        struct r2_page **children;/*Children of page.*/
};


struct r2_btree{
        struct r2_page *root;/*Root of the tree.*/ 
        r2_int64 ncount;/*Size of the B Tree*/
        r2_int64 order;/*Maximum number of keys a page can hold.*/
        r2_cmp kcmp;/*A comparison callback function*/
        r2_fk  fk; /*A callback function that release memory used by key*/
};

struct r2_btree* r2_create_btree(r2_int64, r2_cmp, r2_fk); 
struct r2_page*  r2_create_page(r2_int64);
struct r2_btree* r2_destroy_btree(struct r2_btree *);
struct r2_page*  r2_page_successor(struct r2_page *, void *, r2_cmp); 
struct r2_page*  r2_page_predecessor(struct r2_page *, void * , r2_cmp); 
struct r2_page*  r2_page_minimum(struct r2_page *); 
struct r2_page*  r2_page_maximum(struct r2_page *);
struct r2_page*  r2_btree_search(const struct r2_btree *, void *);
struct r2_btree* r2_btree_insert(struct r2_btree *, void *);
struct r2_btree* r2_btree_delete(struct r2_btree *, void *);
r2_int64 r2_page_height(const struct r2_page *);
r2_int16 r2_btree_empty(const struct r2_btree*);

#endif