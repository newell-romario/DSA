#ifndef R2_DOUBLE_LINKLIST_H_
#define R2_DOUBLE_LINKLIST_H_
#include "r2_types.h"

/**
 * A double linked list is a linear data structure where the node contains a previous and next pointer. 
 * The previous pointer as its name implies points to the previous node in the list and the next pointer points to the next node in the list. 
 * This setup is advantageous because it allows us to traverse the list in any direction (forward/reverse) 
 * and remove a node at any position in the list i.e. we’re no longer relegated to insertion and deletion only at the front/rear of the list. 
 * A double linked list offers more functions than the single linked list. 
 * In a double linked list we’re able to insert at front, insert at back, remove at back, remove at front, insert after position p, 
 * insert before position p, remove node at pos p and get node at position p.
 * 
 */
struct r2_listnode{
        void *data;/*Data*/
        struct r2_listnode *prev;/*Link to previous node*/
        struct r2_listnode *next;/*Link to next node*/
}; 


struct r2_list{
        struct r2_listnode *front;/*first node in list*/
        struct r2_listnode *rear;/*last node in list*/
        r2_int64 lsize;/*number of elements in list*/
        r2_cmp cmp;/*A comparison callback function*/
        r2_cpy cpy;/*A callback function to copy values*/
        r2_fd  fd;/*A callback function that release memory*/
}; 


struct r2_listnode* r2_listnode_at(const struct r2_list *, r2_uint64);
struct r2_listnode* r2_listnode_first(const struct r2_list *); 
struct r2_listnode* r2_listnode_last(const struct r2_list *);
struct r2_list*  r2_create_list(r2_cmp, r2_cpy, r2_fd); 
struct r2_list*  r2_destroy_list(struct r2_list *);
r2_uint16  r2_list_insert_at_front(struct r2_list *, void*); 
r2_uint16  r2_list_insert_at_back(struct r2_list *, void*); 
r2_uint16  r2_list_insert_after(struct r2_list *, struct r2_listnode *, void *);
r2_uint16  r2_list_insert_before(struct r2_list *, struct r2_listnode *, void *);
r2_uint16  r2_list_delete_at_front(struct r2_list *); 
r2_uint16  r2_list_delete_at_back(struct r2_list *);
r2_uint16  r2_list_delete(struct r2_list *, struct r2_listnode *);
struct r2_list* r2_list_copy(const struct r2_list *); 
r2_uint16  r2_list_compare(const struct r2_list *, const struct r2_list *);     
r2_uint16 r2_list_empty(const struct r2_list *);
#endif