#include "r2_btree_test.h"
#include "..\src\r2_queue.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static r2_int16 cmp(const void *, const void *);
static void print_page(const struct r2_page *);
static void print_tree(const struct r2_btree *);

/**
 * @brief   Test create B Tree functionality.
 * 
 */
static void test_r2_create_btree()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        assert(btree != NULL); 
        assert(btree->order  == order); 
        assert(btree->root   == NULL); 
        assert(btree->ncount == 0);
        assert(btree->kcmp   == cmp); 
        assert(btree->fk     == NULL);

        /*Destroy B Tree*/
        r2_destroy_btree(btree);
}


/**
 * @brief  Test create page functionality.
 * 
 */
static void test_r2_create_page()
{
        r2_int64 order = 4;
        struct r2_page *page = r2_create_page(order); 
        assert(page != NULL); 
        assert(page->ncount == 1); 
        assert(page->leaf == TRUE); 
        assert(page->mkeys == order); 
        assert(page->nkeys == 0); 
        assert(page->children != NULL);
        assert(page->indexes != NULL);


        /*Call function to free page*/
        free(page->children);
        free(page->indexes);
        free(page);
}

/**
 * @brief Test B Tree destroy functionality
 * 
 */
static void test_r2_destroy_btree()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);

        /*Destroy B Tree*/
        r2_destroy_btree(btree);
}

/**
 * @brief Test B Tree insert functionality
 * 
 */
static void test_r2_btree_insert()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i){
                btree = r2_btree_insert(btree, &arr[i]);
                test_r2_btree_certify(btree->root, cmp);
        }
             

        assert(btree != NULL); 
        assert(btree->ncount == 9);
        assert(btree->root->indexes[0] == &arr[3]);
        struct r2_page *page = NULL; 
        page = r2_page_minimum(btree->root);
        assert(page->indexes[0] == &arr[16]);
        page = r2_page_maximum(btree->root); 
        assert(page->indexes[1] == &arr[18]);

        

        /*Testing root*/
        page = btree->root;
        assert(page->indexes[0] == &arr[3]);
        
        /*Testing root children 1*/
        page = page->children[0];
        assert(page->indexes[0] == &arr[14]);
        assert(page->indexes[1] == &arr[5]);

        /*Testing root children 2*/
        page = btree->root;
        page = page->children[1];
        assert(page->indexes[0] == &arr[0]);
        assert(page->indexes[1] == &arr[13]);

        /*Testing children of root children 1*/
        page = btree->root;
        page = page->children[0];
        page = page->children[0];
        assert(page->indexes[0] == &arr[16]);
        assert(page->indexes[1] == &arr[15]);

        /*Testing children of root children 1*/
        page = btree->root;
        page = page->children[0];
        page = page->children[1];
        assert(page->indexes[0] == &arr[4]);
        assert(page->indexes[1] == &arr[1]);

        /*Testing children of root children 1*/
        page = btree->root;
        page = page->children[0];
        page = page->children[2];
        assert(page->indexes[0] == &arr[6]);
        assert(page->indexes[1] == &arr[7]);
        assert(page->indexes[2] == &arr[9]);

        /** Testing children of root children 2 */
        page = btree->root;
        page = page->children[1];
        page = page->children[0];
        
        assert(page->indexes[0] == &arr[11]);
        assert(page->indexes[1] == &arr[12]);
        assert(page->indexes[2] == &arr[8]);

        /** Testing children of root children 2 */
        page = btree->root;
        page = page->children[1];
        page = page->children[1];
        assert(page->indexes[0] == &arr[10]);
        assert(page->indexes[1] == &arr[2]);

        /** Testing children of root children 2 */
        page = btree->root;
        page = page->children[1];
        page = page->children[2];
        assert(page->indexes[0] == &arr[17]);
        assert(page->indexes[1] == &arr[18]);

        /*Destroy B Tree*/
        r2_destroy_btree(btree);
}

/**
 * @brief Tests the B Tree search functionality.
 * 
 */
static void test_r2_btree_search()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);
        
        struct r2_page *page = r2_btree_search(btree, &arr[0]); 
        assert(page != NULL); 

        double key = 50; 
        page = r2_btree_search(btree, &key);
        assert(page == NULL);

        /*Destroy BTree*/
        r2_destroy_btree(btree);
}

/**
 * @brief       Tests the btree minimum functionality.
 * 
 */
static void test_r2_btree_minimum()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);
        
        struct r2_page *page = r2_page_minimum(btree->root); 
        assert(page != NULL); 
        assert(page->indexes[0] == &arr[16]);
       
        /*Destroy BTree*/
        r2_destroy_btree(btree);
}

/**
 * @brief       Tests the btree maximum functionality.
 * 
 */
static void test_r2_btree_maximum()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);

        struct r2_page *page = r2_page_maximum(btree->root); 
        assert(page != NULL); 
        assert(page->indexes[1] == &arr[18]);

        /*Destroy BTree*/
        r2_destroy_btree(btree);
}

/**
 * @brief       Tests the successor functionality.
 * 
 */
static void test_r2_page_successor()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL);
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);

        /*Successor of 15*/
        struct r2_page *page = r2_page_successor(btree->root, &arr[3], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[0] == &arr[11]);

        /*Successor of 11*/
        page = r2_btree_search(btree, &arr[5]);
        page = r2_page_successor(page, &arr[5], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[0] == &arr[6]);

        /*Successor of 14*/
        page = r2_btree_search(btree, &arr[9]);
        page = r2_page_successor(page, &arr[9], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[0] == &arr[3]);

        /*Successor of 31*/
        page = r2_btree_search(btree, &arr[18]);
        page = r2_page_successor(page, &arr[18], btree->kcmp); 
        assert(page == NULL); 
        
        /*Destroy BTree*/
        r2_destroy_btree(btree);
}

/**
 * @brief Test the predecessor functionality.
 * 
 */
static void test_r2_page_predecessor()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL); 
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);

        /*Predecessor of 15*/
        struct r2_page *page = r2_page_predecessor(btree->root, &arr[3], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[2] == &arr[9]);

        /*Predecessor of 11*/
        page = r2_btree_search(btree, &arr[5]);
        page = r2_page_predecessor(page, &arr[5], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[1] == &arr[1]);

        /*Predecessor of 14*/
        page = r2_btree_search(btree, &arr[9]);
        page = r2_page_predecessor(page, &arr[9], btree->kcmp); 
        assert(page != NULL); 
        assert(page->indexes[1] == &arr[7]);

        /*Predecessor of 6*/
        page = r2_btree_search(btree, &arr[16]);
        page = r2_page_predecessor(page, &arr[16], btree->kcmp); 
        assert(page == NULL); 
        
        /*Destroy BTree*/
        r2_destroy_btree(btree);
}

r2_int16 cmp(const void *a, const void *b)
{
        double  *c = (double *)a; 
        double  *d = (double *)b; 

        if(*c < *d)
                return -1; 
        else if(*c > *d)
                return 1; 
        else
                return 0;
}

/**
 * @brief       Prints out the keys on a page.
 * 
 * @param page  Page that will be printed.
 */
static void print_page(const struct r2_page *page)
{
        if(page != NULL){
                for(int i = 0; i < page->nkeys; ++i){
                        printf(" %.2lf", *(double *)page->indexes[i]);
                }
        }
}

/**
 * @brief               Prints a B TREE in a depth first traversal style.
 * 
 * @param btree         B Tree to print.
 */
static void print_tree(const struct r2_btree *btree)
{
        struct r2_queue *queue = r2_create_queue(btree->kcmp, NULL,btree->fk); 
        struct r2_page  *page  = btree->root;
        r2_int64 depth = 0;
        if(page != NULL)
                queue = r2_queue_enqueue(queue, page);
        struct r2_queuenode *front = NULL;
        while(r2_queue_empty(queue) != TRUE){
                front  = r2_queue_front(queue);
                page   = front->data;
                printf("\n--------------Page %d--------------\n", depth);
                print_page(page);
                printf("\n--------------Page %d--------------\n", depth);
                for(int i = 0; page->children[i] != NULL; ++i){
                        queue = r2_queue_enqueue(queue, page->children[i]);
                }

                ++depth;
                queue = r2_queue_dequeue(queue);    
        }

        queue = r2_destroy_queue(queue);
}
/**
 * @brief       Tests the delete functionality.
 * 
 */
static void test_r2_btree_delete()
{
        r2_int64 order = 4;
        struct r2_btree *btree = r2_create_btree(order, cmp, NULL); 
        
        double arr[] = {20, 10, 25, 15, 9, 11, 12, 13, 19, 14, 21, 16, 17, 26, 8 , 7, 6, 30, 31};
        for(int i = 0; i < 19; ++i)
                btree = r2_btree_insert(btree, &arr[i]);

  
        struct r2_page * root = NULL;
        void *key = &arr[9]; 
        btree = r2_btree_delete(btree, key);
        root = r2_btree_search(btree, key);
        assert(root == NULL);
        test_r2_btree_certify(btree->root, cmp);

        key   = &arr[7]; 
        btree = r2_btree_delete(btree, key);
        root  = r2_btree_search(btree, key);
        assert(root == NULL);
        test_r2_btree_certify(btree->root, cmp);

        root = btree->root;
        assert(root->ncount == 6); 
        assert(root->indexes[0] == &arr[14]);
        assert(root->indexes[1] == &arr[3]);
        assert(root->indexes[2] == &arr[0]);
        assert(root->indexes[3] == &arr[13]);

        assert(root->children[0]->indexes[0] == &arr[16]);
        assert(root->children[1]->indexes[0] == &arr[4]);
        assert(root->children[2]->indexes[0] == &arr[11]);
        assert(root->children[3]->indexes[0] == &arr[10]);
        assert(root->children[4]->indexes[0] == &arr[17]);

        key   = &arr[16]; 
        btree = r2_btree_delete(btree, key);
        root  = r2_btree_search(btree, key);
        assert(root == NULL);
        test_r2_btree_certify(btree->root, cmp);
        root = btree->root;
        assert(root->ncount == 6); 
        assert(root->indexes[0] == &arr[4]);
        assert(root->indexes[1] == &arr[3]);
        assert(root->indexes[2] == &arr[0]);
        assert(root->indexes[3] == &arr[13]);

        assert(root->children[0]->indexes[0] == &arr[15]);
        assert(root->children[1]->indexes[0] == &arr[1]);
        assert(root->children[2]->indexes[0] == &arr[11]);
        assert(root->children[3]->indexes[0] == &arr[10]);
        assert(root->children[4]->indexes[0] == &arr[17]);

        for(int i = 0; i < 19; ++i){
                btree = r2_btree_delete(btree, &arr[i]);
                test_r2_btree_certify(btree->root, cmp);
        }
                
        /*Free B Tree*/
        r2_destroy_btree(btree);
        
        
}

/**
 * @brief               Certifies a B tree.
 * 
 * @param page          Page.
 * @param cmp           A comparison callback function.
 * @return
 */
static void test_r2_btree_certify(const struct r2_page *page, r2_cmp cmp)
{
        if(page == NULL)
                return; 
        
        assert(page->nkeys <= page->mkeys); 
        
        if(page->parent != NULL)
                assert(page->nkeys >= (page->mkeys / 2));

        test_r2_btree_height(page);

        for(int i = 0; i < page->nkeys -1; ++i)
                assert(cmp(page->indexes[i], page->indexes[i + 1]) < 0);

        struct r2_page *child = NULL;
        for(int i = 0; i <= page->nkeys - 1; i+= 2){
                child = page->children[i]; 
                if(child != NULL)
                        assert(cmp(child->indexes[0], page->indexes[i]) < 0); 
                test_r2_btree_certify(child, cmp);

                child = page->children[i + 1]; 
                if(child != NULL)
                        assert(cmp(child->indexes[0], page->indexes[i]) > 0); 

                test_r2_btree_certify(child, cmp);
        }        
}

/**
 * @brief       Test B Tree functionality on a random dataset.
 * 
 */
static void test_r2_btree_generate()
{
        FILE *dataset = fopen("../tests/sample.csv","r");
        double *data  = malloc(sizeof(double) *100000); 
        struct r2_btree *btree = r2_create_btree(32, cmp, NULL); 

        for(int i = 0; i < 100000; ++i){  
             fscanf(dataset, "%lf", &data[i]);
             btree = r2_btree_insert(btree, &data[i]);
        }

        printf("\nTree height:  %d", r2_page_height(btree->root));
        test_r2_btree_certify(btree->root, cmp);
        for(int i = 0; i < 100000; ++i){
                btree = r2_btree_delete(btree, &data[i]);
                test_r2_btree_certify(btree->root, cmp);
        }
            
        
        free(data);
        fclose(dataset);
        r2_destroy_btree(btree);
}

/**
 * Test the B Tree height functionality.
 * 
 */
static void test_r2_btree_height(const struct r2_page *page)
{
        for(int i = 0; i <= page->nkeys - 1; ++i){
                assert(r2_page_height(page->children[i]) == r2_page_height(page->children[i + 1]));
        }
    
}

void test_r2_btree_run()
{
        test_r2_create_btree(); 
        test_r2_create_page();
        test_r2_destroy_btree(); 
        test_r2_btree_insert();  
        test_r2_btree_search();  
        test_r2_btree_minimum();
        test_r2_btree_maximum();
        test_r2_page_successor();
        test_r2_page_predecessor();
        test_r2_btree_delete();
        test_r2_btree_generate();
}