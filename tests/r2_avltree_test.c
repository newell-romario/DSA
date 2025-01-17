#include "r2_avltree_test.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define SIZE 10

r2_uint64 arr[SIZE];
static r2_int16 cmp(const void *, const void *);
static void* cpy(const void *);

/**
 * @brief Initializes test data
 */
static void test_r2_avltree_init_data()
{
        for(int i = 0; i < SIZE; ++i)
                arr[i] = i + 1;
}

/**
 * @brief  Tests the creation functionality of an avl node. 
 * 
 */
static void test_r2_create_avlnode()
{
        struct r2_avlnode *node = r2_create_avlnode();
        assert(node != NULL); 
        assert(node->data    == NULL); 
        assert(node->key     == NULL);
        assert(node->parent  == NULL); 
        assert(node->left    == NULL);
        assert(node->right   == NULL);
        assert(node->ncount  == 1); 
        assert(node->height  == 0);
        free(node);
}

/**
 * @brief  Tests the successor function for avl tree.
 * 
 */
static void test_r2_avlnode_successor()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);

        struct r2_avlnode *root = r2_avlnode_min(tree->root);
        
        /*Find the successor of the smallest node*/
        root = r2_avlnode_successor(root);
        assert(*(r2_uint64 *)root->key == arr[1]);

        /*Find the successor of the largest node*/
        root = r2_avlnode_max(tree->root);
        root = r2_avlnode_successor(root);
        assert(root == NULL);

        /*free tree*/
        r2_destroy_avltree(tree);
}


/**
 * @brief  Tests the predecessor function of the avl tree.
 * 
 */
static void test_r2_avlnode_predecessor()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);


        struct r2_avlnode *root = r2_avlnode_max(tree->root);
        
        /*Find the predecessor of the largest node*/
        root = r2_avlnode_predecessor(root);
        assert(*(r2_uint64 *)root->key == arr[8]);

        /*Find the predecessor of the smallest node*/
        root = r2_avlnode_min(tree->root);
        root = r2_avlnode_predecessor(root);
        assert(root == NULL);

        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Tests the min functionality of the avl tree.
 * 
 */
static void test_r2_avlnode_min()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]); 

        struct r2_avlnode *root = r2_avlnode_min(tree->root);
        assert(root != NULL); 
        assert(*(r2_uint64 *)root->key == arr[0]);

        /*free tree*/
        r2_destroy_avltree(tree);
         
}


/**
 * @brief       Tests the max functionality of the avl tree.
 * 
 */
static void test_r2_avlnode_max()
{

        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]); 

        struct r2_avlnode *root = r2_avlnode_max(tree->root);
        assert(root != NULL); 
        assert(*(r2_uint64*)root->key == arr[9]);

        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief  Tests the creation of an avl tree.
 * 
 */
static void test_r2_create_avltree()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        assert(tree != NULL);
        assert(tree->kcmp == cmp);
        assert(tree->dcmp == cmp); 
        assert(tree->kcpy == cpy);
        assert(tree->dcpy == cpy);
        assert(tree->fk   == NULL);
        assert(tree->fd   == NULL);
        assert(tree->root == NULL); 
        assert(tree->ncount == 0);
        
        /*free tree*/
        r2_destroy_avltree(tree);
}

/*Test delete functionality*/
static void test_r2_destroy_avltree()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        
        /*free tree*/
        assert(r2_destroy_avltree(tree) == NULL);
}

static r2_int16 cmp(const void *a, const void *b)
{
        r2_uint64 *c = (r2_uint64 *)a; 
        r2_uint64 *d = (r2_uint64 *)b; 

       if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else 
                return 1;
}

/**
 * @brief       Tests the insert functionality of the avl tree.
 * 
 */
static void test_r2_avltree_insert()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i){
               test_r2_avltree_certify(tree->root, tree->kcmp); 
               tree = r2_avltree_insert(tree, &arr[i], &arr[i]);
               test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        }
                
        assert(r2_avltree_empty(tree) != TRUE); 
        assert(*((r2_uint64 *)tree->root->key) == 4); 
        assert(tree->ncount == 10);
        assert(tree->root->ncount == 10);
        assert(tree->root->height == 3);
        assert(tree->root->parent == NULL);
        
        /*Testing if all nodes were inserted and if the tree is built correctly.*/
        struct r2_avlnode *root = NULL; 
        
        /*Searching for node containing key 1 */
        root = r2_avltree_search(tree, &arr[0]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->height == 0);
        assert(root->ncount == 1);

        /*Searching for node containing key 2 */
        root = r2_avltree_search(tree, &arr[1]); 
        assert(root != NULL);
        assert(root->left != NULL); 
        assert(root->right != NULL); 
        assert(*((r2_uint64 *)root->left->key) == 1); 
        assert(*((r2_uint64 *)root->right->key) == 3); 
        assert(root->height == 1);
        assert(root->ncount == 3);

        /*Searching for node containing key 3 */
        root = r2_avltree_search(tree, &arr[2]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->height == 0);
        assert(root->ncount == 1);
        
        /*Searching for node containing key 4 */
        root = r2_avltree_search(tree, &arr[3]); 
        assert(root != NULL);
        assert(root->left != NULL); 
        assert(root->right != NULL); 
        assert(*((r2_uint64 *)root->left->key) == 2); 
        assert(*((r2_uint64 *)root->right->key) == 8); 
        assert(root->height == 3);
        assert(root->ncount == 10);

        /*Searching for node containing key 5 */
        root = r2_avltree_search(tree, &arr[4]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->height == 0);
        assert(root->ncount == 1);

        /*Searching for node containing key 6 */
        root = r2_avltree_search(tree, &arr[5]); 
        assert(root != NULL);
        assert(root->left != NULL); 
        assert(root->right != NULL); 
        assert(*((r2_uint64 *)root->left->key) == 5); 
        assert(*((r2_uint64 *)root->right->key) == 7); 
        assert(root->height == 1);
        assert(root->ncount == 3);

        /*Searching for node containing key 7 */
        root = r2_avltree_search(tree, &arr[6]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->height == 0);
        assert(root->ncount == 1);

        /*Searching for node containing key 8 */
        root = r2_avltree_search(tree, &arr[7]); 
        assert(root != NULL);
        assert(root->left != NULL); 
        assert(root->right != NULL); 
        assert(*((r2_uint64 *)root->left->key) == 6); 
        assert(*((r2_uint64 *)root->right->key) == 9); 
        assert(root->height == 2);
        assert(root->ncount == 6);

        /*Searching for node containing key 9 */
        root = r2_avltree_search(tree, &arr[8]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right != NULL); 
        assert(*((r2_uint64 *)root->right->key) == 10); 
        assert(root->height == 1);
        assert(root->ncount == 2);


        /*Searching for node containing key 10 */
        root = r2_avltree_search(tree, &arr[9]); 
        assert(root != NULL);
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->height == 0);
        assert(root->ncount == 1);

     
        /*Free tree*/
        r2_destroy_avltree(tree);
}


/**
 * @brief       Tests search function of the avl tree.
 * 
 */
static void test_r2_avltree_search()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);

        struct r2_avlnode *root = NULL;
        for(int i = 0; i < SIZE; ++i){
                root = r2_avltree_search(tree, &arr[i]);
                assert(root != NULL);
                assert(*(r2_uint64 *)root->key == arr[i]);
        }

        r2_uint64 key = 100; 
        root = r2_avltree_search(tree, &key); 
        assert(root == NULL);
        
        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Tests avl tree delete functionality.
 * 
 */
static void test_r2_avltree_delete()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);
        

        struct r2_avlnode *root = NULL;
        r2_uint64 ncount  = tree->ncount;
        
        /*Testing to see if delete operations were successful and the correct avl tree was built. */
        /*Delete key with 1*/
        tree = r2_avltree_delete(tree, &arr[0]); 
        test_r2_avltree_certify(tree->root, tree->kcmp); 
        root = r2_avltree_search(tree, &arr[0]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[1]); 
        assert(root->left == NULL);
        assert(root->ncount == 2);
        assert(root->height == 1);
        assert(*((r2_uint64*)root->right->key) == arr[2]);

        /*Delete key with 2*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[1]); 
        test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[1]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[2]); 
        assert(root->left == NULL);
        assert(root->right == NULL);
        assert(root->ncount == 1);
        assert(root->height == 0);
        

        /*Delete key with 3*/
        ncount = tree->ncount;
        tree   = r2_avltree_delete(tree, &arr[2]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); 
        root = r2_avltree_search(tree, &arr[2]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[3]); 
        assert(root->left == NULL);
        assert(*(r2_uint64*)root->right->key == arr[4]);
        assert(root->ncount == 2);
        assert(root->height == 1);


        /*Delete key with 4*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[3]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[3]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[4]); 
        assert(root->left == NULL);
        assert(root->right == NULL);
        assert(root->ncount == 1);
        assert(root->height == 0);


        /*Delete key with 5*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[4]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[4]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[5]); 
        assert(root->left == NULL);
        assert(*(r2_uint64*)root->right->key == arr[6]);
        assert(root->ncount == 2);
        assert(root->height == 1);

        /*Delete key with 6*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[5]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[5]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[6]); 
        assert(root->left == NULL);
        assert(root->right == NULL);
        assert(root->ncount == 1);
        assert(root->height == 0);

        /*Delete key with 7*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[6]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[6]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[7]); 
        assert(root->left == NULL);
        assert(root->right == NULL);
        assert(root->ncount == 1);
        assert(root->height == 0);

        /*Delete key with 8*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[7]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[7]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[8]); 
        assert(root->left == NULL);
        assert(*(r2_uint64*)root->right->key == arr[9]);
        assert(root->ncount == 2);
        assert(root->height == 1);

        /*Delete key with 9*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[8]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[8]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        root = r2_avltree_search(tree, &arr[9]); 
        assert(root->left == NULL);
        assert(root->right == NULL);
        assert(root->ncount == 1);
        assert(root->height == 0);

        /*Delete key with 10*/
        ncount = tree->ncount;
        tree = r2_avltree_delete(tree, &arr[9]); 
       test_r2_avltree_certify(tree->root, tree->kcmp); ; 
        root = r2_avltree_search(tree, &arr[9]); 
        assert(root == NULL); /* key shouldn't exist.*/
        assert(tree->ncount == (ncount - 1));
        assert(r2_avltree_empty(tree) == TRUE);
                
        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief  Test the height functionality of the avl tree.
 * 
 */
static void test_r2_avltree_height()
{
        r2_int64 height = 0;
        const int s = 100000;
        int a[s]; 
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < s; ++i){
                a[i] = i;
                tree = r2_avltree_insert(tree, &a[i], &a[i]);
                if((tree->ncount % 1000) == 0){
                        height = log2(tree->ncount) * 1.44;  /* Height =  1.44 log n according to analysis.*/
                        assert(tree->root->height == r2_avltree_height(tree->root));
                }
        }

        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Tests the avl tree ncount functionality.
 * 
 */
static void test_r2_avltree_size()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i){
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);
                assert(tree->ncount == (i + 1));
                assert(r2_avltree_size(tree->root) == tree->ncount);
        }
                
        assert(r2_avltree_size(tree->root) == tree->ncount);
        /*free tree*/
        r2_destroy_avltree(tree);
}


/**
 * @brief       Tests the avl tree empty functionality.
 * 
 */
static void test_r2_avltree_empty()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        assert(r2_avltree_empty(tree) == TRUE);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);
        

        assert(r2_avltree_empty(tree) != TRUE);
        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Tests the tree level functionality
 * 
 */
static void test_r2_avltree_level()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);
        
        assert(r2_avltree_level(tree->root) == 0);
        assert(r2_avltree_level(r2_avltree_search(tree, &arr[9])) == 3);
        /*free tree*/
        r2_destroy_avltree(tree);      
}


/**
 * @brief Tests the avl tree at functionality.
 * 
 */
static void test_r2_avltree_at()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

         for(int i = 0; i < SIZE; ++i)
                assert(&arr[i] == r2_avltree_at(tree->root, i)->data);
         
                
        /*Find node at index 0.*/
        assert(r2_avlnode_min(tree->root) == r2_avltree_at(tree->root, 0));
        /*Find node at index 4*/
        assert(&arr[4] == r2_avltree_at(tree->root, 4)->data);
        /*Find node at index 9 */
        assert(r2_avlnode_max(tree->root) == r2_avltree_at(tree->root, 9));

        /*free tree*/
        r2_destroy_avltree(tree);
}


static void print_node(void *node, void *arg)
{
        struct r2_avlnode *root  = (struct r2_avlnode *)node; 
        printf(" %d", *((r2_int64 *)root->key));
}

/**
 * @brief       Tests inorder traversal.
 * 
 */
static void test_r2_avltree_inorder()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

        printf("\n/******************************************* AVL Inorder Traversal************************/\n");
        r2_avltree_inorder(tree->root, print_node, NULL);

        struct r2_avlnode *root = r2_avlnode_inorder_first(tree->root); 
        assert(root->data == &arr[0]);
        
        root = r2_avlnode_inorder_next(root); 
        assert(root->data == &arr[1]);

        root = r2_avlnode_max(tree->root);
        root = r2_avlnode_inorder_next(root); 
        assert(root == NULL); 



        printf("\n/*******************************************AVL Inorder Traversal Left************************/\n");
        r2_avltree_inorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************AVL Inorder Traversal Right************************/\n");
        r2_avltree_inorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_avltree(tree);
}


/**
 * @brief       Tests inorder traversal.
 * 
 */
static void test_r2_avltree_postorder()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

        printf("\n/*******************************************AVL Postorder Traversal************************/\n");
        r2_avltree_postorder(tree->root, print_node, NULL);

        struct r2_avlnode *root = r2_avlnode_postorder_first(tree->root); 
        assert(root->data == &arr[0]);
        
        root = r2_avlnode_postorder_next(root); 
        assert(root->data == &arr[2]);

        root = tree->root;
        root = r2_avlnode_postorder_next(root); 
        assert(root == NULL); 


        printf("\n/*******************************************AVL Postorder Traversal Left************************/\n");
        r2_avltree_postorder(tree->root->left, print_node, NULL);

        printf("\n/********************************************AVL Postorder Traversal Right************************/\n");
        r2_avltree_postorder(tree->root->right, print_node, NULL);


        /*free tree*/
        r2_destroy_avltree(tree);

}

/**
 * @brief       Tests preorder traversal.
 * 
 */
static void test_r2_avltree_preorder()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

        printf("\n/*******************************************AVL Preorder Traversal************************/\n");
        r2_avltree_preorder(tree->root, print_node, NULL);

        struct r2_avlnode *root = r2_avlnode_preorder_first(tree->root); 
        assert(root->data == &arr[3]);
        
        root = r2_avlnode_preorder_next(root); 
        assert(root->data == &arr[1]);

        root = r2_avlnode_max(tree->root);
        root = r2_avlnode_preorder_next(root); 
        assert(root == NULL); 

        printf("\n/*******************************************AVL Preorder Traversal Left************************/\n");
        r2_avltree_preorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************AVL Preorder Traversal Right************************/\n");
        r2_avltree_preorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Test get keys functionality.
 * 
 */
static void test_r2_avltree_getkeys()
{ 
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        assert(r2_avltree_get_keys(tree) == NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);    

        void **keys  = r2_avltree_get_keys(tree); 
        assert(keys != NULL);

        printf("\n/****************************Keys********************/\n");
        for(int i = 0; i < SIZE; ++i){
                printf(" %d", *(int *)keys[i]); 
                assert(tree->kcmp(keys[i], &arr[i]) == 0);
        }
                


        free(keys);
         /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief       Test get values functionality.
 * 
 */
static void test_r2_avltree_getvalues()
{
        
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        assert(r2_avltree_get_values(tree) == NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);    

        void **values  = r2_avltree_get_keys(tree); 
        assert( values != NULL);

        printf("\n/****************************Values********************/\n");
        for(int i = 0; i < SIZE; ++i){
                printf(" %d", *(int *)values[i]); 
                assert(tree->dcmp(values[i], &arr[i]) == 0);
        }
                

        free(values);
         /*free tree*/
        r2_destroy_avltree(tree);
}

/**
 * @brief Tests the range query functionality.
 * 
 */
static void test_r2_avltree_rangequery()
{
        struct r2_avltree *tree = r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);    
        r2_uint64 range[2] = {2, 9}; 
       
        printf("\n /*****************Range Query************/\n");
        r2_destroy_list(r2_avltree_range_query(tree, &range[0], &range[1],print_node, NULL));
        r2_destroy_avltree(tree);
}

static void* cpy(const void *data)
{
        long long int *copy = malloc(sizeof(r2_uint64));
        *copy = *((r2_uint64 *)data);
        return copy;  
}


/**
 * @brief       Tests the compare 
 * 
 */
static void test_r2_avltree_compare()
{

        struct r2_avltree *tree = r2_create_avltree(NULL, NULL, NULL, NULL, NULL, NULL);
        /*Shallow comparison*/
        assert(r2_avltree_compare(tree, tree) == TRUE); 
        /*Shallow comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, tree) == TRUE); 

        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

       /*Shallow comparison*/
        tree->kcmp = NULL;
        tree->dcmp = NULL;
        assert(r2_avltree_compare(tree, tree) == TRUE); 
    
        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, tree) == TRUE);

        /*Deep copy*/
        tree->kcpy = cpy;
        tree->dcpy = cpy;
        struct r2_avltree *copy = r2_avltree_copy(tree);

        /*Shallow comparison*/
        tree->kcmp = NULL;
        tree->dcmp = NULL;
        assert(r2_avltree_compare(tree, copy) != TRUE);

        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, copy) == TRUE); /*Deep comparison*/
        r2_destroy_avltree(tree);
        r2_destroy_avltree(copy);
}

static void test_r2_avltree_copy()
{
        
        struct r2_avltree *tree = r2_create_avltree(tree->kcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_avltree *copy = r2_avltree_copy(tree);

        /*Shallow comparison*/
        tree->kcmp = NULL;
        assert(r2_avltree_compare(tree, copy) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, copy) == TRUE); 
        r2_destroy_avltree(copy);

        /*Shallow comparison after deep copy*/
        tree->kcpy = cpy;
        tree->dcpy = cpy;
        tree->kcmp = cmp;
        tree->dcmp = NULL;
        copy = r2_avltree_copy(tree);
        tree->kcmp = NULL;
        assert(r2_avltree_compare(tree, copy) == TRUE);

        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, copy) == TRUE); 
        r2_destroy_avltree(copy);
        

        for(int i = 0; i < SIZE; ++i)
                tree = r2_avltree_insert(tree, &arr[i], &arr[i]);  

        /*Shallow comparison*/
        tree->kcpy = NULL;
        tree->dcpy = NULL;
        tree->kcmp = cmp;
        tree->dcmp = NULL;
        copy = r2_avltree_copy(tree);
        tree->kcmp = NULL;
        assert(r2_avltree_compare(tree, copy) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, copy) == TRUE); 
        r2_destroy_avltree(copy);


        
        /*Shallow comparison*/
        tree->kcpy = cpy;
        tree->dcpy = cpy;
        tree->kcmp = cmp;
        tree->dcmp = NULL;
        copy = r2_avltree_copy(tree);
        tree->kcmp = NULL;
        assert(r2_avltree_compare(tree, copy) != TRUE); 
        
        /*Deep comparison*/
        tree->kcmp = cmp;
        tree->dcmp = cmp;
        assert(r2_avltree_compare(tree, copy) == TRUE); 

        r2_destroy_avltree(tree);
        r2_destroy_avltree(copy);
}


static void test_r2_avltree_certify(const struct r2_avlnode *root, r2_cmp cmp)
{     
        test_r2_avltree_is_avltree(root);
        test_r2_avltree_is_binary_tree(root, cmp);

}

static void test_r2_avltree_is_avltree(const struct r2_avlnode *root)
{
        if(root == NULL)
                return;

        test_r2_avltree_is_avltree(root->left); 
        test_r2_avltree_is_avltree(root->right);
        r2_int64 leftheight     = r2_avltree_height(root->left)  + 1; 
        r2_int64 rightheight    = r2_avltree_height(root->right) + 1;
        r2_int64 balance_factor = leftheight - rightheight;
        assert(balance_factor >= -1 && balance_factor <= 1);


}

static void test_r2_avltree_is_binary_tree(const struct r2_avlnode *root, r2_cmp cmp)
{
        if(root == NULL)
                return; 

        test_r2_avltree_is_binary_tree(root->left, cmp); 
        test_r2_avltree_is_binary_tree(root->right, cmp); 
        if(root->left != NULL)
                assert(cmp(root->left->key, root->key) < 0);
        
        if(root->right != NULL)
                assert(cmp(root->right->key, root->key) > 0);
}

/**
 * @brief Tests insertion and deletion on massive dataset.
 * 
 */
static void test_r2_avltree_generate()
{
        FILE *dataset = fopen("../tests/sample.csv","r");
        struct r2_avltree  *avl =  r2_create_avltree(cmp, cmp, cpy, cpy, NULL, NULL);
        long long *data = malloc(sizeof(r2_uint64) *100000); 

        for(int i = 0; i < 100000;++i){  
             fscanf(dataset, "%lld", &data[i]);
             avl = r2_avltree_insert(avl, &data[i], &data[i]); 
        }

       test_r2_avltree_certify(avl->root, avl->kcmp); ; 

        for(int i = 0; i < 100000;++i){
                avl = r2_avltree_delete(avl, &data[i]); 
               test_r2_avltree_certify(avl->root, avl->kcmp); ; 
        }
            
        
        free(data);
        fclose(dataset);
        r2_destroy_avltree(avl); 
}


/**
 * @brief  Runs all the avl tree tests.
 * 
 */
void test_r2_avltree_run()
{
        test_r2_avltree_init_data(); 
        test_r2_create_avlnode();
        test_r2_create_avltree();
        test_r2_avltree_insert();
        test_r2_avlnode_successor();
        test_r2_avlnode_predecessor();
        test_r2_avlnode_max(); 
        test_r2_avlnode_min();
        test_r2_avltree_search();
        test_r2_avltree_delete();
        test_r2_avltree_height();
        test_r2_avltree_size();
        test_r2_avltree_level();
        test_r2_avltree_at();
        test_r2_avltree_inorder();
        test_r2_avltree_postorder();
        test_r2_avltree_preorder();
        test_r2_avltree_getkeys();
        test_r2_avltree_getvalues();
        test_r2_avltree_rangequery();
        test_r2_avltree_compare();
        test_r2_avltree_copy();
        test_r2_avltree_generate();
}
