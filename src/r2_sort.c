#include "r2_sort.h"
#include <stdlib.h>
#include <string.h>

static void swap(char *, char *, r2_uint64);
static void cpy(r2_uc *, r2_uc *, r2_uint64);
/**
 * @brief               Sorts a sequence in non-decreasing order using insertion sort. 
 *             
 *          
 * @param arr           Array. 
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_insertion_sort(void *arr, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        /**
         * @brief We reserve 1024 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 1024 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[1024] = {0};
        char *buf  = buffer;
        if(es > 1024)
                buf = malloc(sizeof(char) *es);
        
        char *seq  = arr;
        r2_int64 l = 0;
        if(buf != NULL){
                for(r2_uint64 j = 1; j < as; ++j){
                        cpy(&seq[j*es], buf, es);
                        l = j - 1;
                        for(;l >= 0 && cmp(&seq[l*es], buf) > 0 ; --l)
                                cpy(&seq[l*es], &seq[(l + 1)*es] , es);
                        
                        cpy(buf, &seq[(l + 1)*es], es);
                }
        }
}

/**
 * @brief               Sorts a sequence in non-decreasing order using selection sort sort. 
 *             
 *          
 * @param arr           Array. 
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_selection_sort(void *arr, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq  = arr;
        /**
         * @brief We reserve 1024 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 1024 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[1024] = {0};
        char *buf  = buffer;
        if(es > 1024)
                buf = malloc(sizeof(char) *es);
        r2_int64 l = 0;
        r2_int64 k = 0;
        if(buf != NULL){
                for(r2_uint64 j = 0; j < as - 1; ++j){
                        k = j;
                        for(r2_uint64 l = j + 1; l < as; ++l)
                                if(cmp(&seq[l*es], &seq[k*es]) < 0)
                                        k = l;
                        cpy(&seq[j*es], buf, es);
                        cpy(&seq[k*es], &seq[j*es],es);
                        cpy(buf, &seq[k*es], es);
                }
        }
}


/**
 * @brief       Copies src into dest.
 * 
 * @param src   Src. 
 * @param dest  Dest.   
 * @param size  Number of bytes.
 */
static inline void cpy(r2_uc *src, r2_uc *dest, r2_uint64 size)
{
        memcpy(dest, src, size);
}