#ifndef R2_AVL_TREE_H_
#define R2_AVL_TREE_H_
#include "r2_types.h"
#include "r2_list.h"
#define PROFILE_TREE

/**
 *An Adel’son-Vel’skii and Landis (AVL) tree is a balanced binary search tree
 *that offers O(log n) time on insertion, deletion, find, minimum, maximum, successor
 *and predecessor operations etc. It does this by allowing the height difference
 *between left and right subtrees to be either -1, 0, 1 or succinctly 
 *|height(root->left) - height(root->right)| <= 1. If the tree becomes unbalanced
 *we perform either left or right rotations until the latter condition is met.
 *      
 *_____________________________________________________
 *                      Insertion
 *_____________________________________________________
 *Inserting into an AVL tree starts as a regular insert into in a binary tree where
 *we find either a leaf node or unary node and attach on the new node. This insertion 
 *may cause unbalance higher up in the tree. We proceed up the tree from the newly
 *inserted node until we're at a node which violates the rule. 
 *
 *Let the first node that we encounter on the path to the root that violates our condition
 *be called z, let y be the child of z with the heigher height and let x be the child of y
 *with higher height(if both children of y have equal height choose the child that is an ancestor
 *to the newly inserted node). We perform a single or double rotation to fix the imbalance which 
 *solves the height unbalance globally. A detailed description of this can be found in
 *Algorithms Design and Applications (pg 117 - pg 149).
 *
 * ______________________________________________________
 *                      Deletion
 * ______________________________________________________
 *Deleting a node from an AVL tree begins as a regular search operation where we locate the node in question.
 *Lets call this node r. If r is an internal node we can swap the contents of r with it's successor or precedessor 
 *and let r be equal to it's sucessor or predecessor. We then delete r and proceed up the tree from the parent of r.
 * 
 *Let the first node that we encounter on the path to the root that violates our condition
 *be called z, let y be the child of z with the heigher height and let x be the child of y
 *with higher height(if both children of y have equal height choose the child that is on the same side
 *as y i.e. is if y is a left child then x will be the left child of y.). We perform single or double rotation to fix 
 *the imbalance which solves the height unbalance globally. A detailed description of this can be found in
 *Algorithms Design and Applications (pg 117 - pg 149).
 * 
 *______________________________________________________________
 *                      Other Operations
 *______________________________________________________________
 * Other tree operations can be found in Introduction to Algorithms (CLRS) or Algorithms Design and Applications.
 * 
 */
struct r2_avlnode{
        void *key;/*key*/
        void *data;/*data*/
        r2_uint64 ncount;/*number of nodes in subtree or (number of descendants)*/
        r2_int64 height;/*height of tree*/
        struct r2_avlnode *left; /*left child*/
        struct r2_avlnode *right;/*right child*/
        struct r2_avlnode *parent;/*parent of child*/
}; 

struct r2_avltree{
        struct r2_avlnode *root;/*root of tree*/
        r2_uint64 ncount;/*number of keys in the tree*/
        r2_cmp kcmp;/*A comparison callback function*/
        r2_cmp dcmp;/*A comparison callback function*/ 
        r2_cpy kcpy;/*A callback function to copy keys*/
        r2_cpy dcpy;/*A callback function to copy values*/
        r2_fd  fd;/*A callback function that release memory used by data*/
        r2_fk  fk; /*A callback function that release memory used by key*/
        /*Used for bench marking number of comparisons*/
        #ifdef PROFILE_TREE
                r2_int64 num_comparisons;
        #endif 
}; 


struct r2_avlnode* r2_create_avlnode();
struct r2_avlnode* r2_avlnode_successor(struct r2_avlnode *); 
struct r2_avlnode* r2_avlnode_predecessor(struct r2_avlnode *);
struct r2_avlnode* r2_avlnode_min(struct r2_avlnode *); 
struct r2_avlnode* r2_avlnode_max(struct r2_avlnode *);
struct r2_avltree* r2_create_avltree(r2_cmp, r2_cmp, r2_cpy, r2_cpy, r2_fk, r2_fd);
struct r2_avltree* r2_destroy_avltree(struct r2_avltree *);
struct r2_avltree* r2_avltree_insert(struct r2_avltree *, void *, void *);
struct r2_avltree* r2_avltree_delete(struct r2_avltree *, void *); 
struct r2_avlnode* r2_avltree_search(struct r2_avltree *, const void *);
struct r2_avlnode* r2_avltree_at(struct r2_avlnode *, r2_uint64);
struct r2_list* r2_avltree_range_query(const struct r2_avltree *, void *, void *, r2_act, void *);
void** r2_avltree_get_keys(const struct r2_avltree *);
void** r2_avltree_get_values(const struct r2_avltree *);
void r2_avltree_inorder(struct r2_avlnode *, r2_act, void *);
void r2_avltree_postorder(struct r2_avlnode *, r2_act, void *); 
void r2_avltree_preorder(struct r2_avlnode *, r2_act, void *); 
struct r2_avlnode* r2_avlnode_inorder_next(struct r2_avlnode *);
struct r2_avlnode* r2_avlnode_inorder_first(struct r2_avlnode *);
struct r2_avlnode*  r2_avlnode_postorder_next(struct r2_avlnode *); 
struct r2_avlnode*  r2_avlnode_postorder_first(struct r2_avlnode *); 
struct r2_avlnode*  r2_avlnode_preorder_next(struct r2_avlnode *); 
struct r2_avlnode*  r2_avlnode_preorder_first(struct r2_avlnode *); 
r2_int64 r2_avltree_height(const struct r2_avlnode *); 
r2_uint64 r2_avltree_level(const struct r2_avlnode *);
r2_uint64 r2_avltree_size(const struct r2_avlnode *);
r2_uint16 r2_avltree_empty(const struct r2_avltree *); 
struct r2_avltree *r2_avltree_copy(const struct r2_avltree *);
r2_uint16 r2_avltree_compare(const struct r2_avltree *, const struct r2_avltree *);
#endif