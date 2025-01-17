#ifndef R2_RBTREE_H_
#define R2_RBTREE_H_
#include "r2_types.h"
#include "r2_list.h"


enum COLOR{
        RED   = 0, 
        BLACK = 1
}; 

struct r2_rbnode{
        void *key; /*key*/
        void *data; /*data*/
        enum COLOR color; /*color of node*/
        r2_uint64 ncount; /*number of descendants*/
        struct r2_rbnode *left; /*left child*/
        struct r2_rbnode *right;/*right child*/
        struct r2_rbnode *parent;/*parent of child*/
}; 


struct r2_rbtree{
        struct r2_rbnode *root; /*root*/
        r2_uint64 ncount; /*number of keys in tree*/
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

struct r2_rbnode* r2_create_rbnode();
struct r2_rbtree* r2_create_rbtree(r2_cmp, r2_cmp, r2_cpy, r2_cpy, r2_fk, r2_fd);
struct r2_rbtree* r2_destroy_rbtree(struct r2_rbtree *);
struct r2_rbnode* r2_rbnode_successor(const struct r2_rbnode *); 
struct r2_rbnode* r2_rbnode_predeccessor(const struct r2_rbnode *);
struct r2_rbnode* r2_rbnode_min(struct r2_rbnode *); 
struct r2_rbnode* r2_rbnode_max(struct r2_rbnode *);
struct r2_rbtree* r2_rbtree_insert(struct r2_rbtree *, void *, void *); 
struct r2_rbtree* r2_rbtree_delete(struct r2_rbtree *, void *);
struct r2_rbnode* r2_rbtree_search(struct r2_rbtree *, void *);
struct r2_rbnode* r2_rbtree_at(struct r2_rbnode *, r2_uint64 ); 
struct r2_list*   r2_rbtree_range_query(const struct r2_rbtree *, void *, void *, r2_act, void *);
void** r2_rbtree_get_keys(const struct r2_rbtree *);
void** r2_rbtree_get_values(const struct r2_rbtree *);
void r2_rbtree_inorder(struct r2_rbnode *, r2_act, void *); 
void r2_rbtree_preorder(struct r2_rbnode *, r2_act, void *); 
void r2_rbtree_postorder(struct r2_rbnode *, r2_act, void *); 
struct r2_rbnode* r2_rbnode_inorder_first(struct r2_rbnode *); 
struct r2_rbnode* r2_rbnode_inorder_next(struct r2_rbnode *);
struct r2_rbnode* r2_rbnode_preorder_first(struct r2_rbnode *); 
struct r2_rbnode* r2_rbnode_preorder_next(struct r2_rbnode *);
struct r2_rbnode* r2_rbnode_postorder_first(struct r2_rbnode *); 
struct r2_rbnode* r2_rbnode_postorder_next(struct r2_rbnode *);
r2_uint64 r2_rbnode_level(const struct r2_rbnode *);
r2_uint16 r2_rbtree_empty(const struct r2_rbtree *); 
struct r2_rbtree *r2_rbtree_copy(const struct r2_rbtree *);
r2_uint16 r2_rbtree_compare(const struct r2_rbtree *, const struct r2_rbtree *);
#endif