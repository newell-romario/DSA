#include "r2_rbtree.h"
#include <stdlib.h>
#include <stdio.h>

/**********************File scope functions************************/
static void r2_freenode(struct r2_rbnode *, r2_fk, r2_fd);
static r2_uint64 r2_rbnode_recalc_size(const struct r2_rbnode *);
static r2_int64 r2_rbnode_recalc_height(const struct r2_rbnode*); 
static r2_int64 MAX(r2_int64 a, r2_int64 b);
static void r2_rbtree_restructure(struct r2_rbtree *, struct r2_rbnode *, struct r2_rbnode *); 
static void r2_rbnode_right_rotation(struct r2_rbtree *, struct r2_rbnode *); 
static void r2_rbnode_left_rotation(struct r2_rbtree *, struct r2_rbnode *); 
static void r2_rbtree_insert_rebalance(struct r2_rbtree *, struct r2_rbnode *);
static void r2_rbtree_delete_rebalance(struct r2_rbtree *, struct r2_rbnode *);
static struct r2_rbnode* r2_rbnode_getsibling(struct r2_rbnode *);
static r2_uint16 r2_rbnode_is_red(const struct r2_rbnode *);
/***********************File scope functions************************/



/**
 * @brief               Calculates the height of the tree recursively.          
 *                     
 *                      Please call seldomly since it's expensive to calculate the height of the tree recursively.
 *                      The height of tree can be read from root->height without processing the tree.
 * 
 * @param root          Root.
 * @return r2_uint64    Returns the height of the tree, else -1 for empty trees.
 */
r2_int64 r2_rbtree_height(const struct r2_rbnode *root)
{
        if(root == NULL)
                return -1; 

        r2_int64 left_height  = r2_rbtree_height(root->left) + 1;  
        r2_int64 right_height = r2_rbtree_height(root->right) + 1;
        return MAX(left_height, right_height);    
}

/**
 * @brief                       Creates an empty node.
 * 
 * @return struct r2_rbnode*    Returns an empty node, else NULL.
 */
struct r2_rbnode* r2_create_rbnode()
{
        struct r2_rbnode *node = malloc(sizeof(struct r2_rbnode)); 
        if(node != NULL){
                node->key               = NULL; 
                node->data              = NULL;
                node->color             = RED; 
                node->ncount            = 1;
                node->left              = NULL; 
                node->right             = NULL; 
                node->parent            = NULL;  
        }
        return node; 
}

/**
 * @brief                       Creates an empty red and black tree.
 * 
 * @param kcmp                  A comparison callback function for key.
 * @param dcmp                  A comparison callback function for data.
 * @param kcpy                  A callback function to copy key.
 * @param dcpy                  A callback function to copy values.
 * @param fk                    A callback function that release memory used by key.
 * @param fd                    A callback function that release memory used by data.
 * @return struct r2_rbtree*    Returns an empty red and black tree, else NULL.
 */
struct r2_rbtree* r2_create_rbtree(r2_cmp kcmp, r2_cmp dcmp, r2_cpy kcpy, r2_cpy dcpy, r2_fk fk, r2_fd fd)
{
        struct r2_rbtree *tree = malloc(sizeof(struct r2_rbtree)); 
        if(tree != NULL){
                tree->root   = NULL; 
                tree->ncount = 0;
                tree->kcmp   = kcmp;
                tree->dcmp   = dcmp;
                tree->kcpy   = kcpy; 
                tree->dcpy   = dcpy;
                tree->fk     = fk; 
                tree->fd     = fd;
                #ifdef PROFILE_TREE
                        tree->num_comparisons = 0;
                #endif  
        }
}


/**
 * @brief                               Destroys red and black tree.

 * @return struct r2_rbtree*            Returns NULL when tree is successfully destroyed.
 */
struct r2_rbtree* r2_destroy_rbtree(struct r2_rbtree *tree)
{
        struct r2_rbnode *root    = r2_rbnode_postorder_first(tree->root); 
        struct r2_rbnode *oldroot = NULL; 
        while(root != NULL){
                oldroot = root; 
                root = r2_rbnode_postorder_next(root);
                r2_freenode(oldroot, tree->fk, tree->fd); 
        }
        
        free(tree); 
        return NULL; 
}

/**
 * @brief                       Finds the successor of root.
 * 
 * @param root                  Root.
 * @return struct r2_rbnode*    Returns successor, else NULL. 
 */
struct r2_rbnode* r2_rbnode_successor(const struct r2_rbnode *root)
{
        if(root->right != NULL)
                return r2_rbnode_min(root->right);
        
        struct r2_rbnode *successor = root->parent; 
        while(successor != NULL && successor->right == root){
                root      = successor; 
                successor = successor->parent;
        }
        return successor; 
}


/**
 * @brief                       Finds the predecessor of the root.
 * 
 * @param root                  Root.
 * @return struct r2_rbnode*    Returns predecessor, else NULL. 
 */
struct r2_rbnode* r2_rbnode_predeccessor(const struct r2_rbnode *root)
{
        if(root->left != NULL)
                return r2_rbnode_max(root->left);
        
        struct r2_rbnode *predecessor = root->parent; 
        while(predecessor != NULL && predecessor->left == root){
                root        = predecessor; 
                predecessor = predecessor->parent;
        }
        return predecessor; 
}
/**
 * @brief                       Returns the minimum node in tree.
 * 
 * @param root                  Root. 
 * @return struct r2_rbnode*    Returns the minimum node, else NULL.
 */
struct r2_rbnode* r2_rbnode_min(struct r2_rbnode *root)
{
        while(root != NULL && root->left != NULL)
                root = root->left; 
        
        return root;
}

/**
 * @brief                       Returns the maximum node in the tree.
 * 
 * @param root                  Root.
 * @return struct r2_rbnode*    Returns the maximum node, else NULL.
 */
struct r2_rbnode* r2_rbnode_max(struct r2_rbnode *root)
{
        while(root != NULL && root->right != NULL)
                root = root->right; 
        
        return root;

}

/**
 * @brief          Performs a right rotation on root.
 * 
 * @param tree     RB Tree.
 * @param root     Root.
 */
static void r2_rbnode_right_rotation(struct r2_rbtree *tree, struct r2_rbnode *root)
{
        struct r2_rbnode *parent      = root->parent;
        struct r2_rbnode *grandparent = parent->parent; 

        parent->left = root->right; 
        if(parent->left != NULL)
                parent->left->parent = parent;

        parent->ncount        = r2_rbnode_recalc_size(parent);
        root->right           = parent; 
        root->right->parent   = root; 
        root->ncount          = r2_rbnode_recalc_size(root); 

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root; 

                grandparent->ncount        = r2_rbnode_recalc_size(grandparent);
        }
                
        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root   = root; 
                tree->ncount = root->ncount;
        }
}

/**
 * @brief       Performs a left rotation. 
 *      
 * @param tree  RB Tree  
 * @param root  Tree root
 */
static void r2_rbnode_left_rotation(struct r2_rbtree *tree, struct r2_rbnode *root)
{
        struct r2_rbnode *parent      = root->parent;
        struct r2_rbnode *grandparent = parent->parent; 

        parent->right = root->left; 
        if(parent->right != NULL)
                parent->right->parent = parent;

        
      
        parent->ncount        = r2_rbnode_recalc_size(parent);
        root->left            = parent; 
        root->left->parent    = root; 
        root->ncount          = r2_rbnode_recalc_size(root); 

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root; 
        
        
                grandparent->ncount        = r2_rbnode_recalc_size(grandparent);
        }
                
        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root   = root; 
                tree->ncount = root->ncount;
        }
}

/**
 * @brief               Rebalances the red and black tree.
 *    
 * @param tree          RB Tree.
 * @param root          Root.
 */
static void r2_rbtree_insert_rebalance(struct r2_rbtree *tree, struct r2_rbnode *root)
{
                struct r2_rbnode *parent      = root->parent;  
                struct r2_rbnode *grandparent = NULL; 
                struct r2_rbnode *uncle       = NULL;  
                while(parent != NULL && parent->color == RED){
                        parent->ncount = r2_rbnode_recalc_size(parent);
                        grandparent = parent->parent;
                                if(grandparent->right == parent){
                                        uncle = grandparent->left; 
                                        if(uncle != NULL && uncle->color == RED){
                                                uncle->color  = BLACK; 
                                                parent->color = BLACK;
                                                grandparent->color = RED;                                               
                                                root   = grandparent; 
                                        }else{
                                              if(parent->left == root){
                                                        r2_rbnode_right_rotation(tree, root); 
                                              }else root = parent;
                                              r2_rbnode_left_rotation(tree, root);
                                              root->color        = BLACK; 
                                              root->left->color  = RED;  
                                              break;
                                        }
                                }      
                                else{
                                        uncle = grandparent->right;
                                        if(uncle != NULL && uncle->color == RED){
                                                uncle->color  = BLACK; 
                                                parent->color = BLACK;
                                                grandparent->color = RED; 
                                                root   = grandparent; 
                                        }else{
                                              if(parent->right == root){
                                                        r2_rbnode_left_rotation(tree, root); 
                                              }else root = parent;
                                        
                                        
                                              r2_rbnode_right_rotation(tree, root);
                                              root->color         = BLACK; 
                                              root->right->color  = RED; 
                                              break;
                                        }
                                }
                                parent = root->parent; 
                                root->ncount = r2_rbnode_recalc_size(root);         
                }
                
                while(parent != NULL){
                        parent->ncount = r2_rbnode_recalc_size(parent);
                        parent = parent->parent; 
                }
                tree->root->color = BLACK; 
                tree->ncount= tree->root->ncount;
}




/**
 * @brief                       Performs an insertion into the RB Tree.
 * 
 * @param tree                  RB Tree.
 * @param key                   Key that will be inserted.
 * @param data                  Data that will be inserted along with key. 
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE.
 */
r2_uint16 r2_rbtree_insert(struct r2_rbtree *tree, void *key, void *data)
{
        struct r2_rbnode **root  = &tree->root; 
        struct r2_rbnode *parent = NULL;
        r2_int64 result   = 0;
        r2_uint64 SUCCESS = FALSE;
        while(*root != NULL){
                parent = *root; 
                result = tree->kcmp(key, parent->key);
                if( result > 0)
                        root = &((*root)->right);
                else if(result < 0)
                        root = &((*root)->left);
                else {
                        (*root)->data = data;
                        SUCCESS = TRUE;
                        return SUCCESS; 
                }
        }

        struct r2_rbnode *temp = r2_create_rbnode(); 
        if(temp != NULL){
                temp->key = key; 
                temp->data = data; 
                temp->parent = parent; 
                *root = temp;
                r2_rbtree_insert_rebalance(tree, *root);
                SUCCESS = TRUE;
        }  
        return SUCCESS;
}



/**
 * @brief                       Performs the search operation on rb tree.
 * 
 * @param tree                  RB Tree
 * @param key                   Key to search for.
 * @return struct r2_rbnode*    Returns the key along with data.
 */
struct r2_rbnode* r2_rbtree_search(struct r2_rbtree *tree, void *key)
{
        struct r2_rbnode *root  = tree->root; 
        r2_int64 result = 0;
        while(root != NULL){
                #ifdef PROFILE_TREE
                        ++tree->num_comparisons;
                #endif 
                result = tree->kcmp(key, root->key); 
                if(result > 0)
                        root = root->right;
                else if(result < 0)
                        root = root->left;
                else break;
        }
        return root; 
}


/**
 * @brief               Rebalances red and black tree.
 *                      
 * @param tree          RB Tree.
 * @param root          Root.
 */
static void r2_rbtree_delete_rebalance(struct r2_rbtree *tree, struct r2_rbnode *root)
{
        struct r2_rbnode *parent  = NULL; 
        struct r2_rbnode *child   = NULL;
        struct r2_rbnode *sibling = NULL;
        while(root != tree->root && root->color == BLACK){
                parent = root->parent;
                if(parent->right == root){
                        sibling = parent->left;
                        if(sibling->color == RED){
                                sibling->color = BLACK; 
                                parent->color  = RED;
                                r2_rbnode_right_rotation(tree, sibling);
                                sibling = parent->left;
                        }

                        if(r2_rbnode_is_red(sibling->left) == FALSE && r2_rbnode_is_red(sibling->right) == FALSE){
                                sibling->color = RED; 
                                root = sibling->parent;
                        }else{
                                if(r2_rbnode_is_red(sibling->right) == TRUE){
                                        child  = sibling->right;
                                        child->color   = BLACK;
                                        sibling->color = RED;
                                        r2_rbnode_left_rotation(tree, child);
                                        sibling = child;
                                }
                                sibling->color = parent->color; 
                                parent->color  = BLACK; 
                                sibling->left->color = BLACK;
                                r2_rbnode_right_rotation(tree, sibling);
                                root = tree->root;
                        }       
                }else{
                        sibling = parent->right;
                        if(sibling->color == RED){
                                sibling->color = BLACK; 
                                parent->color  = RED;
                                r2_rbnode_left_rotation(tree, sibling);
                                sibling = parent->right;
                        }

                        if(r2_rbnode_is_red(sibling->left) == FALSE && r2_rbnode_is_red(sibling->right) == FALSE){
                                sibling->color = RED; 
                                root = sibling->parent;
                        }else{
                                if(r2_rbnode_is_red(sibling->left) == TRUE){
                                        
                                        child  = sibling->left;
                                        child->color   = BLACK;
                                        sibling->color = RED;
                                        r2_rbnode_right_rotation(tree, child);
                                        sibling = child;
                                }
                                sibling->color = parent->color; 
                                parent->color = BLACK; 
                                sibling->right->color = BLACK;
                                r2_rbnode_left_rotation(tree, sibling);
                                root = tree->root;
                        }       
                }
       }
 
   root->color = BLACK;
   tree->ncount = r2_rbnode_recalc_size(tree->root);
}

/**
 * @brief                       Performs the delete operation on a red and black tree.
 * 
 * @param tree                  Red and black tree.
 * @param key                   Key that will be removed.
 * @return r2_uint16            Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_rbtree_delete(struct r2_rbtree *tree, void *key)
{
        struct r2_rbnode *root = r2_rbtree_search(tree, key);
        r2_uint16 SUCCESS = FALSE;
        if(root != NULL){

                /*Represents where the rebalancing will start from. */
                struct r2_rbnode *child  = NULL; 

                /*Stores the color of root to check if we removed a black node.*/
                enum COLOR root_color = root->color;

                if(root->left == NULL && root->right == NULL)
                /*When root is an external node we set child to root. 
                  This helps us to simplify the delete logic without using a nil node.*/
                        child = root;
                else if(root->right == NULL){
                        child = root->left;
                        r2_rbtree_restructure(tree, root, child);
                }else if(root->left == NULL){
                        child = root->right;
                        r2_rbtree_restructure(tree, root, child);
                }else{
                        struct r2_rbnode *successor = r2_rbnode_successor(root);
                        root_color = successor->color;
                        root->key  = successor->key; 
                        root->data = successor->data;
                        root = successor;
                        /*When root is an external node we set child to root. 
                        This helps us to simplify the delete logic without using a nil node.*/
                        if(root->right != NULL){
                                child = root->right;
                                r2_rbtree_restructure(tree, root, child);
                        }else child = root;
                      
                }
                
                if(root_color == BLACK)
                        r2_rbtree_delete_rebalance(tree, child); 

                /*If root is an external node that means child was set to root earlier.
                  We transplant root will null, else we transplant with it's respective non null child.*/
                if(child == root)
                        r2_rbtree_restructure(tree, root, NULL);         
                
                 child = child->parent;
                 while(child != NULL){
                        child->ncount = r2_rbnode_recalc_size(child); 
                        child = child->parent;   
                }
                        tree->ncount = r2_rbnode_recalc_size(tree->root);

                r2_freenode(root, tree->fk, tree->fd);  
                SUCCESS = TRUE; 
        }

        return SUCCESS;
}

/**
 * @brief                      Returns the first node in an order traversal which is the miniumum node in the subtree.
 * 
 * @param root                 Root.
 * @return struct r2_rbnode*   Returns the first node.
 */
struct r2_rbnode* r2_rbnode_inorder_first(struct r2_rbnode *root)
{
        return r2_rbnode_min(root);
}

/**
 * @brief                       Finds the next node in an inorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_rbnode*    Returns the next node in an inorder traversal, else NULL.
 */
struct r2_rbnode* r2_rbnode_inorder_next(struct r2_rbnode *root)
{
        return r2_rbnode_successor(root);
}

/**
 * @brief               Performs an inorder traversal and an action for each node.
 *                      
 * @param tree          RB Tree.
 * @param action        Action to be performed on node.
 * @param arg           Argument passed to action.
 */
void r2_rbtree_inorder(struct r2_rbnode *root, r2_act action, void *arg)
{
        struct r2_rbnode *old_root = root; 
        struct r2_rbnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_rbnode_inorder_first(root); 
        while(root != NULL){
                action(root, arg);
                root = r2_rbnode_inorder_next(root);
        }

        old_root->parent = parent;
}



/**
 * @brief                      Returns the first node in a preorder traversal.
 * 
 * @param root                 Root.
 * @return struct r2_rbnode*   Returns the first node.
 */
struct r2_rbnode* r2_rbnode_preorder_first(struct r2_rbnode *root)
{
        return root; 
}
/**
 * @brief                      Finds the next node in an preorder traversal.
 * 
 * @param root                 Root.
 * @return struct r2_rbnode*   Returns the next node in an preorder traversal, else NULL.
 */
struct r2_rbnode* r2_rbnode_preorder_next(struct r2_rbnode *root)
{
        if(root->left != NULL)
                return root->left; 
        
        if(root->right != NULL)
                return root->right;
                
        struct r2_rbnode *parent = root->parent; 
        if(parent != NULL){
                if(parent->left == root)
                        return parent->right;
                
                while(parent != NULL && parent->right == root){
                        root = parent; 
                        parent = parent->parent;
                }

                if(parent != NULL)
                        root = parent->right; 
                else
                        root = parent; 
                return root; 
        }
}

/**
 * @brief               Performs an preorder traversal and an action for each node.
 *                      
 * @param tree          RB Tree.
 * @param arg           Argument passed to action.
 * @param action        Action to be performed on node.
 */
void r2_rbtree_preorder(struct r2_rbnode *root, r2_act action, void *arg)
{
        struct r2_rbnode *old_root = root; 
        struct r2_rbnode *parent   = root->parent; 
        root->parent = NULL;

        while(root != NULL){
                action(root, arg); 
                root = r2_rbnode_preorder_next(root);
        }

        old_root->parent = parent;
}

/**
 * @brief                      Returns the first node in a postorder traversal.
 * 
 * @param root                 Root.
 * @return struct r2_rbnode*   Returns the first node.
 */
struct r2_rbnode* r2_rbnode_postorder_first(struct r2_rbnode *root)
{
        while(root != NULL && root->left != NULL)
                root = root->left; 
        
        while(root != NULL && root->right != NULL)
                root = root->right; 

        return root; 

}

/**
 * @brief                      Returns the next node in a postorder traversal.
 * 
 * @param root                 Root.
 * @return struct r2_rbnode*   Returns the first node.
 */
struct r2_rbnode* r2_rbnode_postorder_next(struct r2_rbnode *root)
{
        struct r2_rbnode *parent = root->parent; 
        if(parent != NULL)
                if(parent->left == root){
                        root = parent->right;
                        while(root != NULL){
                                parent = root; 
                                if(root->left != NULL)
                                        root = root->left;
                                else 
                                        root = root->right;
                        }
                } 

        return parent; 
}

/**
 * @brief               Performs an postorder traversal and an action for each node.
 *                      
 * @param tree          RB Tree.
 * @param arg           Argument passed to action.
 * @param action        Action to be performed on node.
 */
void r2_rbtree_postorder(struct r2_rbnode *root, r2_act action, void *arg)
{
        struct r2_rbnode *old_root = root; 
        struct r2_rbnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_rbnode_postorder_first(root);
        while(root != NULL){
                action(root, arg);
                root = r2_rbnode_postorder_next(root); 
        }

        old_root->parent = parent;
}


/**
 * @brief               Gets keys in tree.
 *                      Gets the keys in sorted order.
 * @param tree          RB Tree.
 * @return void**       Returns an array of keys, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the keys. N.B. It's the responsibilty of the caller to free keys array
 *                      by calling free function with the keys array. The size of the keys array will be the 
 *                      size of the tree.
 */
void** r2_rbtree_get_keys(const struct r2_rbtree *tree)
{
        if(tree->ncount == 0)
                return NULL;

        void **keys = malloc(sizeof(void *) * tree->ncount); 
        if(keys != NULL){
                struct r2_rbnode *root = r2_rbnode_inorder_first(tree->root); 
                r2_uint64 size = 0; 
                while(root != NULL){
                        keys[size] = root->key; 
                        ++size;
                        root = r2_rbnode_inorder_next(root);
                }
        }

        return keys;
}

/**
 * @brief               Gets values in tree.
 *                      Gets values in sorted order.
 * @param tree          RB Tree.
 * @return void**       Returns an array of values, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the values. N.B. It's the responsibilty of the caller to free values array
 *                      by calling free function with the values array. The size of the values array will be the 
 *                      size of the tree.
 */
void** r2_rbtree_get_values(const struct r2_rbtree *tree)
{
        if(tree->ncount == 0)
                return NULL;

        void **values= malloc(sizeof(void *) * tree->ncount); 
        if(values != NULL){
                struct r2_rbnode *root = r2_rbnode_inorder_first(tree->root); 
                r2_int64 size = 0; 
                while(root != NULL){
                        values[size] = root->data; 
                        ++size;
                        root = r2_rbnode_inorder_next(root);
                }
        }

        return values;
}

/**
 * @brief                       Finds the root at index.
 *                              Indexing starts at zero.
 * @param tree                  RB Tree.
 * @param pos                   Index of node.
 * @return struct r2_rbnode*    Returns the root node at index, else NULL.
 */
struct r2_rbnode* r2_rbtree_at(struct r2_rbnode *root, r2_uint64 pos)
{ 
        if(pos >= root->ncount)
                return NULL; 

        ++pos;
        r2_int64 size = 0; 
        while(root != NULL){
                size = r2_rbnode_recalc_size(root->left) + 1;
                if(size == pos)
                        break;       
                else if(pos < size)
                        root = root->left;
                else{
                        pos  = pos - size;
                        root = root->right;
                }
        }

        return root;

}

/**
 * @brief               Finds all the nodes between lower and upper inclusively.
 * 
 *                      Returns a list. N.B Please free list when finished by calling r2_destroy_list(list, freekey);
 * @param tree          RB Tree.
 * @param lower         Lower bound.
 * @param upper         Upper bound.
 * @param action        Action to perform on node.
 * @param arg           Argument passed to action.
 * @return void **      Return a list with the keys between lower and upper inclusively. 
 */
struct r2_list*  r2_rbtree_range_query(const struct r2_rbtree *tree, void *lower, void *upper, r2_act action, void *arg)
{
        /*Empty tree doesn't have range.*/
        if(r2_rbtree_empty(tree) == TRUE)
                return NULL; 

        /*List to store keys in the range.*/
        struct r2_list *keys = r2_create_list(tree->kcmp, tree->kcpy, tree->fk);
        if(keys!= NULL){
                struct r2_rbnode *k1 = NULL; 
                struct r2_rbnode *root = tree->root; 
                /*Search for lower. If lower doesn't exist get the last node on the search path.*/
                while(root != NULL){
                        k1 = root; 
                        if(tree->kcmp(lower, root->key) == 0)
                                break; 
                        else if(tree->kcmp(lower, root->key) > 0 )
                                root = root->right;
                        else 
                                root = root->left;
                }

                
                void *key = NULL; 
                while(k1 != NULL && tree->kcmp(k1->key, lower) >= 0 && tree->kcmp(k1->key, upper) <= 0){
                        if(action != NULL)
                                action(k1, arg);
                        
                        
                        key  = tree->kcpy != NULL? tree->kcpy(k1->key): k1->key;
                        keys =  r2_list_insert_at_back(keys, key);
                        k1   =  r2_rbnode_successor(k1);
                }
        }
        
       return keys; 
}

/**
 * @brief                Calculates the depth or level of a subtree.
 * 
 * @param root           RB Tree root.
 * @return r2_int64      Returns the depth of the root.
 */
r2_uint64 r2_rbnode_level(const struct r2_rbnode *root)
{
        r2_uint64  level = 0; 
        root = root->parent;

        while(root != NULL){
                ++level; 
                root = root->parent; 
        }

        return level;

}

/**
 * @brief              Helper function to calculate the size of a tree after balancing procedures.      
 * 
 * @param root         Root.
 * @return r2_int64    Returns the size of the tree.
 */
static r2_uint64 r2_rbnode_recalc_size(const struct r2_rbnode *root)
{
        r2_uint64 size = 0;
        if(root != NULL){
                if(root->left != NULL)
                        size +=  root->left->ncount; 

                if(root->right != NULL)
                        size +=  root->right->ncount;
                
                ++size; /*Adds one for the root*/
        }
        return size; 
}

/**
 * @brief               Helper function to check if a node is red.
 * 
 * @return r2_uint16    Returns TRUE when node is red, otherwise FALSE.
 */
static r2_uint16 r2_rbnode_is_red(const struct r2_rbnode *root)
{
        r2_uint16 result = FALSE;
        if(root != NULL)
                result = root->color == RED? TRUE : FALSE; 

        return result;
}

/**
 * @brief               Transplants a root.
 * 
 * @param tree          RB Tree.
 * @param root          The root that will be transplanted.
 */
static void r2_rbtree_restructure(struct r2_rbtree *tree, struct r2_rbnode *root, struct r2_rbnode *child)
{  
        struct r2_rbnode *parent = root->parent;
        if(parent != NULL){
                if(parent->right == root)
                        parent->right = child; 
                else
                        parent->left  = child;
                
                parent->ncount   = r2_rbnode_recalc_size(parent); 
        }else {
                tree->root    = child;
                tree->ncount  = r2_rbnode_recalc_size(child);
        }

         if(child != NULL)
                child->parent = parent;
}

/**
 * @brief                       Creates a copy of tree.
 *                              
 * @return struct r2_avltree*   Returns a copy RB tree.
 */
struct r2_rbtree *r2_rbtree_copy(const struct r2_rbtree *source)
{
        if(source->kcmp == NULL)
                return NULL;

        
        struct r2_rbtree *dest = r2_create_rbtree(source->kcmp, source->dcmp, source->kcpy, source->dcpy, source->fk, source->fd);
        if(dest != NULL){
                struct r2_list   *list = r2_create_list(source->kcmp, source->kcpy, source->fk);
                if(list != NULL){
                        struct r2_rbnode *root = NULL;
                        struct r2_listnode *front = NULL;
                        list = r2_list_insert_at_back(list, source->root);
                        void *key  = NULL; 
                        void *data = NULL;
                        while(r2_list_empty(list) != TRUE){
                                front = r2_listnode_first(list);
                                root  = front->data;
                                list  = r2_list_delete_at_front(list);
                                if(root != NULL){
                                        if(root->left != NULL)
                                                list  = r2_list_insert_at_back(list, root->left); 
                                        if(root->right != NULL)
                                                list  = r2_list_insert_at_back(list, root->right); 

                                        key  = root->key; 
                                        data = root->data;
                                        if(source->kcpy != NULL && source->dcpy != NULL){
                                                key  = source->kcpy(key);
                                                if(key == NULL){
                                                        dest = r2_destroy_rbtree(dest);
                                                        break;
                                                } 
                                                if(data != NULL){
                                                        data = source->dcpy(data);
                                                        if(data == NULL){
                                                                dest = r2_destroy_rbtree(dest);
                                                                break;
                                                        }
                                                }                                                
                                        }
                                        r2_rbtree_insert(dest, key, data);
                                }    
                        } 
                        r2_destroy_list(list); 
                }          
        }
        return dest;
}

/**
 * @brief               Compares two rb trees. *WARNING FUNCTION WILL FAIL USING ON A COPIED TREE*
 *                      Does a shallow comparison when both callback functions are NULL, else deep comparison.
 * 
 * @param tree1         Tree 1
 * @param tree2         Tree 2
 * @return r2_uint16    Returns TRUE if equal, else FALSE.
 */
r2_uint16 r2_rbtree_compare(const struct r2_rbtree *tree1, const struct r2_rbtree *tree2)
{
        
        r2_uint16 result = FALSE;
        if(r2_rbtree_empty(tree1) == TRUE && r2_rbtree_empty(tree2) == TRUE)
                result = TRUE;
        else if(tree1->ncount == tree2->ncount){
                /*Compare trees using preorder traversal.*/
                struct r2_rbnode *root1 = tree1->root; 
                struct r2_rbnode *root2 = tree2->root;
                while(root1 != NULL && root2 != NULL){
                       
                        result = tree1->kcmp != NULL? (tree1->kcmp(root1->key, root2->key) == 0) :  root1->key == root2->key;
                        result &= tree1->dcmp != NULL? (tree1->dcmp(root1->data, root2->data) == 0) :  root1->data == root2->data;
                        if(result == FALSE)
                                break;
        

                        root1 =  r2_rbnode_preorder_next(root1); 
                        root2 =  r2_rbnode_preorder_next(root2);
                } 
        } 
        return result;
}



/**
 * @brief               Checks if red and black tree is empty.
 * 
 * @param tree          Red and black tree.
 * @return r2_uint16    Returns TRUE when tree is empty, otherwise FALSE.
 */
r2_uint16 r2_rbtree_empty(const struct r2_rbtree *tree){
        return tree->root == NULL && tree->ncount == 0;
} 

/**
 * @brief Helper function to free a node. 
 *        
 *        If r2_freedata is NULL it doesn't free the data portion of node. 
 */
static void r2_freenode(struct r2_rbnode *root, r2_fk freekey, r2_fd freedata)
{
        if(freedata != NULL)
                freedata(root->data);
        
        if(freekey != NULL)
                freekey(root->data);
        free(root);
}

static r2_int64 MAX(r2_int64 a, r2_int64 b) 
{ 
        return a > b? a : b; 
}