#include "r2_wavltree_test.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>


static r2_int16 cmp(const void *, const void *);
static r2_int16 cmp2(const void *a, const void *b);
static r2_int64 r2_wavlnode_rank_diff(const struct r2_wavlnode *parent, const struct r2_wavlnode *root);
static void test_r2_wavltree_is_binary_tree(const struct r2_wavlnode *root, r2_cmp cmp);

/**
 * @brief Test the get keys functionality.
 * 
 */
static void test_r2_wavltree_get_keys()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        void **keys = r2_wavltree_get_keys(tree);

        assert(keys[0] == &arr[2]);
        assert(keys[1] == &arr[8]);
        assert(keys[2] == &arr[1]);
        assert(keys[3] == &arr[0]);
        assert(keys[4] == &arr[5]);
        assert(keys[5] == &arr[4]);
        assert(keys[6] == &arr[3]);
        assert(keys[7] == &arr[6]);
        assert(keys[8] == &arr[7]);
        free(keys);
        /*Destroy tree*/
        r2_destroy_wavltree(tree);  
}

/**
 * @brief Test get values functionality.
 * 
 */
static void test_r2_wavltree_get_values()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        void **values = r2_wavltree_get_values(tree);

        assert(values[0] == &arr[2]);
        assert(values[1] == &arr[8]);
        assert(values[2] == &arr[1]);
        assert(values[3] == &arr[0]);
        assert(values[4] == &arr[5]);
        assert(values[5] == &arr[4]);
        assert(values[6] == &arr[3]);
        assert(values[7] == &arr[6]);
        assert(values[8] == &arr[7]);
        free(values);
        /*Destroy tree*/
        r2_destroy_wavltree(tree);  
}

/**
 * @brief Tests the at functionality.
 * 
 */
static void test_r2_wavltree_at()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);   

        struct r2_wavlnode *root  = r2_wavltree_at(tree->root, 0); 
        assert(root != NULL); 
        assert(root->data == &arr[2]); 
        root  = r2_wavltree_at(tree->root, 8);
        assert(root != NULL); 
        assert(root->data == &arr[7]); 
        
        /*Destroy tree*/
        r2_destroy_wavltree(tree);

}


/**
 * @brief  Tests the min functionality.
 * 
 */
static void test_r2_wavlnode_min()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        struct r2_wavlnode *root = r2_wavlnode_min(tree->root);
        assert(root->data == &arr[2]);

        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief Tests the max functionality.
 * 
 */
static void test_r2_wavlnode_max()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        struct r2_wavlnode *root = r2_wavlnode_max(tree->root);
        assert(root->data == &arr[7]);

        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief  Tests successor functionality.
 * 
 */
static void test_r2_wavlnode_successor()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        struct r2_wavlnode *root = r2_wavltree_search(tree, &arr[4]); 
        struct r2_wavlnode *successor = r2_wavlnode_successor(root);
        assert(successor != NULL); 
        assert(successor->data == &arr[3]);
        successor = r2_wavlnode_successor(successor);
        assert(successor != NULL); 
        assert(successor->data == &arr[6]);
        root = r2_wavltree_search(tree, &arr[7]); 
        successor = r2_wavlnode_successor(root);
        assert(successor == NULL); 

        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief       Tests predecessor functionality.
 * 
 */
static void test_r2_wavlnode_predecessor()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        struct r2_wavlnode *root = r2_wavltree_search(tree, &arr[4]); 
        struct r2_wavlnode *predecessor = r2_wavlnode_predecessor(root);
        assert(predecessor != NULL); 
        assert(predecessor->data == &arr[5]);
        predecessor = r2_wavlnode_predecessor(predecessor);
        assert(predecessor->data == &arr[0]); 
        root = r2_wavltree_search(tree, &arr[2]); 
        predecessor = r2_wavlnode_predecessor(root);
        assert(predecessor == NULL);


        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief       Tests empty functionality of WAVL Tree.
 * 
 */
static void test_r2_wavltree_empty()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        assert(r2_wavltree_empty(tree) == TRUE); 
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        assert(r2_wavltree_empty(tree) != TRUE);   

        /*Destroy tree*/
        r2_destroy_wavltree(tree);        
}


/**
 * @brief   Test the WAVL tree create functionality.
 * 
 */
static void test_r2_create_wavltree()
{
        struct r2_wavltree *tree = r2_create_wavltree(NULL, NULL, NULL, NULL, NULL, NULL);
        assert(tree != NULL);
        assert(tree->root == NULL); 
        assert(tree->ncount == 0);
        assert(tree->kcmp == NULL); 
        assert(tree->dcmp == NULL); 
        assert(tree->kcpy == NULL); 
        assert(tree->dcpy == NULL);
        assert(tree->fk   == NULL); 
        assert(tree->fd   == NULL);
        assert(r2_wavltree_empty(tree) == TRUE); 
        
        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief       Tests the wavl node create functionality.
 * 
 */
static void test_r2_create_wavlnode()
{
        struct r2_wavlnode *root = r2_create_wavlnode(); 
        assert(root->key  == NULL);
        assert(root->data == NULL);
        assert(root->parent == NULL); 
        assert(root->left == NULL); 
        assert(root->right == NULL); 
        assert(root->ncount == 1); 
        assert(root->rank == 0);
        free(root); 
}

/**
 * @brief       Tests the wavl tree insert.
 * 
 */
static void test_r2_wavltree_insert()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i){
                r2_wavltree_insert(tree, &arr[i], &arr[i]);
                test_r2_wavltree_certify(tree->root);
        }
               

        assert(tree->ncount == 9); 
        assert(tree->root->data == &arr[0]);
        assert(tree->root->rank == 3);

        /*2*/
        struct r2_wavlnode *root = r2_wavltree_search(tree, &arr[1]);
        assert(root != NULL); 
        assert(root->rank == 0);
        assert(root->ncount == 1);
        assert(root->left == NULL);
        assert(root->right == NULL);
        
        /*1*/
        root = r2_wavltree_search(tree, &arr[2]);
        assert(root != NULL); 
        assert(root->rank == 0);
        assert(root->ncount == 1);
        assert(root->left == NULL);
        assert(root->right == NULL);

        /*5*/
        root = r2_wavltree_search(tree, &arr[3]);
        assert(root != NULL); 
        assert(root->rank == 0);
        assert(root->ncount == 1);
        assert(root->left == NULL);
        assert(root->right == NULL);

        /*4*/
        root = r2_wavltree_search(tree, &arr[4]);
        assert(root != NULL); 
        assert(root->ncount == 5);
        assert(root->rank == 2);
        assert(root->left->data == &arr[5]);
        assert(root->left->rank == 0);
        assert(root->left->ncount == 1);
        assert(root->right->data == &arr[6]);
        assert(root->right->ncount == 3);
        assert(root->right->rank == 1);

        /*7*/
        root = r2_wavltree_search(tree, &arr[7]);
        assert(root != NULL); 
        assert(root->ncount == 1);
        assert(root->rank == 0);
        assert(root->left == NULL);
        assert(root->right == NULL);

        /*6*/
        root = r2_wavltree_search(tree, &arr[6]);
        assert(root != NULL); 
        assert(root->ncount == 3);
        assert(root->rank == 1);
        assert(root->left->data == &arr[3]);
        assert(root->left->rank == 0);
        assert(root->left->ncount == 1);
        assert(root->right->data == &arr[7]);
        assert(root->right->ncount == 1);
        assert(root->right->rank == 0);

        /*1.5*/
        root = r2_wavltree_search(tree, &arr[8]);
        assert(root != NULL); 
        assert(root->ncount == 3);
        assert(root->rank == 1);
        assert(root->left->data == &arr[2]);
        assert(root->left->ncount == 1);
        assert(root->left->rank == 0);
        assert(root->right->data == &arr[1]);
        assert(root->right->ncount == 1);
        assert(root->right->rank == 0);

        /*Destroy tree*/
        r2_destroy_wavltree(tree);
}

/**
 * @brief       Tests the wavl tree delete.
 * 
 */
static void test_r2_wavltree_delete()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        struct r2_wavlnode *root = NULL;
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        /*Delete 1*/
        tree = r2_wavltree_delete(tree, &arr[2]); 
        test_r2_wavltree_certify(tree->root);
        assert(tree->ncount == 8);
        root = r2_wavltree_search(tree, &arr[2]);
        assert(root == NULL);
        root = r2_wavltree_search(tree, &arr[8]);
        assert(root != NULL);
        assert(root->rank == 1); 
        assert(root->ncount == 2); 
        assert(root->right->rank == 0); 
        assert(root->right->ncount == 1);
        assert(root->right->data == &arr[1]);
        
        /*Delete 2*/
        tree = r2_wavltree_delete(tree, &arr[1]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[1]);
        assert(root == NULL);
        assert(tree->root->data == &arr[4]);
        assert(tree->root->rank ==3 );
        root = r2_wavltree_search(tree, &arr[0]);
        assert(root != NULL);
        assert(root->rank == 2); 
        assert(root->ncount == 3); 
        assert(root->left->data  == &arr[8]); 
        assert(root->right->data == &arr[5]);
        assert(root->left->rank  == 0); 
        assert(root->right->rank == 0);
        assert(root->left->ncount  == 1); 
        assert(root->right->ncount == 1);

        /*Delete 5*/
        tree = r2_wavltree_delete(tree, &arr[3]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[3]);
        assert(root == NULL);
        root = r2_wavltree_search(tree, &arr[6]);
        assert(root != NULL);
        assert(root->rank == 1); 
        assert(root->ncount == 2); 
        assert(root->right->rank == 0); 
        assert(root->right->ncount == 1);
        assert(root->right->data == &arr[7]);

        /*Delete 7*/
        tree = r2_wavltree_delete(tree, &arr[7]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[7]);
        assert(root == NULL);
        root = r2_wavltree_search(tree, &arr[6]);
        assert(root != NULL);
        assert(root->rank == 0); 
        assert(root->ncount == 1); 
        assert(root->left == NULL); 
        assert(root->right == NULL);
        assert(tree->root->data == &arr[4]);
        assert(tree->root->rank == 2);
        root = r2_wavltree_search(tree, &arr[0]);
        assert(root != NULL);
        assert(root->rank == 1); 
        assert(root->ncount == 3); 
        assert(root->left->data  == &arr[8]); 
        assert(root->right->data == &arr[5]);
        assert(root->left->rank  == 0); 
        assert(root->right->rank == 0);
        assert(root->left->ncount  == 1); 
        assert(root->right->ncount == 1);

        /*Delete 1.5*/
        tree = r2_wavltree_delete(tree, &arr[8]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[8]);
        assert(root == NULL);
        root = r2_wavltree_search(tree, &arr[0]);
        assert(root != NULL);
        assert(root->rank == 1); 
        assert(root->ncount == 2); 
        assert(root->right->data == &arr[5]); 
        assert(root->right->rank == 0);
        assert(root->right->ncount == 1);


        /*Delete 6*/
        tree = r2_wavltree_delete(tree, &arr[6]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[6]);
        assert(root == NULL);
        assert(tree->root->data == &arr[5]);
        assert(tree->root->rank == 2);
        assert(tree->root->ncount == 3);
        root = tree->root;
        assert(root->right->data == &arr[4]); 
        assert(root->right->rank == 0);
        assert(root->right->ncount == 1);
        assert(root->left->data == &arr[0]); 
        assert(root->left->rank == 0);
        assert(root->left->ncount == 1);

        /*Insert 3.6. Then delete 3*/
        double  key = 3.6; 
        tree = r2_wavltree_insert(tree, &key, &key);
        tree = r2_wavltree_delete(tree, &arr[0]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[0]);
        assert(root == NULL);
        assert(tree->root->data == &key);
        assert(tree->root->rank == 2);
        assert(tree->root->ncount == 3);
        root = tree->root;
        assert(root->right->data == &arr[4]); 
        assert(root->right->rank == 0);
        assert(root->right->ncount == 1);
        assert(root->left->data == &arr[5]); 
        assert(root->left->rank == 0);
        assert(root->left->ncount == 1);

        /*Insert 3. Then delete 4*/
        tree = r2_wavltree_insert(tree, &arr[0], &arr[0]);
        tree = r2_wavltree_delete(tree, &arr[4]); 
        test_r2_wavltree_certify(tree->root);
        root = r2_wavltree_search(tree, &arr[4]);
        assert(root == NULL);
        assert(tree->root->data == &arr[5]);
        assert(tree->root->rank == 2);
        assert(tree->root->ncount == 3);
        root = tree->root;
        assert(root->right->data == &key); 
        assert(root->right->rank == 0);
        assert(root->right->ncount == 1);
        assert(root->left->data == &arr[0]); 
        assert(root->left->rank == 0);
        assert(root->left->ncount == 1);
        
        
        /*Destroy tree*/
        r2_destroy_wavltree(tree);     

        tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);   

        for(int i = 0; i < 9; ++i){
                test_r2_wavltree_certify(tree->root);
                tree = r2_wavltree_delete(tree,  &arr[i]);
                test_r2_wavltree_certify(tree->root);
        }
                

        assert(r2_wavltree_empty(tree) == TRUE);
        /*Destroy tree*/
        r2_destroy_wavltree(tree);      
}

/**
 * @brief       Test search functionality in a WAVL Tree
 * 
 */
static void test_r2_wavltree_search()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        struct r2_wavlnode *root = NULL;
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i){

                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);
                root = r2_wavltree_search(tree, &arr[i]); 
                assert(root->data == &arr[i]);
        }
                
        double key = 8; 
        root = r2_wavltree_search(tree, &key); 
        assert(root == NULL); 

        /*Destroy tree*/
        r2_destroy_wavltree(tree);

}

/**
 * @brief       Tests destroy functionality.
 * 
 */
static void test_r2_destroy_wavltree()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        struct r2_wavlnode *root = NULL;
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);
         

        /*Destroy tree*/
        assert(r2_destroy_wavltree(tree) == NULL);
}

static void *cpy(const void *data)
{
        double *copy = malloc(sizeof(double));
        *copy = *((double*)data);
        return copy;  
}

static void print_node(void *node, void *arg)
{
        struct r2_wavlnode *root  = (struct r2_wavlnode *)node; 
        printf(" %.1f", *((double *)root->key));
}

/**
 * @brief Tests WAVL Tree inorder functionality.
 * 
 */
static void test_r2_wavltree_inorder()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        printf("\n/*******************************************WAVL Inorder Traversal************************/\n");
        r2_wavltree_inorder(tree->root, print_node, NULL);

        struct r2_wavlnode *root = r2_wavlnode_inorder_first(tree->root); 
        assert(root->data == &arr[2]);
        
        root = r2_wavlnode_inorder_next(root); 
        assert(root->data == &arr[8]);

        root = r2_wavlnode_max(tree->root);
        root = r2_wavlnode_inorder_next(root); 
        assert(root == NULL); 


        printf("\n/*******************************************WAVL Inorder Traversal Left************************/\n");
        r2_wavltree_inorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************WAVL Inorder Traversal Right************************/\n");
        r2_wavltree_inorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_wavltree(tree);        
}


/**
 * @brief Test the WAVL Tree postorder
 * 
 */
static void test_r2_wavltree_postorder()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        printf("\n/*******************************************WAVL Postorder Traversal************************/\n");
        r2_wavltree_postorder(tree->root, print_node, NULL);

        struct r2_wavlnode *root = r2_wavlnode_postorder_first(tree->root); 
        assert(root->data == &arr[2]);
        
        root = r2_wavlnode_postorder_next(root); 
        assert(root->data == &arr[1]);

        root = r2_wavlnode_postorder_next(tree->root);  
        assert(root == NULL); 


        printf("\n/*******************************************WAVL Postorder Traversal Left************************/\n");
        r2_wavltree_postorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************WAVL Postorder Traversal Right************************/\n");
        r2_wavltree_postorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_wavltree(tree);     

}

/**Test WAVL Tree Preorder */
static void test_r2_wavltree_preorder()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);

        printf("\n/*******************************************WAVL Preorder Traversal************************/\n");
        r2_wavltree_preorder(tree->root, print_node, NULL);

        struct r2_wavlnode *root = r2_wavlnode_preorder_first(tree->root); 
        assert(root->data == &arr[0]);
        
        root = r2_wavlnode_preorder_next(root); 
        assert(root->data == &arr[8]);

        root = r2_wavlnode_max(tree->root);
        root = r2_wavlnode_preorder_next(root);  
        assert(root == NULL); 


        printf("\n/*******************************************WAVL Preorder Traversal Left************************/\n");
        r2_wavltree_preorder(tree->root->left, print_node, NULL);

        printf("\n/*******************************************WAVL Preorder Traversal Right************************/\n");
        r2_wavltree_preorder(tree->root->right, print_node, NULL);

        /*free tree*/
        r2_destroy_wavltree(tree);    

}

/**
 * @brief Test WAVL Tree copy.
 * 
 */
static void test_r2_wavltree_copy()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_wavltree *copy = r2_wavltree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_wavltree_compare(tree, copy) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_wavltree_compare(tree, copy) == TRUE); 
        r2_destroy_wavltree(copy);

        /*Deep copy*/
        tree->kcpy = cpy;
        tree->dcpy = cpy;
        copy = r2_wavltree_copy(tree);

        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_wavltree_compare(tree, copy) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;        
        assert(r2_wavltree_compare(tree, copy) == TRUE); 
        r2_destroy_wavltree(copy);
        
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);  


        /*Shallow comparison*/
        tree->kcpy = NULL;
        tree->dcpy = NULL;
        tree->kcmp = cmp; 
        tree->dcmp = NULL;
        copy = r2_wavltree_copy(tree);
        tree->kcmp = NULL; 
        assert(r2_wavltree_compare(tree, copy) == TRUE); 
        
        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;    
        assert(r2_wavltree_compare(tree, copy) == TRUE); 
        r2_destroy_wavltree(copy);

        /*Deep comparison*/
        tree->kcpy = cpy;
        tree->dcpy = cpy;
        copy = r2_wavltree_copy(tree);
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL;
        assert(r2_wavltree_compare(tree, copy) != TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;    
        assert(r2_wavltree_compare(tree, copy) == TRUE); 

        r2_destroy_wavltree(tree);
        r2_destroy_wavltree(copy);
}


/**
 * @brief Tests WAVL Tree comparison
 * 
 */
static void test_r2_wavltree_compare()
{

        struct r2_wavltree *tree = r2_create_wavltree(cmp, NULL, NULL, NULL, NULL, NULL);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL; 
        assert(r2_wavltree_compare(tree, tree) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_wavltree_compare(tree, tree) == TRUE);

        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);  

       
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL; 
        assert(r2_wavltree_compare(tree, tree) == TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_wavltree_compare(tree, tree) == TRUE); 
        
       
        tree->kcpy = cpy; 
        tree->dcpy = cpy;
        struct r2_wavltree *copy = r2_wavltree_copy(tree);
        
        /*Shallow comparison*/
        tree->kcmp = NULL; 
        tree->dcmp = NULL; 
        assert(r2_wavltree_compare(tree, copy) != TRUE); 

        /*Deep comparison*/
        tree->kcmp = cmp; 
        tree->dcmp = cmp;
        assert(r2_wavltree_compare(tree, copy) == TRUE); 
        r2_destroy_wavltree(tree);
        r2_destroy_wavltree(copy);
}

/**
 * @brief Tests range query functionality.
 * 
 */
static void test_r2_wavltree_range_query()
{
        struct r2_wavltree *tree = r2_create_wavltree(cmp, cmp, NULL, NULL, NULL, NULL);
        double arr[] = {3, 2, 1, 5, 4, 3.5, 6, 7, 1.5};
        for(int i = 0; i < 9; ++i)
                tree = r2_wavltree_insert(tree, &arr[i], &arr[i]);    
        double range[2] = {2, 6}; 
       
        printf("\n /*****************WAVL Range Query************/\n");
        r2_destroy_list(r2_wavltree_range_query(tree, &range[0], &range[1], print_node, NULL));
        r2_destroy_wavltree(tree);
}



/**
 * @brief       Certify the properties of WAVL Tree.
 * 
 */
static void test_r2_wavltree_certify(const struct r2_wavlnode *root)
{
        if(root == NULL)
                return;
        
        test_r2_wavltree_certify(root->left); 
        test_r2_wavltree_certify(root->right);

        assert(r2_wavlnode_rank_diff(root, root->left) == 1 || r2_wavlnode_rank_diff(root, root->left) == 2);
        assert(r2_wavlnode_rank_diff(root, root->right) == 1 || r2_wavlnode_rank_diff(root, root->right) == 2);
}


/**
 * @brief                       Calculates the rank difference of root.
 *                              Rank difference is rank(parent(root)) - rank(root)
 * 
 * @param root                  Tree root.
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
 * @brief Produces benchmarks for this WAVL tree.
 * 
 */
static void test_r2_wavltree_stats()
{
       struct r2_wavltree *wavl = r2_create_wavltree(cmp2, NULL, NULL, NULL, NULL, NULL);
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
                        wavl = r2_wavltree_insert(wavl, key, key);
                        elapse += (clock() - before)/(r2_ldbl)CLOCKS_PER_SEC;
                }

                if(wavl->ncount == 20000000)
                        break;
       }
       
        r2_uint64 height = r2_wavltree_height(wavl->root);
        printf("\nAverage insertion time: %Lf", elapse/wavl->ncount);
        printf("\nHeight: %ld", height);


        fclose(fp);
        fp =  fopen("com-friendster.ungraph.txt", "r"); 
        elapse = 0;
        
        /*File 30GB and we can't edit the file because of size. Skip first 15 lines because it's irrelevant*/
        for(r2_uint16 i = 0; i < 15; ++i){
                fscanf(fp, "%s", line);
        }
        
        r2_uint64 count = 0;
        while(fscanf(fp, "%lld\t%lld", &a[0], &a[1]) == 2){
                for(r2_uint16 i = 0; i < 2; ++i){
                        before = clock();
                        if(r2_wavltree_search(wavl, &a[i]) == NULL)
                                break;
                        elapse += (clock() - before)/(r2_ldbl)CLOCKS_PER_SEC;
                }
                if(count == 20000000)
                        break;
                ++count;
       }

       test_r2_wavltree_certify(wavl->root);
       test_r2_wavltree_is_binary_tree(wavl->root, cmp2);
       fclose(fp);  
       r2_destroy_wavltree(wavl);           
}

/**
 * @brief       Run all tests.
 * 
 */
void test_r2_wavltree_run()
{
        test_r2_wavltree_insert();
        test_r2_wavltree_delete();
        test_r2_wavltree_search();
        test_r2_create_wavlnode();
        test_r2_create_wavltree();
        test_r2_destroy_wavltree();
        test_r2_wavltree_empty();
        test_r2_wavlnode_successor();
        test_r2_wavlnode_predecessor();
        test_r2_wavlnode_max();
        test_r2_wavlnode_min();
        test_r2_wavltree_at();
        test_r2_wavltree_get_keys(); 
        test_r2_wavltree_get_values();  
        test_r2_wavltree_copy(); 
        test_r2_wavltree_compare();   
        test_r2_wavltree_inorder();
        test_r2_wavltree_postorder(); 
        test_r2_wavltree_preorder();
        test_r2_wavltree_range_query();
       // test_r2_wavltree_generate();
        test_r2_wavltree_stats();
}

static r2_int16 cmp(const void *a, const void *b)
{
        double *c = (double *)a; 
        double *d = (double *)b;

        if(*c > *d)
                return 1; 
        else if(*c < *d)
                return -1; 
        return 0; 
}


static void test_r2_wavltree_is_binary_tree(const struct r2_wavlnode *root, r2_cmp cmpfunc)
{
        if(root == NULL)
                return; 
        
        test_r2_wavltree_is_binary_tree(root->left, cmpfunc); 
        test_r2_wavltree_is_binary_tree(root->right, cmpfunc); 
        if(root->left != NULL)
                assert(cmpfunc(root->left->key, root->key) < 0);
        
        if(root->right != NULL)
                assert(cmpfunc(root->right->key, root->key) > 0);
}

static void test_r2_wavltree_generate()
{
        FILE *dataset = fopen("../tests/sample.csv","r");
        struct r2_wavltree  *wavl = r2_create_wavltree(cmp2, cmp2, NULL, NULL, NULL, NULL);
        long long *data = malloc(sizeof(long long) *100000); 

        for(int i = 0; i < 100000;++i){  
             fscanf(dataset, "%lld", &data[i]);
             wavl = r2_wavltree_insert(wavl, &data[i], &data[i]); 
        }

        test_r2_wavltree_certify(wavl->root); 
        test_r2_wavltree_is_binary_tree(wavl->root, cmp2);
 
        for(int i = 0; i < 100000;++i){
                wavl = r2_wavltree_delete(wavl, &data[i]); 
                test_r2_wavltree_certify(wavl->root);
                test_r2_wavltree_is_binary_tree(wavl->root, cmp2);
        }
            
        
        free(data);
        fclose(dataset);
        r2_destroy_wavltree(wavl); 
}

static r2_int16 cmp2(const void *a, const void *b)
{
        const long long int *c = a; 
        const long long int *d = b; 

       if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else 
                return 1;
}

