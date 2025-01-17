#include <stdlib.h> 
#include <assert.h>
#include "r2_list_test.h"
#define SIZE 1000

static int arr[SIZE];

/**
 * @brief       Initializes test data.
 * 
 */
static void test_r2_init_data()
{
        for(int i = 0; i < SIZE; ++i)
                arr[i] = rand() % SIZE + 1;
}

/**
 * @brief       Tests the create functionality for the list node.
 * 
 */
static void test_r2_create_listnode()
{
        struct r2_listnode *node = r2_create_listnode(); 
        assert(node != NULL);
        assert(node->data == NULL); 
        assert(node->next == NULL);
        assert(node->prev == NULL);
        free(node);
}

/**
 * @brief       Tests the create functionality for the list.
 * 
 */
static void test_r2_create_list()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 
        assert(list != NULL); 
        assert(list->front == NULL); 
        assert(list->rear  == NULL);
        assert(list->cmp   == NULL); 
        assert(list->cpy   == NULL); 
        assert(list->fd    == NULL);
        assert(list->lsize == 0);
        r2_destroy_list(list);
}

/**
 * @brief       Tests the destroy the functionality of the list.
 * 
 */
static void test_r2_destroy_list()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 
        assert(r2_destroy_list(list) == NULL);
}


/**
 * @brief       Tests at functionality of the list. 
 * 
 */
static void test_r2_listnode_at()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 

        for(r2_uint64 i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]); 
        

        struct r2_listnode *pos = NULL;
        for(int i = 0; i < SIZE; ++i){
                pos = r2_listnode_at(list, i);
                assert(pos->data == &arr[i]);
        }

        pos = r2_listnode_at(list, list->lsize);
        assert(pos == NULL);
        r2_destroy_list(list);
}


/**
 * @brief      Tests the first functionality of the list.
 * 
 */
static void test_r2_listnode_first()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode *pos = NULL; 

        for(int i = 0; i < SIZE; ++i){
                list = r2_list_insert_at_front(list, &arr[i]);
                pos  = r2_listnode_first(list); 
                assert(pos->data == &arr[i]);  
        }

        r2_destroy_list(list);
}


/**
 * @brief       Tests the last functionality of the list.
 * 
 */
static void test_r2_listnode_last()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode *pos = NULL; 

        for(int i = 0; i < SIZE; ++i){
                list = r2_list_insert_at_back(list, &arr[i]);
                pos  = r2_listnode_last(list); 
                assert(pos->data == &arr[i]);  
        }

        r2_destroy_list(list);
}


/**
 * @brief       Tests the insert at front of the list functionality.
 * 
 */
static void test_r2_list_insert_at_front()
{
        struct r2_list         *list = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode     *pos  = NULL; 

        for(int i = 0; i < SIZE; ++i){
                list = r2_list_insert_at_front(list, &arr[i]);
                pos  = r2_listnode_first(list);
                assert(pos->data == &arr[i]);
        }

        r2_destroy_list(list);
}

/**
 * @brief       Tests the insert at back of the list functionality.
 * 
 */
static void test_r2_list_insert_at_back()
{
        struct r2_list     *list   = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode *pos    = NULL;

        for(int i = 0; i < SIZE; ++i){
                list = r2_list_insert_at_back(list, &arr[i]);
                pos  = r2_listnode_last(list); 
                assert(pos->data == &arr[i]);
        } 

        r2_destroy_list(list);
}

/**
 * @brief       Tests the insert after functionality of the list.  
 *
 *  
 */
static void test_r2_list_insert_after()
{
        struct r2_list *list   = r2_create_list(NULL, NULL, NULL);  
        
        /**
         * @brief Tests insert after for empty list.
         * 
         */
        struct r2_listnode *first    = r2_listnode_at(list, 0); /* first node in the list.*/
        list  = r2_list_insert_after(list, first, &arr[0]);
        first = r2_listnode_at(list, 0); 
        assert(first->next == NULL);
        assert(first->prev == NULL);
        assert(first->data == &arr[0]);
        assert(list->lsize  ==  1);
        
        int a[3] = {1997, 20, 11};
       
       
       /**
        * @brief Test insert after the first node. N.B Only one node is in the list.
        * 
        */
        first    = r2_listnode_at(list,0); /* first node in the list.*/
        list     = r2_list_insert_after(list, first, &a[0]);  
        struct r2_listnode      *second = r2_listnode_at(list, 1); /*this is the new node containing the address of a[0].*/
        struct r2_listnode      *third  = r2_listnode_at(list, 2); /*third node in the list.*/
        assert(first->data == &arr[0]);
        assert(first->next  == second);
        assert(second->prev == first);
        assert(second->data == &a[0]);
        assert(second->next == third); 
        assert(third == NULL); /*Only two node in the list at this time so the third node must be null.*/
        assert(list->lsize ==  2);

        
    
      
        /**
         * @brief Test insert after the last node. N.B. Only two node in the list.
         * 
         */

        r2_int64 pos = list->lsize - 1;
        struct r2_listnode      *last    = r2_listnode_at(list, pos); /* last node in the list.*/
        list    = r2_list_insert_after(list, last, &a[1]);  
        struct r2_listnode      *last_next = r2_listnode_at(list, pos + 1); /*this is the new last node in the list.*/
        struct r2_listnode      *last_next_next   = r2_listnode_at(list, pos + 2); /*this is the node after the last node in the list.*/
        assert(last->data == &a[0]);
        assert(last->next == last_next); 
        assert(last_next->prev == last);
        assert(last_next->next == last_next_next);
        assert(last_next->data == &a[1]); 
        assert(last_next_next  == NULL);
        assert(list->lsize == 3);

        /**
         * @brief Insert in the middle of the list.
         * 
         */
        
        pos = 1;
        struct r2_listnode      *cur    = r2_listnode_at(list,pos); /*  node at pos in the list.*/
        list    = r2_list_insert_after(list, cur, &a[2]);  
        struct r2_listnode      *cur_next = r2_listnode_at(list, pos + 1); /*this node is now at pos in the list.*/
        struct r2_listnode      *cur_next_next   = r2_listnode_at(list, pos + 2); /*this is the node after pos in the list.*/
        assert(cur->data == &a[0]);
        assert(cur->next == cur_next); 
        assert(cur_next->prev == cur);
        assert(cur_next->data == &a[2]);
        assert(cur_next->next == cur_next_next); 
        assert(cur_next_next->data == &a[1]); 
        assert(list->lsize == 4);

        
       r2_destroy_list(list);
}

/**
 * @brief       Tests the insert before functionality.
 * 
 */
static void test_r2_list_insert_before()
{
        struct r2_list          *list   = r2_create_list(NULL, NULL, NULL);  
        
        /**
         * @brief Tests insert before for empty list.
         * 
         */
        struct r2_listnode      *first    = r2_listnode_at(list, 0); /* first node in the list.*/
        list  = r2_list_insert_before(list, first, &arr[0]);
        first = r2_listnode_at(list, 0); 
        assert(first->next == NULL);
        assert(first->prev == NULL);
        assert(first->data == &arr[0]);
        assert(list->lsize  ==  1);
        
        int a[3] = {1997, 20, 11};
       
       
       /**
        * @brief Test insert before the first node. N.B Only one node is in the list.
        * 
        */
        first    = r2_listnode_at(list,0); /* first node in the list.*/
        list     = r2_list_insert_before(list, first, &a[0]);  
        first    = r2_listnode_at(list,0); /* first node in the list containing address of a[0]*/
        struct r2_listnode      *second = r2_listnode_at(list, 1); /*this is the old node containing the address of arr[0].*/
        struct r2_listnode      *third  = r2_listnode_at(list, 2); /*third node in the list.*/
        assert(first->data  == &a[0]);
        assert(first->next  == second);
        assert(second->prev == first);
        assert(second->data == &arr[0]);
        assert(second->next == third); 
        assert(third == NULL); /*Only two node in the list at this time so the third node must be null.*/
        assert(list->lsize ==  2);

        
    
        /**
         * @brief Test insert before the last node. N.B. Only two node in the list.
         * 
         */

        size_t pos = list->lsize - 1;
        struct r2_listnode      *last    = r2_listnode_at(list, pos); /* last node in the list.*/
        list    = r2_list_insert_before(list, last, &a[1]);  
        struct r2_listnode      *last_prev = r2_listnode_at(list, pos); /*the new node in the list with address a[1].*/
        struct r2_listnode      *last_prev_prev   = r2_listnode_at(list, pos - 1); /*this is the node before the new node in the list.*/
        assert(last->data == &arr[0]);
        assert(last->prev == last_prev);
        assert(last_prev->data == &a[1]); 
        assert(last_prev_prev == last_prev->prev);
        assert(last_prev_prev->data == &a[0]);
        assert(list->lsize == 3);

        /**
         * @brief Insert in the middle of the list.
         * 
         */
        
        pos = 1;
        struct r2_listnode      *cur    = r2_listnode_at(list,pos); /*  node at pos in the list.*/
        list    = r2_list_insert_before(list, cur, &a[2]);  
        cur    = r2_listnode_at(list,pos); /*  node at pos in the list.*/

        struct r2_listnode      *prev = r2_listnode_at(list, pos - 1); /*this node is now at pos in the list.*/
        struct r2_listnode      *next   = r2_listnode_at(list, pos + 1); /*this is the node after pos in the list.*/
        assert(cur->data == &a[2]);
        assert(cur->prev == prev); 
        assert(prev->data == &a[0]);
        assert(prev->next == cur);
        assert(cur->next == next);
        assert(next->prev == cur);
        assert(next->data == &a[1]); 
        assert(list->lsize == 4);

        
       r2_destroy_list(list);
}


/**
 * @brief       Tests the delete at front functionality.
 * 
 */
static void test_r2_list_delete_at_front()
{
        struct r2_list     *list   = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode *pos    = NULL;

        for(int i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]);
        
        for(int i = 0; i <= SIZE - 2; i += 2){
                pos  = r2_listnode_first(list);
                assert(pos->data == &arr[i]);
                list = r2_list_delete_at_front(list);
                pos  = r2_listnode_first(list);
                assert(pos->data == &arr[i + 1]);
                list = r2_list_delete_at_front(list);
        }

        assert(r2_list_empty(list) == TRUE); 
        r2_destroy_list(list);
}


/**
 * @brief       Tests the delete at back functionality.
 * 
 */
static void test_r2_list_delete_at_back()
{
        struct r2_list     *list   = r2_create_list(NULL, NULL, NULL); 
        struct r2_listnode *pos    = NULL;

        for(int i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]);
        
        for(int i = SIZE; i >= 2; i -= 2){   
                pos  = r2_listnode_last(list);
                assert(pos->data == &arr[i - 1]);
                list = r2_list_delete_at_back(list);
                pos  = r2_listnode_last(list);
                assert(pos->data == &arr[i - 2]);
                list = r2_list_delete_at_back(list);
        }

        assert(r2_list_empty(list) == TRUE); 
        r2_destroy_list(list);
}

/**
 * @brief       Tests the delete functionality.
 * 
 */
static void test_r2_list_delete()
{
        struct r2_list     *list   = r2_create_list(NULL, NULL, NULL);     
        r2_int64 pos = 0; 
        
        /*Tries delete a position in an empty list. Shouldn't work.*/
        struct r2_listnode *cur = r2_listnode_at(list, pos); 
        list = r2_list_delete(list, cur);
        
        /**
         * If successfully the size would decrease by 1. 
         * This would make the list empty function return FALSE. 
         * See the definiton of the list empty function as to why.
         * 
         */
        assert(r2_list_empty(list) == TRUE); 

        int a[5] = {1997,20,11,2024,2023};
        for(int i = 0; i < 5; ++i)
                list = r2_list_insert_at_back(list, &a[i]);
        
        
        /*Test deleting the first node of the list.*/
        pos  = 0;
        cur  = r2_listnode_at(list, pos); 
        list = r2_list_delete(list, cur);
        cur  = r2_listnode_at(list, pos); 
        assert(cur->data == &a[1]);
        assert(list->lsize == 4);

        /*Test deleting the last node of the list.*/
        pos  = list->lsize - 1;
        cur  = r2_listnode_at(list, pos); 
        list = r2_list_delete(list, cur);
        pos  = list->lsize - 1;
        cur  = r2_listnode_at(list, pos); 
        assert(cur->data == &a[3]);
        assert(list->lsize == 3);

        /*Test deleting the middle node*/
        pos  = 1;
        cur  = r2_listnode_at(list, pos); 
        list = r2_list_delete(list, cur);
        pos  = list->lsize - 1;
        cur  = r2_listnode_at(list, pos); 
        assert(cur->data == &a[3]);
        pos  = 0;
        cur  = r2_listnode_at(list, pos); 
        assert(cur->data == &a[1]);
        assert(list->lsize == 2);
}

static void* cpy(const void *data)
{
        int *ptr = malloc(sizeof(int)); 
        if(ptr != NULL)
                *ptr = *((int *)data);

        return ptr;
}

static r2_int16 cmp(const void *d1, const void *d2)
{
        return (*((int *)d1)) == (*((int *)d2));
}

/**
 * @brief       Tests the copy functionality of the stack.
 * 
 */
static void test_r2_list_copy()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 

        /*Shallow copy on empty list.*/
        struct r2_list *copy = r2_list_copy(list);
        
        /*Shallow comparison*/
        assert(r2_list_compare(list, copy) == TRUE); 

        /*Deep comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, copy) == TRUE);
        r2_destroy_list(copy);

        /*Deep copy on empty list*/
        list->cpy = cpy;
        copy = r2_list_copy(list);
        /*Shallow comparison*/
        list->cmp = NULL;
        assert(r2_list_compare(list, copy) == TRUE); 

        /*Deep comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, copy) == TRUE);
        r2_destroy_list(copy);


        for(int i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]); 
        
        
        /*Shallow copy on list.*/
        list->cpy = NULL;
        copy = r2_list_copy(list);
        
        /*Shallow comparison*/
        list->cmp = NULL;
        assert(r2_list_compare(list, copy) == TRUE); 
        
        /*Deep comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, copy) == TRUE);
        r2_destroy_list(copy);

        /*Deep copy on list.*/
        list->cpy = cpy;
        copy = r2_list_copy(list);
        /*Shallow comparison*/
        list->cmp = NULL;
        assert(r2_list_compare(list, copy) != TRUE); 
        
        /*Deep comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, copy) == TRUE);
        
        r2_destroy_list(copy);
        r2_destroy_list(list);
}

/**
 * @brief       Test the comparison functionality. 
 * 
 */
static void test_r2_list_compare()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 

        /*Shallow comparison.*/
        assert(r2_list_compare(list, list) == TRUE); 

        /*Deep Comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, list) == TRUE);


        for(int i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]); 
        
        
        
        /*Comparison on  list.*/
        /*Shallow comparison*/
        list->cmp = NULL;
        assert(r2_list_compare(list, list) == TRUE); 
        /*Deep Comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, list) == TRUE);
       

        /*Comparison on list after deep copy.*/
        list->cpy = cpy;
        struct r2_list *copy = r2_list_copy(list);
        
        /*Shallow comparison*/
        list->cmp = NULL;
        assert(r2_list_compare(list, copy) != TRUE); 
        
        /*Deep Comparison*/
        list->cmp = cmp;
        assert(r2_list_compare(list, copy) == TRUE);
        r2_destroy_list(copy);
        r2_destroy_list(list);
} 

/**
 * @brief 
 * 
 */
static void test_r2_list_empty()
{
        struct r2_list *list = r2_create_list(NULL, NULL, NULL); 
        assert(r2_list_empty(list) == TRUE); 

        for(int i = 0; i < SIZE; ++i)
                list = r2_list_insert_at_back(list, &arr[i]); 

        assert(r2_list_empty(list) != TRUE); 
        r2_destroy_list(list);
}       

/**
 * @brief 
 * 
 */
void test_r2_list_run()
{
        test_r2_init_data();
        test_r2_create_listnode(); 
        test_r2_create_list(NULL, NULL, NULL); 
        test_r2_destroy_list();
        test_r2_listnode_at(); 
        test_r2_listnode_first(); 
        test_r2_listnode_last();
        test_r2_list_insert_at_front();
        test_r2_list_insert_at_back();
        test_r2_list_insert_after(); 
        test_r2_list_insert_before();
        test_r2_list_delete_at_front(); 
        test_r2_list_delete_at_back(); 
        test_r2_list_delete();
        test_r2_list_copy(); 
        test_r2_list_compare(); 
        test_r2_list_empty();
}