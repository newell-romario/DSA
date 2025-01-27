#ifndef R2_WAVL_TREE_H_
#define R2_WAVL_TREE_H_
#include "r2_types.h"
#include "r2_list.h"

/**
 * @brief       
 *              Introduction
 *  
 *              A Weak AVL (WAVL) Tree is a rank balanced binary search tree i.e. 
 *              every node has a rank which is represented by rank(x).
 *              Whenever x == NULL, rank(x) = -1, otherwise rank(x) is non-negative. 
 * 
 *              A central idea used in balancing a WAVL tree is the rank difference which is 
 *              defined as rank(parent(x)) - rank(x) where parent(x) returns the parent of x. 
 *              In simpler words the rank difference of x would be the rank of x parent minus
 *              the rank of x. The idea of rank difference naturally leads to the existence of different
 *              node types in a WAVL tree. In a WAVL tree nodes can be labled i-node when the rank difference 
 *              of the left and right children are equal, else a node is i,j-node 
 *              where i and j represents the rank difference of the left child and right child espectively. 
 * 
 * 
 *              A WAVL tree only allow node types of rank differences 1, and 2 where are all 
 *              leaf nodes have a rank of 0. A WAVL tree combines the best properties of AVL and RB
 *              trees. 
 * 
 *              Bottom Up Rebalancing After Insertion
 *                      
 *              Balancing in a WAVL tree can be done bottom up through the use of promotion, demotion and rotations.
 *              A promotion increases the rank(x) by 1 while a demotion does the opposite.When inserting into a WAVL tree
 *              the new node is given a rank of zero which automatically makes it a 1,1 node. However, that insertion
 *              may cause an imbalance in the tree. 
 * 
 *              We fix this by considering these conditions: 
 * 
 *              i) The parent of the new node was previously 1,1 leaf and after the insertion it became 0,1 unary node.
 *                 However, we only allow internal nodes of rank differences 1 or 2 which breakes the rule. How do we fix this? 
 *                 
 *                 a) While parent(x) 0,1 node promote parent(x) and x = parent(x) then repeat.
 *                 This either fixes the issue or now the issue is at parent(x) is a (2, 0) node. 
 *                 
 *                 b) The parent(x) is a (2, 0) node which means x is a 0 child. 
 *                      i) Let y = left(x) and assume x = left(parent(x)). 
 *                              a) Y can be either null or 2 child. If this is the case peform a left rotation on x and demote y.
 *                              b) Y can be a 1 node which means we need to a double rotation on y and demote parent(x), and x and promote y.
 *                 c) The parent (x) is a (0, 2) node which means x is a 0 child. It is symmetric. 
 * 
 *            Bottom Up Rebalancing after Deletion
 *              
 *            Deletion of a leaf or unary node in a WAVL tree can violate the rank rule, either by creating a (2, 2) leaf or 3 node. 
 *            The former happens when  the node to be deleted is a 1 child of unary node; the unary node becomes a (2,  2)  leaf node. 
 *            The latter happens when the deleted node is a 2-child, the node replacing it becomes a 3 child, which is null if the 
 *            deleted node was a leaf.
 *            
 *            We can fix this by considering these cases: 
 * 
 *            Case 1) x is a (2, 2) leaf which we forbid. Demoting rank(x) solves this issue or it propogates 
 *                   the issue at parent(x). We proceed as x = parent(x).
 *             
 *            Case 2) x has a 3 child. Let x be the 3 child and y=sibling(x)
 *                    a) When y is a 2 child 
 *                          We demote rank(parent(x)). Let x = parent(x). 
 * 
 *                    b) When y is a (2, 2) node 
 *                         We demote the rank of both y and parent(x). Let x = parent (x).
 * 
 *                   c) When y is not a 2 child and is not (2, 2) node. This means parent(x) is (1, 3) node and y is a 1 child.
 *                      Let v = left(y) and w = right(y).
 *                      
 *                      W is a 1 child: Peform a left or right rotation depending on the orientation of w. 
 *                      Promote rank(y) and demote rank(z). If is z is leaf demote it again.
 * 
 *                      W is a 2 child: Peform a double rotation on v. Promote v twice and demote  y once and demote z twice.  
 *              
 * 
 *            
 *                 
 *           
 */
struct r2_wavlnode{
        void *key; /*key*/
        void *data; /*data*/
        r2_uint64 ncount; /*number of descendants*/ 
        r2_int64 rank; /*rank of node*/
        struct r2_wavlnode *parent; /*parent*/
        struct r2_wavlnode *left; /*left child*/
        struct r2_wavlnode *right; /*right child*/
}; 

struct r2_wavltree{
        struct r2_wavlnode *root; 
        r2_uint64 ncount; /*number of keys in the tree*/
        r2_cmp kcmp;/*A comparison callback function*/
        r2_cmp dcmp;/*A comparison callback function*/ 
        r2_cpy kcpy;/*A callback function to copy keys*/
        r2_cpy dcpy;/*A callback function to copy values*/
        r2_fd  fd;/*A callback function that release memory used by data*/
        r2_fk  fk;/*A callback function that release memory used by key*/
        #ifdef PROFILE_TREE
                r2_int64 num_comparisons;
        #endif 
}; 

enum CHILD_TYPE{
        ZERO_CHILD,
        ONE_CHILD,
        TWO_CHILD, 
        TRHEE_CHILD 
};

 

r2_uint16 r2_wavltree_empty(const struct r2_wavltree *);
struct r2_wavlnode* r2_create_wavlnode(); 
struct r2_wavltree* r2_create_wavltree(r2_cmp, r2_cmp, r2_cpy, r2_cpy, r2_fk, r2_fd); 
struct r2_wavltree* r2_destroy_wavltree(struct r2_wavltree*); 
struct r2_wavlnode* r2_wavlnode_successor(struct r2_wavlnode *);
struct r2_wavlnode* r2_wavlnode_predecessor(struct r2_wavlnode *);
struct r2_wavlnode* r2_wavlnode_min(struct r2_wavlnode *); 
struct r2_wavlnode* r2_wavlnode_max(struct r2_wavlnode *);
struct r2_wavlnode* r2_wavltree_search(struct r2_wavltree *, const void *);
struct r2_wavltree* r2_wavltree_insert(struct r2_wavltree *, void *, void *);
struct r2_wavltree* r2_wavltree_delete(struct r2_wavltree *, void *);
struct r2_wavlnode* r2_wavltree_at(struct r2_wavlnode*,  r2_uint64);
void** r2_wavltree_get_keys(const struct r2_wavltree *); 
void** r2_wavltree_get_values(const struct r2_wavltree *); 
struct r2_list* r2_wavltree_range_query(const struct r2_wavltree *, void *, void *, r2_act, void *);
void r2_wavltree_inorder(struct r2_wavlnode *, r2_act, void *); 
void r2_wavltree_preorder(struct r2_wavlnode *, r2_act, void *); 
void r2_wavltree_postorder(struct r2_wavlnode *, r2_act, void *); 
struct r2_wavlnode *r2_wavlnode_inorder_next(struct r2_wavlnode*); 
struct r2_wavlnode *r2_wavlnode_preorder_next(struct r2_wavlnode*); 
struct r2_wavlnode *r2_wavlnode_postorder_next(struct r2_wavlnode*); 
struct r2_wavlnode *r2_wavlnode_inorder_first(struct r2_wavlnode*); 
struct r2_wavlnode *r2_wavlnode_preorder_first(struct r2_wavlnode*); 
struct r2_wavlnode *r2_wavlnode_postorder_first(struct r2_wavlnode*); 
size_t r2_wavlnode_level(const struct r2_wavlnode *); 
struct r2_wavltree* r2_wavltree_copy(const struct r2_wavltree *);
r2_uint16 r2_wavltree_compare(const struct r2_wavltree *, const struct r2_wavltree   *);
r2_int64  r2_wavltree_height(const struct r2_wavlnode *);
#endif