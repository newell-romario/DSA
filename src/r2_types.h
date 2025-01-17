#ifndef R2_TYPES_H_
#define R2_TYPES_H_
#define TRUE  1
#define FALSE 0
#define PROFILE_TREE

typedef unsigned int            r2_uint16; 
typedef unsigned long           r2_uint32;
typedef unsigned long long      r2_uint64; 
typedef int                     r2_int16;
typedef long int                r2_int32; 
typedef long long int           r2_int64;   
typedef unsigned long long      size_t;       
typedef long double             r2_ldbl;
typedef unsigned char           r2_uc;

/*A callback function used to create a copy of any value.*/
typedef void* (*r2_cpy)(const void *); 

/*A callback function used to compare two values. */
typedef r2_int16 (*r2_cmp)(const void *, const void*); 

/*A callback function used to free the data portion of a node when the node is being destroyed.*/
typedef void (*r2_fd)(void *); 
/*A callback function used to free the key portion of a node when the node is being destroyed.*/
typedef void (*r2_fk)(void *);

/*A callback function used to perform an user defined action on value*/
typedef void (*r2_act)(void *, void *); 

#endif