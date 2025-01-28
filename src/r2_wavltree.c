#include "r2_wavltree.h"
#include <stdlib.h>
#include <stdio.h>

/********************File scope functions************************/
static void r2_freenode(struct r2_wavlnode *, r2_fd, r2_fk);
static r2_int64 r2_wavlnode_recalc_size(const struct r2_wavlnode *root);
static r2_int64 r2_wavlnode_is_leaf(const struct r2_wavlnode *);
static r2_int64 MAX(r2_int64 a, r2_int64 b);
static r2_int64 r2_wavlnode_rank_diff(const struct r2_wavlnode *, const struct r2_wavlnode *root);
static r2_int16 r2_wavlnode_has_child(const struct r2_wavlnode *, enum CHILD_TYPE);
static struct r2_wavlnode* r2_wavlnode_get_sibling(const struct r2_wavlnode *);
static void r2_wavltree_restructure(struct r2_wavltree *, struct r2_wavlnode *, struct r2_wavlnode *); 
static void r2_wavlnode_right_rotation(struct r2_wavltree *, struct r2_wavlnode *); 
static void r2_wavlnode_left_rotation(struct r2_wavltree *, struct r2_wavlnode *); 
static void r2_wavltree_insert_rebalance(struct r2_wavltree *, struct r2_wavlnode *);
static void r2_wavltree_delete_rebalance(struct r2_wavltree *, struct r2_wavlnode *);
/********************File scope functions************************/



r2_int64  r2_wavltree_height(const struct r2_wavlnode *root)
{
        if(root == NULL)
                return -1; 

        r2_int64 left_height  = r2_wavltree_height(root->left) + 1;  
        r2_int64 right_height = r2_wavltree_height(root->right) + 1;
        return MAX(left_height, right_height);    
}

/**
 * @brief                               Creates a wavl node.
 * 
 * @return struct r2_wavlnode*          Returns an empty wavl node, else NULL. 
 */
struct r2_wavlnode* r2_create_wavlnode()
{
        struct r2_wavlnode* root = malloc(sizeof(struct r2_wavlnode)); 
        if(root != NULL){
                root->key       = NULL; 
                root->data      = NULL; 
                root->ncount    = 1; 
                root->rank      = 0;
                root->parent    = NULL; 
                root->left      = NULL; 
                root->right     = NULL;
        }

        return root; 
}

/**
 * @brief               Checks if WAVL Tree is empty.
 * 
 * @param tree          WAVL tree.
 * @return r2_uint16    Returns TRUE when tree is empty, otherwise FALSE.
 */
r2_uint16 r2_wavltree_empty(const struct r2_wavltree *tree)
{
        return tree->root == NULL && tree->ncount == 0; 
}


/**
 * @brief                       Creates a WAVL tree.
 *
 * @param kcmp                  A comparison callback function for key.
 * @param dcmp                  A comparison callback function for data.
 * @param kcpy                  A callback function to copy key.
 * @param dcpy                  A callback function to copy values.
 * @param fk                    A callback function that release memory used by key.
 * @param fd                    A callback function that release memory used by data.
 * @return struct r2_wavltree*  Returns an empty WAVL tree, else NULL.
 */
struct r2_wavltree* r2_create_wavltree(r2_cmp kcmp, r2_cmp dcmp, r2_cpy kcpy, r2_cpy dcpy, r2_fk fk, r2_fd fd)
{
        struct r2_wavltree *tree = malloc(sizeof(struct r2_wavltree));
        if(tree != NULL){
                tree->root = NULL; 
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
 * @brief                        Destroys an WAVL tree.
 * 
 * @param tree                   WAVL Tree.
 * @return struct r2_wavltree*   Returns NULL whenever WAVL tree is deleted successfully.
 */
struct r2_wavltree* r2_destroy_wavltree(struct r2_wavltree *tree)
{
        struct r2_wavlnode *root     = r2_wavlnode_postorder_first(tree->root);
        struct r2_wavlnode *oldroot  = root; 

        while(root != NULL){
                oldroot = root;
                root = r2_wavlnode_postorder_next(root);
                r2_freenode(oldroot, tree->fd, tree->fk); 
        }

     
        free(tree); 
        return NULL;
}

/**
 * @brief                        Returns the first node after the root in an inorder traversal.
 * 
 * @param root                   Root.
 * @return struct r2_wavlnode*   Returns the successor of the root, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_successor(struct r2_wavlnode *root)
{
        struct r2_wavlnode *successor = NULL; 
        if(root->right != NULL)
                successor = r2_wavlnode_min(root->right);
        else{
                successor = root->parent;
                while(successor != NULL && successor->right == root){
                        root = successor;
                        successor = successor->parent;
                } 
        }

        return successor;
}

/**
 * @brief                        Returns the largest node before the root in an inorder traversal.
 * 
 * @param root                   Root.
 * @return struct r2_wavlnode*   Returns the successor of the root, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_predecessor(struct r2_wavlnode *root)
{
        struct r2_wavlnode *predecessor = NULL; 
        if(root->left != NULL)
                predecessor = r2_wavlnode_max(root->left);
        else{
                predecessor = root->parent;
                while(predecessor != NULL && predecessor->left == root){
                        root = predecessor;
                        predecessor = predecessor->parent;
                } 
        }

        return predecessor;
}

/**
 * @brief                             Returns the minimum node in a sub tree.
 * 
 * @param root                        Root.
 * @return struct r2_wavlnode*        Returns the minimum node in the subtree, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_min(struct r2_wavlnode *root)
{
        while(root != NULL && root->left != NULL)
                root = root->left;

        return root; 
}

/**
 * @brief                             Returns the maximum node in a sub tree.
 * 
 * @param root                        Tree root.
 * @return struct r2_wavlnode*        Returns the maximum node in the subtree, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_max(struct r2_wavlnode *root)
{
        while(root != NULL && root->right != NULL)
                root = root->right;

        return root; 
}

/**
 * @brief                Calculates the depth or level of a subtree.
 * 
 * @param root           Root.
 * @return r2_int64      Returns the depth of the root.
 */
r2_uint64 r2_wavlnode_level(const struct r2_wavlnode *root)
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
 * @brief                               Performs a search for a specific key.
 * 
 * @param tree                          WAVL Tree.
 * @param key                           Key.
 * @return struct r2_wavlnode*          Returns the subtree which contains the key, else NULL.
 */
struct r2_wavlnode* r2_wavltree_search(struct r2_wavltree *tree, const void *key)
{
        struct r2_wavlnode *root = tree->root; 
        r2_int64 result = 0; 
        while(root != NULL){
                #ifdef PROFILE_TREE
                        ++tree->num_comparisons;
                #endif 
                result = tree->kcmp(key, root->key); 
                if(result < 0)
                       root = root->left;
                else if(result > 0)
                        root = root->right;
                else
                        break;              
        }

        return root;
}


/**
 * @brief                       Inserts a key and accompanying data into the tree. 
 *                              
 * 
 * @param tree                  WAVL tree.
 * @param key                   Key.
 * @param data                  Data.
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE. 
 */
r2_uint16 r2_wavltree_insert(struct r2_wavltree *tree, void *key, void *data)
{
        r2_int64 result = 0;
        struct r2_wavlnode **root = &tree->root;
        struct r2_wavlnode *parent = NULL; 
        r2_uint16 SUCCESS = FALSE;
        while(*root != NULL){
                parent = *root; 
                result = tree->kcmp(key, parent->key); 
                if(result > 0)
                        root = &(*root)->right; 
                else if(result < 0)
                        root = &(*root)->left;
                else{
                        (*root)->data = data; 
                        SUCCESS = TRUE;
                        return SUCCESS;
                } 
        }

        struct r2_wavlnode *temp = r2_create_wavlnode();
        if(temp != NULL){
                temp->key    = key;
                temp->data   = data;
                temp->parent = parent;
                *root = temp; 
                r2_wavltree_insert_rebalance(tree, *root);
                tree->ncount = r2_wavlnode_recalc_size(tree->root);
        }
        
        return SUCCESS;
}

/**
 * @brief               Peforms bottom up rebalancing on WAVL Tree.
 *                      Uses the bottom up rebalancing algorithm from
 *                      Rank Balanced Trees by Bernard Haeupler et al (2014) in 
 *                      section 4. Please see section 4 for detailed description
 *                      of bottom up rebalancing after insertion.
 * @param tree          WAVL Tree
 * @param root          Node where rebalancing will start.
 */
static void r2_wavltree_insert_rebalance(struct r2_wavltree *tree, struct r2_wavlnode *root)
{
        struct r2_wavlnode *parent = root->parent; 
        r2_int64 left_rdiff    = 0 ;
        r2_int64 right_rdiff   = 0 ;
        
        /**
         * @brief       Continues looping when parent has a zero child.
         *              A 0 child invalidates the rank difference rule for WAVL trees. 
         */
        while(parent != NULL && r2_wavlnode_has_child(parent, ZERO_CHILD) == TRUE){
                left_rdiff  = r2_wavlnode_rank_diff(parent, parent->left); 
                right_rdiff = r2_wavlnode_rank_diff(parent, parent->right);
                
                /**
                 * @brief Case 1: Whenever we have parent being a (1,0) or (0,1) node
                 *                we have violated the rule where all rank differences 
                 *                can only be 1 or 2. To fix this we promote the 
                 *                parent which is basically increasing the rank by one.
                 *                
                 *                Then we continue at the parent checking for a violation.
                 */
                if(r2_wavlnode_has_child(parent, ONE_CHILD) && r2_wavlnode_has_child(parent, ZERO_CHILD)){
                        ++parent->rank;
                        root   = parent; 
                        parent = root->parent;
                }else 
                        /**
                         * @brief   
                         *      Case 2: 
                         *           Whenever we have the parent being a (2, 0) node we  have violated 
                         *           the rule where all rank differences can only be 1 or 2.  
                         *                  
                         *           We have two subcases to consider when this occurs:
                         *           Firstly, let the root be the child with rank difference 0. 
                         *                  a) Root has a child with rank difference 1. 
                         *                     We perform a double rotation whether it be left rotation then right
                         *                     rotation or vice versa.
                         *                  
                         *                 b) Root has 2 child or null. We perform a single rotation depending on the orientation.
                         *                                  
                         */
                        if(left_rdiff == 2 && right_rdiff == 0){
                                root = parent->right;
                                left_rdiff = r2_wavlnode_rank_diff(root, root->left);
                                if(left_rdiff == 1){
                                        --root->rank;
                                        root   = root->left;
                                        r2_wavlnode_right_rotation(tree, root);
                                        ++root->rank;
                                        parent = root->parent;
                                } 
                                r2_wavlnode_left_rotation(tree, root);
                                --parent->rank;
                                break;
                        }else{
                                root = parent->left;
                                right_rdiff = r2_wavlnode_rank_diff(root, root->right);
                                if(right_rdiff == 1){
                                        --root->rank;
                                        root = root->right;
                                        r2_wavlnode_left_rotation(tree, root);
                                        ++root->rank;
                                        parent = root->parent;
                                }

                                r2_wavlnode_right_rotation(tree, root);
                                --parent->rank;
                                break;

                        }

                root->ncount      = r2_wavlnode_recalc_size(root);
        } 

        /*Recalculates the size of a subtree*/
        while(root != NULL){
                root->ncount = r2_wavlnode_recalc_size(root);
                root = root->parent;
        }
}

/**
 * @brief                       Deletes a key from the tree if it exists.
 * 
 * @param tree                  WAVL Tree. 
 * @param key                   Key.
 * @return r2_uint16            Returns TRUE upon successful deletion, else FALSE. 
 */
r2_uint16  r2_wavltree_delete(struct r2_wavltree *tree, void *key)
{
        struct r2_wavlnode *root  = r2_wavltree_search(tree, key);
        struct r2_wavlnode *child = NULL; 
        r2_uint16 SUCCESS = FALSE;
        if(root != NULL){
                if(r2_wavlnode_is_leaf(root) == TRUE){
                        child = root; 
                        /**
                         * We turned root into a dummy node to help us in the deletion case. 
                         * DO NOT set root->parent = NULL. We need to be able to go up the tree.
                         * 
                         */
                        root->rank     = -1; 
                        root->ncount   = 0;
                        root->left     = NULL; 
                        root->right    = NULL; 
                }else if(root->left == NULL){
                        child = root->right; 
                        r2_wavltree_restructure(tree, root, child);
                }else if(root->right == NULL){
                        child = root->left; 
                        r2_wavltree_restructure(tree, root, child);
                }else{
                        struct r2_wavlnode *successor = r2_wavlnode_successor(root);
                        root->key  = successor->key; 
                        root->data = successor->data;
                        root = successor;
                        if(root->right == NULL){
                                child = root;
                                /**
                                 * We turned root into a dummy node to help us in the deletion case. 
                                 * DO NOT set root->parent = NULL. 
                                 */
                                root->rank      = -1; 
                                root->ncount    = 0;
                                root->left      = NULL; 
                                root->right     = NULL; 
                        }
                        else{
                                child = root->right;
                                r2_wavltree_restructure(tree, root, child);
                        }
                                
                }

                /*Rebalancing starts from root.*/
                r2_wavltree_delete_rebalance(tree, child);

                /*Detaches root from the tree since it's going to be deleted. */
                if(child == root)
                        r2_wavltree_restructure(tree, root, NULL); 

                /*Recalculates the size of a subtree*/
                while(child != NULL){
                        child->ncount = r2_wavlnode_recalc_size(child); 
                        child = child->parent;
                } 
                tree->ncount = r2_wavlnode_recalc_size(tree->root);
                /*Releases root memory*/
                r2_freenode(root, tree->fd, tree->fk);
                SUCCESS = TRUE;
        }

        return SUCCESS;
}

/**
 * @brief               Peforms bottom up rebalancing on WAVL Tree.
 *                      Uses the bottom up rebalancing algorithm from
 *                      Rank Balanced Trees by Bernard Haeupler et al (2014) in 
 *                      section 4. Please see section 4 for detailed description
 *                      of bottom up rebalancing after deletion.
 * @param tree          WAVL Tree
 * @param root          Node where rebalancing will start.
 */
static void r2_wavltree_delete_rebalance(struct r2_wavltree *tree, struct r2_wavlnode *root)
{
        /**
         * Case 1) Root is a (2,2) leaf which violates the WAVL rule. 
         *         Leaves in a WAVL tree is of rank 0 and a 1 node or (1, 1). 
         *         To fix this we demote the leaf which either fixes
         *         the issue or makes root a 3 child.        
         * 
         * If root is currently the dummy node  that means it's parent maybe a
         * leaf. N.B Root is the dummy node when it's rank is -1. We verify that 
         * parent is indeed a (2, 2) leaf with the check. 
         */
        struct r2_wavlnode *parent  = root->parent;
        r2_int64 left_rdiff    =  parent != NULL? r2_wavlnode_rank_diff(parent, parent->left): 0;
        r2_int64 right_rdiff   =  parent != NULL? r2_wavlnode_rank_diff(parent, parent->right): 0;
        if(root->rank == -1 && left_rdiff == 2 && right_rdiff == 2){
                --parent->rank;
                root = parent;
                parent  = root->parent;
        }
                
        
       
        
        struct r2_wavlnode *sibling = NULL; 
        /**
         * We need to determine if root is a 3 child.
         * The rest of the rebalncing depends on root being a 3 child.
         * This variable stores root child type.
         */
        r2_int64 root_ctype    = r2_wavlnode_rank_diff(parent, root);  
        r2_int64 sibling_rdiff = 0; 
        while(root_ctype == 3){
                sibling = r2_wavlnode_get_sibling(root);
                sibling_rdiff = r2_wavlnode_rank_diff(parent, sibling);
                left_rdiff    = r2_wavlnode_rank_diff(sibling, sibling->left); 
                right_rdiff   = r2_wavlnode_rank_diff(sibling, sibling->right);

                /**
                 *  Case 2)  When sibling is a 2 child. We demote siblings parent.
                 *           This either fixes the issue or propogates up the tree.
                 */
                if(sibling_rdiff == 2){
                        --parent->rank; 
                        root   = parent; 
                        parent = root->parent;
                }
                /**
                 * Case 3) When sibling  is a (2, 2) node. 
                 *         We demote both sibling and it's parent. This 
                 *         either fixes the issue or propogates up the tree.
                 */
                else if(left_rdiff == 2 && right_rdiff == 2){;
                        --sibling->rank;
                        --parent->rank;
                        root   = parent; 
                        parent = root->parent; 
                }else 
                /***
                 * Case 4 and Case 5
                 * When root is a 3 child we have violated a WAVL property. We fix this by
                 * doing rotations.
                 * 
                 * If sibling is the right child of parent and sibling right child is 
                 * a 1 child. 
                 */
                if(parent->right == sibling){
                        if(r2_wavlnode_rank_diff(sibling, sibling->right) == 1)
                        {
                                r2_wavlnode_left_rotation(tree, sibling);
                                ++sibling->rank;
                                --parent->rank;
                                /**
                                 * If root isn't a legit node then it's the dummy node 
                                 * which has a rank of -1. If the sibling of the dummy node is 
                                 * null then parent is really a leaf. Demote parent again.
                                 * 
                                 */
                                sibling = r2_wavlnode_get_sibling(root);
                                if(sibling == NULL && root->rank == -1)
                                        --parent->rank;
                        }else{
                                sibling = sibling->left; 
                                r2_wavlnode_right_rotation(tree, sibling);
                                r2_wavlnode_left_rotation(tree, sibling);
                                
                                sibling->rank += 2; 
                                parent->rank -= 2;
                                --sibling->right->rank;
                        }
                }else{
                        if(r2_wavlnode_rank_diff(sibling, sibling->left) == 1)
                        {
                                r2_wavlnode_right_rotation(tree, sibling);
                                ++sibling->rank;
                                --parent->rank;

                                 /**
                                 * If root isn't a legit node then it's the dummy node 
                                 * which has a rank of -1. If the sibling of the dummy node is 
                                 * null then parent is really a leaf. Demote parent again.
                                 * 
                                 */
                                sibling = r2_wavlnode_get_sibling(root);
                                if(sibling == NULL && root->rank == -1)
                                        --parent->rank;
                        }else{
                                sibling = sibling->right; 
                                r2_wavlnode_left_rotation(tree, sibling);
                                r2_wavlnode_right_rotation(tree, sibling);
                                sibling->rank += 2; 
                                parent->rank -= 2;
                                --sibling->left->rank;
                        }
                }
                
                root_ctype = r2_wavlnode_rank_diff(parent, root); 
        }  


}


/**
 * @brief                       Finds the root at index.
 *                              Indexing starts at zero.
 * @param tree                  WAVL Tree.
 * @param pos                   Index of node.
 * @return struct r2_avlnode*   Returns the root node at index, else NULL.
 */
struct r2_wavlnode* r2_wavltree_at(struct r2_wavlnode *root, r2_uint64 pos)
{
        
        if(pos >= root->ncount)
                return NULL; 
                
        ++pos;
        r2_int64 size = 0; 
        while(root != NULL){
                size = r2_wavlnode_recalc_size(root->left) + 1;
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
 * @brief               Gets keys in tree.
 *                      Gets the keys in sorted order.
 * @param tree          WAVL Tree.
 * @return void**       Returns an array of keys, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the keys. N.B. It's the responsibilty of the caller to free keys array
 *                      by calling free function with the keys array. The size of the keys array will be the 
 *                      size of the tree.
 */
void** r2_wavltree_get_keys(const struct r2_wavltree *tree)
{
        if(tree->ncount <= 0)
                return NULL;

        void **keys = malloc(sizeof(void *) * tree->ncount); 
        if(keys != NULL){
                struct r2_wavlnode *root = r2_wavlnode_inorder_first(tree->root); 
                r2_int64 size = 0; 
                while(root != NULL){
                        keys[size] = root->key; 
                        ++size;
                        root = r2_wavlnode_inorder_next(root);
                }
        }

        return keys;
}

/**
 * @brief               Gets values in tree.
 *                      Gets values in sorted order.
 * @param tree          WAVL Tree.
 * @return void**       Returns an array of values, else NULL when tree is empty or we're unable to allocate
 *                      an array to hold the values. N.B. It's the responsibilty of the caller to free values array
 *                      by calling free function with the values array. The size of the values array will be the 
 *                      size of the tree.
 */
void** r2_wavltree_get_values(const struct r2_wavltree *tree)
{
        if(tree->ncount <= 0)
                return NULL;

        void **values= malloc(sizeof(void *) * tree->ncount); 
        if(values != NULL){
                struct r2_wavlnode *root = r2_wavlnode_inorder_first(tree->root); 
                r2_int64 size = 0; 
                while(root != NULL){
                        values[size] = root->data; 
                        ++size;
                        root = r2_wavlnode_inorder_next(root);
                }
        }

        return values;
}

/**
 * @brief                        Returns the first node in an order traversal which is the miniumum node in the subtree.
 * 
 * @param root                   Root.
 * @return struct r2_wavlnode*   Returns the first node.
 */
struct r2_wavlnode* r2_wavlnode_inorder_first(struct r2_wavlnode *root)
{
        return r2_wavlnode_min(root);
}

/**
 * @brief                       Finds the next node in an inorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the next node in an inorder traversal, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_inorder_next(struct r2_wavlnode *root)
{
        return r2_wavlnode_successor(root);
}

/**
 * @brief               Performs an inorder traversal and an action for each node.
 *                      
 * @param tree          WAVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Argument passed to action.
 */
void r2_wavltree_inorder(struct r2_wavlnode *root, r2_act action, void *arg)
{
        struct r2_wavlnode *old_root = root; 
        struct r2_wavlnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_wavlnode_inorder_first(root); 
        while(root != NULL){
                action(root, arg);
                root = r2_wavlnode_inorder_next(root);
        }

        old_root->parent = parent;
}



/**
 * @brief                       Returns the first node in a preorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the first node.
 */
struct r2_wavlnode* r2_wavlnode_preorder_first(struct r2_wavlnode *root)
{
        return root; 
}
/**
 * @brief                       Finds the next node in an preorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the next node in an preorder traversal, else NULL.
 */
struct r2_wavlnode* r2_wavlnode_preorder_next(struct r2_wavlnode *root)
{
        if(root->left != NULL)
                return root->left; 
        
        if(root->right != NULL)
                return root->right;
                
        struct r2_wavlnode *parent = root->parent; 
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
 * @param tree          WAVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Argument passed to action.
 */
void r2_wavltree_preorder(struct r2_wavlnode *root, r2_act action, void *arg)
{
        struct r2_wavlnode *old_root = root; 
        struct r2_wavlnode *parent   = root->parent; 
        root->parent = NULL;

        while(root != NULL){
                action(root, arg); 
                root = r2_wavlnode_preorder_next(root);
        }

        old_root->parent = parent;
}

/**
 * @brief                       Returns the first node in a postorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the first node.
 */
struct r2_wavlnode* r2_wavlnode_postorder_first(struct r2_wavlnode *root)
{
        while(root != NULL && root->left != NULL)
                root = root->left; 
        
        while(root != NULL && root->right != NULL)
                root = root->right; 

        return root; 

}

/**
 * @brief                       Returns the next node in a postorder traversal.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the first node.
 */
struct r2_wavlnode* r2_wavlnode_postorder_next(struct r2_wavlnode *root)
{
        struct r2_wavlnode *parent = root->parent; 
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
 * @param tree          WAVL Tree.
 * @param action        Action to be performed on node.
 * @param arg           Argument passed to action.
 */
void r2_wavltree_postorder(struct r2_wavlnode *root, r2_act action, void *arg)
{
        struct r2_wavlnode *old_root = root; 
        struct r2_wavlnode *parent   = root->parent; 
        root->parent = NULL;

        root = r2_wavlnode_postorder_first(root);
        while(root != NULL){
                action(root, arg);
                root = r2_wavlnode_postorder_next(root); 
        }

        old_root->parent = parent;
}


/**
 * @brief               Compares two WAVL TREES. 
 *                      Compares trees using preorder traversal. If trees have different structure then it will fail.
 * 
 * @param tree1         Tree 1
 * @param tree2         Tree 2
 * @return r2_uint16    Returns TRUE if equal, else FALSE.
 */
r2_uint16 r2_wavltree_compare(const struct r2_wavltree *tree1, const struct r2_wavltree *tree2)
{

        
        r2_uint16 result = FALSE;
        if(r2_wavltree_empty(tree1) == TRUE && r2_wavltree_empty(tree2) == TRUE)
                result = TRUE;
        else if(tree1->ncount == tree2->ncount){
                /*Compare trees using preorder traversal.*/
                struct r2_wavlnode *root1 = tree1->root; 
                struct r2_wavlnode *root2 = tree2->root;
                while(root1 != NULL && root2 != NULL){
                       
                        result = tree1->kcmp!= NULL? (tree1->kcmp(root1->key, root2->key) == 0) :  root1->key == root2->key;
                        result &= tree1->dcmp != NULL? (tree1->dcmp(root1->data, root2->data) == 0) :  root1->data == root2->data;
                        if(result == FALSE)
                                break;
        

                        root1 =  r2_wavlnode_preorder_next(root1); 
                        root2 =  r2_wavlnode_preorder_next(root2);
                } 
        } 
        return result;

}

/**
 * @brief                       Creates a copy of tree.
 *                              
 * @return struct r2_avltree*   Returns a copy of a WAVL tree.
 */
struct r2_wavltree* r2_wavltree_copy(const struct r2_wavltree *source)
{
        if(source->kcmp == NULL)
                return NULL;

        struct r2_wavltree *dest = r2_create_wavltree(source->kcmp, source->dcmp, source->kcpy, source->dcpy, source->fk, source->fd);
        if(dest != NULL){
                void *key  = NULL; 
                void *data = NULL;
                struct r2_wavlnode *root = r2_wavlnode_inorder_first(source->root);
                while(root != NULL){
                        key = root->key; 
                        data = root->data;
                        if(source->kcpy != NULL && source->dcpy != NULL){
                                key  = source->kcpy(key); 
                                if(data != NULL){
                                        data = source->dcpy(data);
                                        if(data == NULL){
                                                r2_destroy_wavltree(dest);
                                                break;
                                        }
                                }
                                
                        }
                        r2_wavltree_insert(dest, key, data);
                        root  = r2_wavlnode_inorder_next(root);
                } 
        }
        
        return dest;

}


/**
 * @brief               Finds all the nodes between lower and upper inclusively.
 * 
 *                      Returns a list. N.B Please free list when finished by calling r2_destroy_list(list, freekey);
 * @param tree          WAVL Tree.
 * @param lower         Lower bound.
 * @param upper         Upper bound.
 * @param action        Action to perform on node.
 * @param arg           Argument passed to action.
 * @return void **      Return a list with the keys between lower and upper inclusively. 
 */
struct r2_list* r2_wavltree_range_query(const struct r2_wavltree *tree, void *lower, void *upper, r2_act action, void *arg)
{
      /*Empty tree doesn't have range.*/
        if(r2_wavltree_empty(tree) == TRUE)
                return NULL; 

        /*List to store keys in the range.*/
        struct r2_list *keys = r2_create_list(tree->kcmp, tree->kcpy, tree->fk);
        if(keys!= NULL){
                struct r2_wavlnode *k1 = NULL; 
                struct r2_wavlnode *root = tree->root; 
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
                                keys = r2_destroy_list(keys);
                                break;
                        }
                        if(r2_list_insert_at_back(keys, key) == FALSE){
                                keys = r2_destroy_list(keys);
                                break;
                        }
                        k1 =  r2_wavlnode_successor(k1);
                }

        }      
       return keys;   
}

/**
 * @brief               Peforms a left rotation on root.
 * 
 * @param tree          WAVL Tree.
 * @param root          Root.
 */
static void r2_wavlnode_left_rotation(struct r2_wavltree *tree, struct r2_wavlnode *root)
{
        struct r2_wavlnode *parent      = root->parent;
        struct r2_wavlnode *grandparent = parent->parent; 

        parent->right = root->left; 
        if(parent->right != NULL)
                parent->right->parent = parent;

        
      
        parent->ncount        = r2_wavlnode_recalc_size(parent);
        root->left            = parent; 
        root->left->parent    = root; 
        root->ncount            = r2_wavlnode_recalc_size(root); 

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root; 
        
        
                grandparent->ncount          = r2_wavlnode_recalc_size(grandparent);
        }
                
        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root   = root; 
                tree->ncount = root->ncount;
        }
}


/**
 * @brief               Peforms a right rotation on root.
 * 
 * @param tree          WAVL Tree.
 * @param root          Root.
 */
static void r2_wavlnode_right_rotation(struct r2_wavltree *tree, struct r2_wavlnode *root)
{
        struct r2_wavlnode *parent      = root->parent;
        struct r2_wavlnode *grandparent = parent->parent; 

        parent->left = root->right; 
        if(parent->left != NULL)
                parent->left->parent = parent;

        parent->ncount          = r2_wavlnode_recalc_size(parent);
        root->right           = parent; 
        root->right->parent   = root; 
        root->ncount            = r2_wavlnode_recalc_size(root); 

        if(grandparent != NULL){
                if(grandparent->right == parent)
                        grandparent->right = root; 
                else
                        grandparent->left  = root; 

                grandparent->ncount          = r2_wavlnode_recalc_size(grandparent);
        }
                
        root->parent = grandparent;
        if(root->parent == NULL){
                tree->root = root; 
                tree->ncount = root->ncount;
        }
}

/**
 * @brief              Helper function to calculate the size of a tree after balancing procedures.      
 * 
 * @param root         Root.
 * @return r2_int64    Returns the size of the tree.
 */
static r2_int64 r2_wavlnode_recalc_size(const struct r2_wavlnode *root)
{
        size_t size = 0;
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
 * @brief                       Calculates the rank difference of root.
 *                              Rank difference is rank(parent(root)) - rank(root)
 * 
 * @param root                  Root.
 * @return r2_int64             Returns the rank difference, else -1 when root is NULL.
 */
static r2_int64 r2_wavlnode_rank_diff(const struct r2_wavlnode *parent, const struct r2_wavlnode *root)
{
        r2_int64 root_rank = -1;
        if(root != NULL)
               root_rank = root->rank;

        return parent != NULL? parent->rank - root_rank : 0;
}


/**
 * @brief                    Determines the if root has a specific child.
 * 
 * @param root               Root.
 * @param child_type         Child type.
 * @return r2_int16          Returns TRUE if either of roots' children equal that child type.
 */
static r2_int16 r2_wavlnode_has_child(const struct r2_wavlnode *root, enum CHILD_TYPE child_type)
{
        r2_int16 result = FALSE; 
        if(root != NULL){
                r2_int64 left_rdiff  = r2_wavlnode_rank_diff(root, root->left); 
                r2_int64 right_rdiff = r2_wavlnode_rank_diff(root, root->right);

                if(left_rdiff == child_type || right_rdiff == child_type)
                        result = TRUE; 
        }
        return result; 
}

/**
 * @brief               Checks whether root is a leaf.
 * 
 * @param root          Root.
 * @return r2_int64     Returns TRUE if root is a leaf, otherwise FALSE.
 */
static r2_int64 r2_wavlnode_is_leaf(const struct r2_wavlnode *root)
{
        return root->left == NULL && root->right == NULL;
}


/**
 * @brief                       Get's the sibling of root.
 * 
 * @param root                  Root.
 * @return struct r2_wavlnode*  Returns the sibling of root.
 */
static struct r2_wavlnode* r2_wavlnode_get_sibling(const struct r2_wavlnode *root)
{
        const struct r2_wavlnode *parent = root->parent; 
        return parent->right == root? parent->left : parent->right;
}



/**
 * @brief               Transplants a root.
 * 
 * @param tree          WAVL Tree.
 * @param root          Root.
 */
static void r2_wavltree_restructure(struct r2_wavltree *tree, struct r2_wavlnode *root, struct r2_wavlnode *child)
{
        struct r2_wavlnode *parent = root->parent;
        if(parent != NULL){
                if(parent->right == root)
                        parent->right = child; 
                else
                        parent->left  = child;
                
                parent->ncount   = r2_wavlnode_recalc_size(parent); 
        }else {
                tree->root = child;
                tree->ncount = r2_wavlnode_recalc_size(child);
        }

        if(child != NULL)
                child->parent = parent;
}



/**
 * @brief Helper function to free a node. 
 *        
 *        If r2_freedata is NULL it doesn't free the data portion of node. 
 */
static void r2_freenode(struct r2_wavlnode *root, r2_fd freedata , r2_fk freekey)
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