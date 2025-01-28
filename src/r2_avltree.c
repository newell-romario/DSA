#include "r2_avltree.h"
#include <stdlib.h>

/********************File scope functions************************/
static void r2_freenode(struct r2_avlnode *, r2_fk, r2_fd);
static r2_uint64 r2_avlnode_recalc_size(const struct r2_avlnode *);
static r2_int64 r2_avlnode_recalc_height(const struct r2_avlnode *); 
static r2_int64 MAX(r2_int64, r2_int64);
static r2_int64 r2_avlnode_calc_bf(const struct r2_avlnode *);
static void r2_avltree_restructure(struct r2_avltree *, struct r2_avlnode *, struct r2_avlnode *); 
static void r2_avlnode_right_rotation(struct r2_avltree *, struct r2_avlnode *); 
static void r2_avlnode_left_rotation(struct r2_avltree *, struct r2_avlnode *); 
static void r2_avltree_rebalance(struct r2_avltree *, struct r2_avlnode *);
/********************File scope functions************************/


/**
 * @brief               Calculates the height of the tree recursively.          
 *                     
 *                      Please call seldomly since it's expensive to calculate the height of the tree recursively.
 *                      The height of tree can be read from root->height without processing the tree.
 * 
 * @param root          Root.
 * @return r2_uint64    Returns the height of the tree, else -1 for empty trees.
 */
r2_int64 r2_avltree_height(const struct r2_avlnode *root)
{
        if(root == NULL)
                return -1; 

        r2_int64 left_height  = r2_avltree_height(root->left) + 1;  
        r2_int64 right_height = r2_avltree_height(root->right) + 1;
        return MAX(left_height, right_height);                
}

/**
 * @brief               Calculates the depth or level of a subtree.
 *                      
 * @param root          Root.
 * @return r2_uint64    Returns the depth/level of a tree.
 */
r2_uint64  r2_avltree_level(const struct r2_avlnode *root)
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
 * @brief               Calculates the size of the tree recursively.
 *                      
 *                      Please call seldomly since it's expensive to calculate the size of the tree recursively.
 *                      The size of tree can be read from root->ncount without processing the tree.
 *                      
 * @param root          Root.
 * @return r2_uint64    Returns the size of the tree. 
 */
r2_uint64 r2_avltree_size(const struct r2_avlnode *root)
{
        if(root == NULL)
                return 0; 
        
        r2_uint64 left_size  = r2_avltree_size(root->left); 
        r2_uint64 right_size = r2_avltree_size(root->right);
        return left_size + right_size + 1;           
}

/**
 * @brief               Checks if AVL tree is empty.
 * 
 * @param tree          AVL Tree.
 * @return r2_uint16    Returns TRUE if empty, else FALSE.
 */
r2_uint16 r2_avltree_empty(const struct r2_avltree *tree)
{
        return tree->root == NULL && tree->ncount == 0;
}


/**
 * @brief                               Creates an empty AVL node.
 * 
 * @return struct r2_avlnode*           Returns AVL node, else NULL.
 */
struct r2_avlnode* r2_create_avlnode()
{
        struct r2_avlnode *node = malloc(sizeof(struct r2_avlnode));
        if(node != NULL){
                node->key       = NULL;
                node->data      = NULL;
                node->ncount    = 1; 
                node->height    = 0; 
                node->left      = NULL; 
                node->right     = NULL; 
                node->parent    = NULL;  
        }
        return node; 
}

/**
 * @brief                          Creates an empty AVL Tree.                                         
 * 
 * @param kcmp                     A comparison callback function for key.
 * @param dcmp                     A comparison callback function for data.
 * @param kcpy                     A callback function to copy key.
 * @param dcpy                     A callback function to copy values.
 * @param fk                       A callback function that release memory used by key.
 * @param fd                       A callback function that release memory used by data.
 * @return struct r2_avltree*      Returns an empty AVL Tree, else NULL. 
 */
struct r2_avltree* r2_create_avltree(r2_cmp kcmp, r2_cmp dcmp, r2_cpy kcpy, r2_cpy dcpy, r2_fk fk, r2_fd fd)
{
        struct r2_avltree* tree = malloc(sizeof(struct r2_avltree)); 
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
        return tree; 
}

/**
 * @brief                       Destroys an AVL tree.
 *                              
 * @param tree                  AVL Tree.
 * @return struct r2_avltree*   Returns NULL whenever AVL Tree is destroyed successfully.
 */
struct r2_avltree* r2_destroy_avltree(struct r2_avltree *tree)
{
        struct r2_avlnode *root  = r2_avlnode_postorder_first(tree->root);
        struct r2_avlnode *oldroot  = root; 
        while(root != NULL){
                oldroot = root;
                root = r2_avlnode_postorder_next(root);
                r2_freenode(oldroot, tree->fk, tree->fd); 
        }

        free(tree); 
        return NULL;
} 

/**
 * @brief                             Returns the minimum node in tree.
 * 
 * @param root                        Root.
 * @return struct r2_avlnode*         Returns minimum node, else NULL.
 */
struct r2_avlnode* r2_avlnode_min(struct r2_avlnode *root)
{
        while(root != NULL && root->left != NULL)
                root = root->left;

        return root; 
}


/**
 * @brief                               Returns the maximum node in tree.
 * 
 * @param root                          Root.
 * @return struct r2_avlnode*           Returns maximum node, else NULL.
 */
struct r2_avlnode* r2_avlnode_max(struct r2_avlnode *root)
{
        while(root != NULL && root->right != NULL)
                root = root->right;

        return root;
}

/**
 * @brief                                Returns the node before the root in an inorder traversal.
 * 
 * @param root                           Root.
 * @return struct r2_avlnode*            Returns the predecessor of root, else NULL.
 */
struct r2_avlnode* r2_avlnode_predecessor(struct r2_avlnode *root)
{
        struct r2_avlnode *predecessor = NULL;
        if(root->left != NULL)
                predecessor = r2_avlnode_max(root->left);
        else{
            predecessor = root->parent; 
            while(predecessor != NULL && predecessor->left == root){
                root        = predecessor;
                predecessor = predecessor->parent; 
            }
        }
        return predecessor;  
}

/**
 * @brief                       Returns the node after the root in an inorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the successor of the root, else NULL.
 */
struct r2_avlnode* r2_avlnode_successor(struct r2_avlnode *root)
{
        struct r2_avlnode *successor = NULL;
        if(root->right != NULL)
                successor = r2_avlnode_min(root->right);
        else{
                successor = root->parent; 
                while(successor != NULL && successor->right == root){
                        root      = successor; 
                        successor = successor->parent;
                }
        }
        return successor;
}

/**
 * @brief               Performs a right rotation
 * 
 * @param root          Root. 
 */
static void r2_avlnode_right_rotation(struct r2_avltree *tree, struct r2_avlnode *root)
{
        struct r2_avlnode *parent      = root->parent;
        struct r2_avlnode *grandparent = parent->parent;

        parent->left = root->right;
        if(parent->left != NULL)
                parent->left->parent = parent; 
        
        parent->ncount   = r2_avlnode_recalc_size(parent);
        parent->height   = r2_avlnode_recalc_height(parent);

        root->right = parent; 
        root->right->parent = root;
        root->ncount = r2_avlnode_recalc_size(root);
        root->height = r2_avlnode_recalc_height(root);

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root;                        
                
                grandparent->ncount = r2_avlnode_recalc_size(grandparent);
                grandparent->height = r2_avlnode_recalc_height(grandparent);
        }


        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root = root; 
                tree->ncount  = root->ncount;
        }
}


/**
 * @brief       Performs a left rotation on root.
 * 
 * @param root  Root.
 */
static void r2_avlnode_left_rotation(struct r2_avltree *tree, struct r2_avlnode *root)
{
        struct r2_avlnode *parent      = root->parent;
        struct r2_avlnode *grandparent = parent->parent;

        parent->right = root->left;
        if(parent->right != NULL)
                parent->right->parent = parent; 
        
        parent->ncount = r2_avlnode_recalc_size(parent);
        parent->height = r2_avlnode_recalc_height(parent);

        root->left = parent; 
        root->left->parent = root;
        root->ncount = r2_avlnode_recalc_size(root);
        root->height = r2_avlnode_recalc_height(root);

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root;                        
                
                grandparent->ncount = r2_avlnode_recalc_size(grandparent);
                grandparent->height = r2_avlnode_recalc_height(grandparent);
        }

        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root = root; 
                tree->ncount  = root->ncount ;
        }
}


/**
 * @brief                       Inserts key and accompanying data. 
 *                              Inserts a new key and accompanying data into the tree.
 *                              Whenever a duplicate key is inserted, the insertion basically
 *                              acts like a replace function replacing the data.
 * 
 * @param tree                  AVL Tree.
 * @param key                   Key.
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE when successfully inserted, else FALSE. 
 */
r2_uint16 r2_avltree_insert(struct r2_avltree *tree, void *key, void *data)
{
        struct r2_avlnode **root  = &tree->root; 
        struct r2_avlnode *parent = NULL;
        r2_int64 result = 0;
        r2_uint16 SUCCESS = FALSE;
        while(*root != NULL){
                parent = *root; 
                result = tree->kcmp(key, parent->key); 
                if(result > 0)
                        root = &((*root)->right); 
                else if(result < 0)
                        root = &((*root)->left);
                else{
                        (*root)->data = data;
                        SUCCESS = TRUE;
                        return SUCCESS; 
                }
                        
        }

        struct r2_avlnode *temp = r2_create_avlnode(); 
        if(temp != NULL){   
                temp->key    = key; 
                temp->data   = data; 
                temp->parent = parent;
                *root = temp;
                r2_avltree_rebalance(tree, *root);
                SUCCESS = TRUE;
        }
        return SUCCESS;
}


/**
 * @brief                       Finds key in tree.
 * 
 * @param tree                  AVL Tree.
 * @param key                   Key.
 * @return struct r2_avlnode*   Returns the subtree which contains the key, else NULL.
 */
struct r2_avlnode* r2_avltree_search(struct r2_avltree *tree, const void *key)
{
        struct r2_avlnode *root = tree->root; 
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
 * @brief               Transplants a root.
 * 
 * @param tree          AVL Tree.
 * @param root          Root.
 */
static void r2_avltree_restructure(struct r2_avltree *tree, struct r2_avlnode *root, struct r2_avlnode *child)
{  
        struct r2_avlnode *parent = root->parent;
        if(parent != NULL){
                if(parent->right == root)
                        parent->right = child; 
                else
                        parent->left  = child;

                parent->ncount   = r2_avlnode_recalc_size(parent); 
                parent->height   = r2_avlnode_recalc_height(parent);
        }else {
                tree->root   = child;
                tree->ncount = r2_avlnode_recalc_size(child);
        }

         if(child != NULL)
                child->parent = parent;
}

/**
 * @brief                       Deletes a key from the tree if it exists.
 * 
 * @param tree                  AVL Tree. 
 * @param key                   Key.
 * @return r2_uint16            Returns TRUE when successfully deleted, else FALSE. 
 */
r2_uint16 r2_avltree_delete(struct r2_avltree *tree, void*key)
{
        /*Node to be deleted.*/
        struct r2_avlnode *root = r2_avltree_search(tree, key);
        r2_uint16 SUCCESS =  FALSE;
        if(root != NULL){
                struct r2_avlnode *parent = NULL;
                if(root->right == NULL)
                        r2_avltree_restructure(tree, root, root->left);
                else if(root->left == NULL)
                        r2_avltree_restructure(tree, root, root->right); 
                else{
                        struct r2_avlnode *successor = r2_avlnode_successor(root);
                        root->key  = successor->key;
                        root->data = successor->data;
                        root       = successor;
                        r2_avltree_restructure(tree, root, root->right); 
                }
                parent = root->parent;
                r2_freenode(root, tree->fk, tree->fd);
                r2_avltree_rebalance(tree, parent);  
                SUCCESS = TRUE;               
        }
        return SUCCESS;
}



/**
 * @brief               Rebalances an AVL Tree after insertion or deletion.
 * 
 * @param tree          AVL Tree.
 * @param root          The node which the rebalancing starts from. 
 */
static void r2_avltree_rebalance(struct r2_avltree *tree, struct r2_avlnode *root)
{
        r2_int64 balance_factor = 0;
        while(root != NULL){
                root->ncount    = r2_avlnode_recalc_size(root); 
                root->height    = r2_avlnode_recalc_height(root);
                balance_factor  = r2_avlnode_calc_bf(root); 

                if(balance_factor > 1){
                        root = root->left;
                        balance_factor = r2_avlnode_calc_bf(root); 
                        if(balance_factor < 0 ){
                                root = root->right; 
                                r2_avlnode_left_rotation(tree, root);
                        }      
                        r2_avlnode_right_rotation(tree, root);
                }else if(balance_factor < -1)
                {
                        root =  root->right;
                        balance_factor = r2_avlnode_calc_bf(root); 
                        if(balance_factor > 0){  
                                root = root->left; 
                                r2_avlnode_right_rotation(tree, root); 
                        }     
                        r2_avlnode_left_rotation(tree, root);
                }
                root = root->parent;
        }     

        tree->ncount = r2_avlnode_recalc_size(tree->root); 

}  

/**
 * @brief                       Locates a node based on index.
 *                              Example the node at index zero is the smallest node in the 
 *                              tree. N.B  Indexing starts at zero.
 *                             
 * @param tree                  AVL Tree.
 * @param pos                   Pos.
 * @return struct r2_avlnode*   Returns the root node at index, else NULL.
 */
struct r2_avlnode* r2_avltree_at(struct r2_avlnode *root, r2_uint64 pos)
{
        if(pos >= root->ncount)
                return NULL; 
                
        ++pos;
        r2_int64 size = 0; 
        while(root != NULL){
                size = r2_avlnode_recalc_size(root->left) + 1;
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
 * @brief               Performs an inorder traversal and an action for each node.
 *                      
 * @param tree          AVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Arguments passed to action.
 */
void r2_avltree_inorder(struct r2_avlnode *root, r2_act action, void *arg)
{
        
        struct r2_avlnode *old_root = root; 
        struct r2_avlnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_avlnode_min(root); 
        while(root != NULL){
                action(root, arg);
                root = r2_avlnode_inorder_next(root);
        }

        old_root->parent = parent;
}

/**
 * @brief               Performs an postorder traversal and an action for each node.
 * 
 * @param tree          AVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Arguments passed to action.
 */
void r2_avltree_postorder(struct r2_avlnode *root, r2_act action, void *arg)
{
        struct r2_avlnode *old_root = root; 
        struct r2_avlnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_avlnode_postorder_first(root);
        while(root != NULL){
                action(root, arg);
                root = r2_avlnode_postorder_next(root); 
        }

        old_root->parent = parent;
}


/**
 * @brief               Performs an preorder traversal and an action for each node.
 * 
 * @param tree          AVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Argument passed to action.
 */
void r2_avltree_preorder(struct r2_avlnode *root, r2_act action, void *arg)
{
        struct r2_avlnode *old_root = root; 
        struct r2_avlnode *parent   = root->parent; 
        root->parent = NULL;

        while(root != NULL){
                action(root, arg); 
                root = r2_avlnode_preorder_next(root);
        }

        old_root->parent = parent;
}


/**
 * @brief                       Finds the next node in an preorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the next node in an preorder traversal, else NULL.
 */
struct r2_avlnode*  r2_avlnode_preorder_next(struct r2_avlnode *root)
{
        if(root->left != NULL)
                return root->left; 
        
        if(root->right != NULL)
                return root->right;
                
        struct r2_avlnode *parent = root->parent; 
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
                        root = NULL; 
                return root; 
        }
}

/**
 * @brief                       Finds the next node in an postorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the next node in an postorder traversal, else NULL.
 */
struct r2_avlnode* r2_avlnode_postorder_next(struct r2_avlnode *root)
{
        struct r2_avlnode *parent = root->parent; 
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
 * @brief                       Finds the next node in an inorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the next node in an inorder traversal, else NULL.
 */
struct r2_avlnode* r2_avlnode_inorder_next(struct r2_avlnode *root)
{
        return r2_avlnode_successor(root);
}



/**
 * @brief                       Returns the first node in an order traversal which is the miniumum node in the subtree.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the first node.
 */
struct r2_avlnode* r2_avlnode_inorder_first(struct r2_avlnode *root)
{
        return r2_avlnode_min(root);
}

/**
 * @brief                       Returns the first node in an postorder traversal which is the miniumum node in the subtree.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the first node.
 */
struct r2_avlnode*  r2_avlnode_postorder_first(struct r2_avlnode *root)
{
        if(root != NULL){
                while(root->left != NULL)
                        root = root->left; 

                while(root->right != NULL)
                        root = root->right;    
        }
       return root;
}

/**
 * @brief                       Returns the first node in an preorder traversal which is the miniumum node in the subtree.
 * 
 * @param root                  Root.
 * @return struct r2_avlnode*   Returns the first node.
 */
struct r2_avlnode*  r2_avlnode_preorder_first(struct r2_avlnode *root)
{
        return  root;
}

/**
 * @brief               Gets keys in an AVL Tree.
 *                      Gets the keys in sorted order.
 * @param tree          AVL Tree.
 * @return void**       Returns an array of keys, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the keys. N.B. It's the responsibilty of the caller to free keys array
 *                      by calling free function with the keys array. The size of the keys array will be the 
 *                      size of the tree.
 */
void** r2_avltree_get_keys(const struct r2_avltree *tree)
{
        if(tree->ncount <= 0)
                return NULL;

        void **keys = malloc(sizeof(void *) * tree->ncount); 
        if(keys != NULL){
                struct r2_avlnode *root = r2_avlnode_inorder_first(tree->root); 
                r2_int64 size = 0; 
                while(root != NULL){
                        keys[size] = root->key; 
                        ++size;
                        root = r2_avlnode_inorder_next(root);
                }
        }

        return keys;
}

/**
 * @brief               Gets values in an avl tree.
 *                      Gets values in sorted order.
 * @param tree          AVL Tree.
 * @return void**       Returns an array of values, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the values. N.B. It's the responsibilty of the caller to free values array
 *                      by calling free function with the values array. The size of the values array will be the 
 *                      size of the tree.
 */
void** r2_avltree_get_values(const struct r2_avltree *tree)
{
        if(tree->ncount <= 0)
                return NULL;

        void **values= malloc(sizeof(void *) * tree->ncount); 
        if(values != NULL){
                struct r2_avlnode *root = r2_avlnode_inorder_first(tree->root); 
                r2_int64 size = 0; 
                while(root != NULL){
                        values[size] = root->data; 
                        ++size;
                        root = r2_avlnode_inorder_next(root);
                }
        }
        return values;
}

/**
 * @brief               Finds all the nodes between lower and upper inclusively.
 * 
 *                      Returns a list. N.B Please free list when finished by calling r2_destroy_list(list);
 * @param tree          AVL Tree.
 * @param lower         Lower bound.
 * @param upper         Upper bound.
 * @param action        Action to perform on node.
 * @param arg           Argument passed to action.
 * @return r2_list*     Return a list with the keys between lower and upper inclusively. 
 */
struct r2_list* r2_avltree_range_query(const struct r2_avltree *tree, void *lower, void *upper, r2_act action, void *arg)
{
        /*Empty tree doesn't have range.*/
        if(r2_avltree_empty(tree) == TRUE)
                return NULL; 

        /*List to store keys in the range.*/
        struct r2_list *keys = r2_create_list(tree->kcmp, tree->kcpy, tree->fk);
        if(keys != NULL){
                struct r2_avlnode *k1   = NULL; 
                struct r2_avlnode *root = tree->root; 
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

                        key = tree->kcpy != NULL? tree->kcpy(k1->key): k1->key;
                        if(key == NULL){
                                keys =  r2_destroy_list(keys);
                                break;    
                        }
                        if(r2_list_insert_at_back(keys, key) == FALSE){
                               keys =  r2_destroy_list(keys);
                               break;
                        }
                        k1 =  r2_avlnode_successor(k1);
                }
        }
        
       return keys; 
}

/**
 * @brief               Compares two avl trees.
 *                      Comparison is only successfully when both trees have the same preorder traversal i.e. the same structure.       
 * 
 * 
 * @param tree1         Tree 1
 * @param tree2         Tree 2
 * @return r2_uint16    Returns TRUE if equal, else FALSE.
 */
r2_uint16 r2_avltree_compare(const struct r2_avltree *tree1, const struct r2_avltree *tree2)
{
        r2_uint16 result = FALSE;
        if(r2_avltree_empty(tree1) == TRUE && r2_avltree_empty(tree2) == TRUE)
                result = TRUE;
        else if(tree1->ncount == tree2->ncount){
                /*Compare trees using preorder traversal.*/
                struct r2_avlnode *root1 = tree1->root; 
                struct r2_avlnode *root2 = tree2->root;
                while(root1 != NULL && root2 != NULL){
                       
                        result = tree1->kcmp != NULL? (tree1->kcmp(root1->key, root2->key) == 0) :  root1->key == root2->key;
                        result &= tree1->dcmp != NULL? (tree1->dcmp(root1->data, root2->data) == 0) :  root1->data == root2->data;
                        if(result == FALSE)
                                break;
        
                        root1 =  r2_avlnode_preorder_next(root1); 
                        root2 =  r2_avlnode_preorder_next(root2);
                } 
        } 
        return result;
}

/**
 * @brief                       Creates a copy of tree.
 *                              Does a shallow copy when both copy functions are NULL, else deep copy.
 * 
 * @param source                AVL Tree.
 * @return struct r2_avltree*   Returns a copy avl tree.
 */
struct r2_avltree *r2_avltree_copy(const struct r2_avltree *source)
{
        if(source->kcmp == NULL)
                return NULL;

        struct r2_avltree *dest = r2_create_avltree(source->kcmp, source->dcmp, source->kcpy, source->dcpy, source->fk, source->fd);
        if(dest != NULL){
                struct r2_avlnode *root = source->root;
                void *key; 
                void *data;
                while(root != NULL){
                        key = root->key; 
                        data = root->data;
                        if(source->kcpy != NULL && source->dcpy != NULL){
                                key  = source->kcpy(key);
                                if(data != NULL){
                                        data = source->dcpy(data);
                                        if(data == NULL){
                                                dest = r2_destroy_avltree(dest);
                                                break;
                                        }

                                } 
                                if(key == NULL){
                                        dest = r2_destroy_avltree(dest);
                                        break;
                                }
                        }
                        r2_avltree_insert(dest, key, data);
                        root  = r2_avlnode_preorder_next(root);
                } 
        }
        return dest;
}


static r2_int64 MAX(r2_int64 a, r2_int64 b) 
{ 
        return a > b? a : b; 
}

/**
 * @brief Helper function to free a node. 
 *        
 *        If r2_freedata is NULL it doesn't free the data portion of node. 
 */
static void r2_freenode(struct r2_avlnode *root, r2_fk freekey, r2_fd freedata)
{
        if(freedata != NULL)
                freedata(root->data);
        
        if(freekey != NULL)
                freekey(root->data);

        free(root);
}

/**
 * @brief              Helper function to calculate the size of a tree after balancing procedures.      
 * 
 * @param root         Root.
 * @return r2_int64    Returns the size of the tree.
 */
static r2_uint64 r2_avlnode_recalc_size(const struct r2_avlnode *root)
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
 * @brief              Helper function to calculate the height of a tree after balancing procedures.          
 * 
 * @param root         Rroot.
 * @return r2_int64    Returns the height of tree.
 */
static r2_int64 r2_avlnode_recalc_height(const struct r2_avlnode *root)
{
        r2_int64 height = -1;
        if(root != NULL){
                if(root->left != NULL)
                        height = MAX(height, root->left->height);
                
                if(root->right != NULL)
                        height = MAX(height, root->right->height);

                ++height;
        }

        return height;
}

/**
 * @brief               Calculates the balance factor of the root.
 * 
 * @param root          Root.
 * @return r2_int64     Returns the balance factor of the root.
 */
static r2_int64 r2_avlnode_calc_bf(const struct r2_avlnode *root)
{ 
        r2_int64 left_height = -1; 
        r2_int64 right_heigt = -1;
        if(root != NULL){
                if(root->left != NULL)
                        left_height = root->left->height;
                
                if(root->right != NULL)
                        right_heigt =  root->right->height;
        }

 
        return (left_height + 1) - (right_heigt + 1);
}