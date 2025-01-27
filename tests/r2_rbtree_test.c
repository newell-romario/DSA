#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "r2_rbtree_test.h"


#define SIZE   9
static r2_uint64 arr[SIZE] = {1, 9, 2, 8, 3, 7, 4, 6, 5};
static r2_int16 cmp(const void *a, const void *b);
static r2_int64 r2_rbnode_size(const struct r2_rbnode *);


/**
 * @brief       Tests create node functionality.
 * 
 */
static void test_r2_create_rbnode()
{
        struct r2_rbnode *root = r2_create_rbnode();
        assert(root != NULL);
        assert(root->color  == RED); 
        assert(root->data   == NULL); 
        assert(root->key    == NULL); 
        assert(root->parent == NULL); 
        assert(root->left   == NULL); 
        assert(root->right  == NULL); 
        assert(root->ncount == 1);
        free(root);
}

/**
 * @brief       Tests create tree functionality.
 * 
 */
static void test_r2_create_rbtree()
{
        struct r2_rbtree *tree = r2_create_rbtree(NULL, NULL, NULL, NULL, NULL, NULL); 
        assert(tree != NULL);
        assert(tree->root == NULL); 
        assert(tree->ncount == 0);
        assert(tree->kcmp == NULL); 
        assert(tree->dcmp == NULL); 
        assert(tree->kcpy == NULL); 
        assert(tree->dcpy == NULL);
        assert(tree->fk   == NULL); 
        assert(tree->fd   == NULL);
        assert(r2_rbtree_empty(tree) == TRUE);
        r2_destroy_rbtree(tree);
}

/**
 * @brief       Tests the destroy tree functionality.s
 * 
 */
static void test_r2_destroy_rbtree()
{
        struct r2_rbtree *tree = r2_create_rbtree(NULL, NULL, NULL, NULL, NULL, NULL); 
        assert(r2_destroy_rbtree(tree) == NULL);

}


/**
 * @brief       Tests the insert functionality.
 * 
 */
static void test_r2_rbnode_insert()
{
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_rbnode *root = NULL; 
        for(int i = 0 ; i < SIZE; ++i){
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
                test_r2_rbtree_certify(tree->root, cmp);
        }

        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        assert(r2_rbtree_empty(tree) != TRUE);
        assert(tree->ncount == SIZE);

        /*Finds 1*/
        root = r2_rbtree_search(tree, &arr[0]);
        assert(root != NULL);
        assert(root->color == BLACK);
        assert(root->data  == &arr[0]);
        assert(root->left  == NULL);
        assert(root->right == NULL);
        assert(root->ncount  == 1);

        /*Finds 9*/
        root = r2_rbtree_search(tree, &arr[1]);
        assert(root != NULL);
        assert(root->color == BLACK);
        assert(root->data  == &arr[1]);
        assert(root->left  == NULL);
        assert(root->right == NULL);
        assert(root->ncount  == 1);
        
        /*Finds 2*/
        root = r2_rbtree_search(tree, &arr[2]);
        assert(root != NULL);
        assert(root->color             == RED);
        assert(root->data              == &arr[2]);
        assert(root->left->data        == &arr[0]);
        assert(root->right->data       == &arr[4]);
        assert(root->ncount  == 3);

        /*Finds 8*/
        root = r2_rbtree_search(tree, &arr[3]);
        assert(root != NULL);
        assert(root->color             == RED);
        assert(root->data              == &arr[3]);
        assert(root->left->data        == &arr[7]);
        assert(root->right->data       == &arr[1]);
        assert(root->ncount  == 5);

        /*Finds 3*/
        root = r2_rbtree_search(tree, &arr[4]);
        assert(root != NULL);
        assert(root->color == BLACK);
        assert(root->data  == &arr[4]);
        assert(root->left  == NULL);
        assert(root->right == NULL);
        assert(root->ncount  == 1);

        /*Finds 7*/
        root = r2_rbtree_search(tree, &arr[5]);
        assert(root != NULL);
        assert(root->color == RED);
        assert(root->data  == &arr[5]);
        assert(root->left  == NULL);
        assert(root->right == NULL);
        assert(root->ncount  == 1);

        /*Finds 4*/
        root = r2_rbtree_search(tree, &arr[6]);
        assert(root != NULL);
        assert(root->color == BLACK);
        assert(root->data  == &arr[6]);
        assert(root->left->data  == &arr[2]);
        assert(root->right->data == &arr[3]);
        assert(root->ncount  == 9);

        /*Finds 6*/
        root = r2_rbtree_search(tree, &arr[7]);
        assert(root != NULL);
        assert(root->color == BLACK);
        assert(root->data  == &arr[7]);
        assert(root->left->data  == &arr[8]);
        assert(root->right->data == &arr[5]);
        assert(root->ncount  == 3);

        /*Finds 5*/
        root = r2_rbtree_search(tree, &arr[8]);
        assert(root != NULL);
        assert(root->color == RED);
        assert(root->data  == &arr[8]);
        assert(root->left  == NULL);
        assert(root->right == NULL);
        assert(root->ncount  == 1);

    
        /*free tree*/
        r2_destroy_rbtree(tree);
}

/**
 * @brief Tests the delete functionality.
 * 
 */
static void test_r2_rbnode_delete()
{
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_rbnode *root = NULL; 
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        /*Testing case 2 of the delete algorithm.*/
        /*Deleting node with key 1.*/
        root = r2_rbtree_search(tree, &arr[0]);
        assert( root != NULL);
        tree = r2_rbtree_delete(tree, &arr[0]);
        root = r2_rbtree_search(tree, &arr[0]);
        assert(root == NULL);
        test_r2_rbtree_certify(tree->root, cmp);

       /*Testing case 3-4 of the delete algorithm.*/
       /*Deleting node with key 9.*/
        root = r2_rbtree_search(tree, &arr[1]);
        assert( root != NULL);
        tree = r2_rbtree_delete(tree, &arr[1]);
        root = r2_rbtree_search(tree, &arr[1]);
        assert(root == NULL);
        test_r2_rbtree_certify(tree->root, cmp);
        
        /*Testing the delete of a black node with one child.*/
        /*Deleting node with key 2.*/
        root = r2_rbtree_search(tree, &arr[2]);
        assert( root != NULL);
        tree = r2_rbtree_delete(tree, &arr[2]);
        root = r2_rbtree_search(tree, &arr[2]);
        assert( root == NULL);
        test_r2_rbtree_certify(tree->root, cmp);
        
        /*Testing the delete of a black leaf node. A black leaf node is 3.*/
        /*Testing case 1 of the delete algorithm.*/
        root = r2_rbtree_search(tree, &arr[4]);
        assert( root != NULL);
        tree = r2_rbtree_delete(tree, &arr[4]);
        root = r2_rbtree_search(tree, &arr[4]);
        assert( root == NULL);
        test_r2_rbtree_certify(tree->root, cmp);
        
        /*Testing the delete of a black internal node.*/
        /*Deleting node with key 7.*/
        root = r2_rbtree_search(tree, &arr[5]);
        assert( root != NULL);
        tree = r2_rbtree_delete(tree, &arr[5]);
        root = r2_rbtree_search(tree, &arr[5]);
        assert( root == NULL);
        test_r2_rbtree_certify(tree->root, cmp);
        
        for(int i = 0; i < SIZE; ++i){
                test_r2_rbtree_certify(tree->root, cmp);
                tree = r2_rbtree_delete(tree, &arr[i]);
                test_r2_rbtree_certify(tree->root, cmp);
        }
                

        assert(r2_rbtree_empty(tree) == TRUE);
        
        /*free tree*/
        r2_destroy_rbtree(tree);
}

/**
 * @brief        Verifies is a tree is a red and black tree.
 * 
 * @param root 
 */
static r2_uint64 test_r2_rbnode_blackheight(const struct r2_rbnode *root)
{
        if(root == NULL)
                return 0;
        
        
        r2_int64 leftblackheight  = test_r2_rbnode_blackheight(root->left);
        r2_int64 rightblackheight = test_r2_rbnode_blackheight(root->right);
       
        assert(leftblackheight == rightblackheight);
        if(root->color == BLACK)
                ++leftblackheight; 
        
        return leftblackheight;
}

/**
 * @brief       Tests that no consecutive red node exist on the path to any
 *              nil node.
 * 
 * @param root 
 */
static void test_r2_rbnode_noconsecreds(const struct r2_rbnode *root)
{
        if(root == NULL)
                return; 
        
        if(root->color == RED)
        {
                enum COLOR leftcolor = root->left != NULL? root->left->color : BLACK; 
                enum COLOR rightcolor = root->right != NULL? root->right->color : BLACK;
                assert(leftcolor == BLACK); 
                assert(rightcolor == BLACK); 
        }

        test_r2_rbnode_noconsecreds(root->left); 
        test_r2_rbnode_noconsecreds(root->right); 
}

static void test_r2_rbtree_is_binary_tree(const struct r2_rbnode *root, r2_cmp cmpfunc)
{
        if(root == NULL)
                return; 
        test_r2_rbtree_is_binary_tree(root->left, cmpfunc); 
        test_r2_rbtree_is_binary_tree(root->right, cmpfunc); 
        if(root->left != NULL)
                assert(cmpfunc(root->left->key, root->key) < 0);
        
        if(root->right != NULL)
                assert(cmpfunc(root->right->key, root->key) > 0);
}

static void test_r2_rbtree_certify(const struct r2_rbnode *root, r2_cmp cmpfunc)
{
        if(root == NULL)
                return;
        test_r2_rbtree_is_binary_tree(root, cmpfunc);
        test_r2_rbnode_noconsecreds(root);
        assert(test_r2_rbnode_blackheight(root->left) == test_r2_rbnode_blackheight(root->right));
}

static r2_int16 cmp(const void *a, const void *b){
        const long long int *c = a; 
        const long long int *d = b; 

       if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else 
                return 1;
}



/**
 * @brief Tests the minimum functionality.
 * 
 */
static void test_r2_rbnode_min()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL); 
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = r2_rbnode_min(tree->root); 
        assert(root->data == &arr[0]);

        /*free tree*/
        r2_destroy_rbtree(tree);
}

/**
 * @brief Tests the maximum functionality.
 * 
 */
static void test_r2_rbnode_max()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = r2_rbnode_max(tree->root); 
        assert(root->data == &arr[1]);

        /*free tree*/
        r2_destroy_rbtree(tree);
}

/**
 * @brief Tests the successor functionality.
 * 
 */
static void test_r2_rbnode_successor()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = r2_rbnode_successor(tree->root); 
        assert(root->data == &arr[8]);
        
        root = r2_rbtree_search(tree, &arr[5]);
        root = r2_rbnode_successor(root); 
        assert(root->data == &arr[3]);

        root = r2_rbtree_search(tree, &arr[1]);
        root = r2_rbnode_successor(root); 
        assert(root == NULL);
        /*free tree*/
        r2_destroy_rbtree(tree);  
}



/**
 * @brief Tests the predecessor functionality.
 * 
 */
static void test_r2_rbnode_predecessor()
{
         /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = r2_rbnode_predeccessor(tree->root); 
        assert(root->data == &arr[4]);
        
        root = r2_rbtree_search(tree, &arr[8]);
        root = r2_rbnode_predeccessor(root); 
        assert(root->data == &arr[6]);

        root = r2_rbtree_search(tree, &arr[0]);
        root = r2_rbnode_predeccessor(root); 
        assert(root == NULL);
        /*free tree*/
        r2_destroy_rbtree(tree);

}

/**
 * @brief       Testing the search functionality.
 * 
 */
static void test_r2_rbnode_search()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = NULL;

        root = r2_rbtree_search(tree, &arr[0]);
        assert(root->data == &arr[0]);


        int key = 0;
        root = r2_rbtree_search(tree, &key);
        assert(root == NULL);


        /*free tree*/
        r2_destroy_rbtree(tree);

}

/*Test the size is updated correctly for every node.*/
static void test_r2_rbnode_size()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i){
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
                assert(tree->ncount== r2_rbnode_size(tree->root));
        }
                
         /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        for(int i = 0 ; i < SIZE; ++i){
                tree = r2_rbtree_delete(tree, &arr[0]);
                assert(tree->ncount == r2_rbnode_size(tree->root));
        }
        


        /*free tree*/
        r2_destroy_rbtree(tree);

}

/**
 * @brief               Calculates the size of the tree recursively.
 * 
 * @param root          RB Tree
 * @return r2_int64     Returns size
 */
static r2_int64 r2_rbnode_size(const struct r2_rbnode *root)
{
        if(root == NULL)
                return  0; 
        
        r2_int64 leftsize  = r2_rbnode_size(root->left); 
        r2_int64 rightsize = r2_rbnode_size(root->right); 
        return leftsize + rightsize + 1; 
}

/**
 * @brief  Tests the index functionality.
 * 
 */
static void test_r2_rbtree_at()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        
        struct r2_rbnode *root = NULL;
        root = r2_rbtree_at(tree->root, 0);
        assert(root->data == &arr[0]); 

        root = r2_rbtree_at(tree->root, 8);
        assert(root->data == &arr[1]); 
        
        root = r2_rbtree_at(tree->root, 9);
        assert(root == NULL); 
                

        /*free tree*/
        r2_destroy_rbtree(tree);
}

static void print_node(void *node, void *arg)
{
        struct r2_rbnode *root  = (struct r2_rbnode *)node; 
        printf(" %d", *((r2_int64 *)root->key));
}

static void test_r2_rbtree_inorder()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);

        printf("\n/*******************************************RB Inorder Traversal************************/\n");
        r2_rbtree_inorder(tree->root, print_node, NULL);

        struct r2_rbnode *root = r2_rbnode_inorder_first(tree->root);
        assert(root->data == &arr[0]); 

        root = r2_rbnode_inorder_next(root);
        assert(root->data == &arr[2]); 

        root = r2_rbnode_max(tree->root);
        root = r2_rbnode_inorder_next(root);
        assert(root == NULL); 
        
        printf("\n/*******************************************RB Inorder Traversal Left************************/\n");
        r2_rbtree_inorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************RB Inorder Traversal Right************************/\n");
        r2_rbtree_inorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_rbtree(tree);

}
static void test_r2_rbtree_preorder()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);

        printf("\n/*******************************************RB Preorder Traversal************************/\n");
        r2_rbtree_preorder(tree->root, print_node, NULL);

        struct r2_rbnode *root = r2_rbnode_preorder_first(tree->root);
        assert(root->data == &arr[6]); 

        root = r2_rbnode_preorder_next(root);
        assert(root->data == &arr[2]); 

        root = r2_rbnode_max(tree->root);
        root = r2_rbnode_preorder_next(root);
        assert(root == NULL); 
        
        printf("\n/*******************************************RB Preorder Traversal Left************************/\n");
        r2_rbtree_preorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************RB Preorder Traversal Right************************/\n");
        r2_rbtree_preorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_rbtree(tree);

}
static void test_r2_rbtree_postorder()
{
         /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);
        

        printf("\n/*******************************************RB Postorder Traversal************************/\n");
        r2_rbtree_postorder(tree->root, print_node, NULL);

        struct r2_rbnode *root = r2_rbnode_postorder_first(tree->root);
        assert(root->data == &arr[0]); 

        root = r2_rbnode_postorder_next(root);
        assert(root->data == &arr[4]); 

        root = r2_rbnode_max(tree->root);
        root = r2_rbnode_postorder_next(root);
        assert(root->data == &arr[3]); 
        
        printf("\n/*******************************************RB Postorder Traversa Left************************/\n");
        r2_rbtree_postorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************RB Postorder Traversal Right************************/\n");
        r2_rbtree_postorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_rbtree(tree);
}

static void *cpy(const void *data)
{
        long long int *copy = malloc(sizeof(long long int));
        *copy = *((long long int *)data);
        return copy;  
}

/**
 * @brief      Tests the copy functionality.
 * 
 */
static void test_r2_rbtree_copy()
{
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_rbtree *copy = r2_rbtree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, copy) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_rbtree_compare(tree, copy) == TRUE); 
        r2_destroy_rbtree(copy);

        /*Deep copy comparison*/
        tree->kcpy = cpy; 
        tree->dcpy = cpy;
        copy = r2_rbtree_copy(tree); 
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, copy) == TRUE);

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_rbtree_compare(tree, copy) == TRUE); 
        r2_destroy_rbtree(copy);
        

        for(int i = 0; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);  

        /*Shallow comparison*/
        tree->kcpy = NULL; 
        tree->dcpy = NULL;
        copy = r2_rbtree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, copy) == TRUE);

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_rbtree_compare(tree, copy) == TRUE); 
        r2_destroy_rbtree(copy);

        /*Deep copy*/
        tree->kcpy = cpy; 
        tree->dcpy = cpy;
        copy = r2_rbtree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, copy) != TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_rbtree_compare(tree, copy) == TRUE); 

        r2_destroy_rbtree(tree);
        r2_destroy_rbtree(copy);
}

/*Test comparison*/
static void test_r2_rbtree_compare()
{
        struct r2_rbtree *tree = r2_create_rbtree(NULL, NULL, NULL, NULL, NULL, NULL);

        assert(r2_rbtree_compare(tree, tree) == TRUE); /*Shallow comparison*/

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_rbtree_compare(tree, tree) == TRUE); 

        for(int i = 0; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);  

        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, tree) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;        
        assert(r2_rbtree_compare(tree, tree) == TRUE); 

        /*Deep copy*/
        tree->kcpy = cpy; 
        tree->dcpy = cpy;
        struct r2_rbtree *copy = r2_rbtree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_rbtree_compare(tree, copy) != TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;  
        assert(r2_rbtree_compare(tree, copy) == TRUE); 
        r2_destroy_rbtree(tree);
        r2_destroy_rbtree(copy);
}

/**
 * @brief      Test the get keys functionality.
 * 
 */
static void test_r2_rbtree_getkeys()
{
        /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);

        void **keys = r2_rbtree_get_values(tree); 
        assert((*(int *)keys[0]) == 1);
        assert((*(int *)keys[1]) == 2);
        assert((*(int *)keys[2]) == 3);
        assert((*(int *)keys[3]) == 4);
        assert((*(int *)keys[4]) == 5);
        assert((*(int *)keys[5]) == 6);
        assert((*(int *)keys[6]) == 7);
        assert((*(int *)keys[7]) == 8);
        assert((*(int *)keys[8]) == 9);
        free(keys);
        r2_destroy_rbtree(tree);
}

/**
 * @brief       Tests the get values functionality
 * 
 */
static void test_r2_rbtree_getvalues()
{
         /*{1, 9, 2, 8, 3, 7, 4, 6, 5}*/
        struct r2_rbtree *tree = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        for(int i = 0 ; i < SIZE; ++i)
                tree = r2_rbtree_insert(tree, &arr[i], &arr[i]);

        void **values = r2_rbtree_get_values(tree); 
        assert((*(int *)values[0]) == 1);
        assert((*(int *)values[1]) == 2);
        assert((*(int *)values[2]) == 3);
        assert((*(int *)values[3]) == 4);
        assert((*(int *)values[4]) == 5);
        assert((*(int *)values[5]) == 6);
        assert((*(int *)values[6]) == 7);
        assert((*(int *)values[7]) == 8);
        assert((*(int *)values[8]) == 9);
        free(values);
        r2_destroy_rbtree(tree);
}

/**
 * @brief   Generates use sample data to test insertion and deletion.
 * 
 */
static void test_r2_rbtree_generate()
{
        FILE *dataset = fopen("../tests/sample.csv","r");
        struct r2_rbtree  *rb = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        long long int *data = malloc(sizeof(r2_uint64) *100000); 

        for(int i = 0; i < 100000;++i){  
             fscanf(dataset, "%lld", &data[i]);
             rb = r2_rbtree_insert(rb, &data[i], &data[i]); 
        }

        test_r2_rbtree_certify(rb->root, cmp);
 
        for(int i = 0; i < 100000;++i){
                rb = r2_rbtree_delete(rb, &data[i]); 
                test_r2_rbtree_certify(rb->root, cmp); 
        }
            
        
        free(data);
        fclose(dataset);
        r2_destroy_rbtree(rb);     
}

/**
 * @brief Produces benchmarks for this RB tree.
 * 
 */
static void test_r2_rbtree_stats()
{
        struct r2_rbtree *rb = r2_create_rbtree(cmp, NULL, NULL, NULL, NULL, NULL);
        FILE *fp = fopen("com-friendster.ungraph.txt", "r"); 
        char line[100];
        r2_uint64 a[2] = {0}; 
        r2_uint64 *key = NULL; 
        clock_t before; 
        r2_ldbl elapse = 0;


        /*File 30GB and we can't edit the file because of size. Skip first 15 lines because it's irrelevant*/
        for(r2_uint16 i = 0; i < 15; ++i){
                        fscanf(fp, "%s", line);
        }

        while(fscanf(fp, "%lld\t%lld", &a[0], &a[1]) == 2){
                        for(r2_uint16 i = 0; i < 2; ++i){
                                key = malloc(sizeof(r2_uint64));  
                                *key = a[i];
                                before = clock();
                                rb = r2_rbtree_insert(rb, key, key);
                                elapse += (clock() - before)/(r2_ldbl)CLOCKS_PER_SEC;
                        }

                        if(rb->ncount == 20000000)
                                break;
        }

        rewind(fp);
        /*File 30GB and we can't edit the file because of size. Skip first 15 lines because it's irrelevant*/
        for(r2_uint16 i = 0; i < 15; ++i)
                fscanf(fp, "%s", line);
        
        elapse = 0;
        r2_uint64 count = 0;
        while(fscanf(fp, "%lld\t%lld", &a[0], &a[1]) == 2){
                for(r2_uint16 i = 0; i < 2; ++i){
                        before = clock();
                        if(r2_rbtree_search(rb, &a[i]) == NULL)
                                break;

                        elapse += (clock() - before)/(r2_ldbl)CLOCKS_PER_SEC;
                }

                if(count == 20000000)
                        break;
                ++count;
        }
        r2_uint64 height = r2_rbtree_height(rb->root);
        printf("\nAverage insertion time: %Lf", elapse/rb->ncount);
        printf("\nHeight: %ld", height);

        test_r2_rbnode_noconsecreds(rb->root);
        test_r2_rbnode_blackheight(rb->root);
        test_r2_rbtree_is_binary_tree(rb->root, cmp);
        r2_destroy_rbtree(rb);
        fclose(fp);
}

/**
 * @brief Run all tests.
 * 
 */
void test_r2_rbtree_run()
{
        test_r2_create_rbnode();
        test_r2_create_rbtree();
        test_r2_destroy_rbtree();
        test_r2_rbnode_insert();
        test_r2_rbnode_delete();
        test_r2_rbnode_min();
        test_r2_rbnode_max();
        test_r2_rbnode_successor();
        test_r2_rbnode_predecessor();
        test_r2_rbnode_search();
        test_r2_rbnode_size();
        test_r2_rbtree_at();
        test_r2_rbtree_inorder();
        test_r2_rbtree_preorder(); 
        test_r2_rbtree_postorder();
        test_r2_rbtree_copy();
        test_r2_rbtree_compare();
        test_r2_rbtree_getkeys();
        test_r2_rbtree_getvalues();
        //test_r2_rbtree_generate();
        test_r2_rbtree_stats();
}