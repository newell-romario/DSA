#include "r2_sort.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define CUT_OFF 32

static void swap(char *, char *, r2_uint64);
static void cpy(void *, void *, r2_uint64);
static void merge(char *, char *, r2_uint64, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void merge_sort(void *, void *, r2_uint64, r2_uint64 , r2_uint64 , r2_cmp );
static void merge_sort_mod(void *, void*, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void bmerge_sort(void *, void*, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void bmerge_sort_mod(void *, void*, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void is_sorted(void *, r2_uint64, r2_uint64, r2_uint64,r2_cmp);
static r2_int64 hoare(char*, r2_int64, r2_int64, r2_uint64, r2_cmp);
static r2_int64 lomuto(char*, r2_uint64, r2_uint64, r2_int64, r2_cmp);
static void quick_sort(void *, r2_int64, r2_int64, r2_uint64, r2_cmp);
static r2_int64 newell(char *, r2_int64, r2_int64, r2_int64, r2_cmp);
static void bubble_up(char *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void bubble_down(char *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
static void quick_sort_mod(void *, r2_int64, r2_int64, r2_uint64, r2_cmp);
/**
 * @brief               Sorts a sequence in non-decreasing order using insertion sort. 
 *                
 * @param arr           Array.
 * @param start         Start. 
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_insertion_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        /**
         * @brief We reserve 64 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 64 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[64] = {0};
        char *buf  = buffer;
        if(es > 64)
                buf = malloc(sizeof(char) *es);
        
        char *seq  = arr;
        r2_int64 l = 0;
        if(buf != NULL){
                for(r2_uint64 j = start + 1; j < as; ++j){
                        cpy(&seq[j*es], buf, es);
                        l = j - 1;
                        for(;l >= (r2_int64)start && cmp(&seq[l*es], buf) > 0; --l)
                                cpy(&seq[l*es], &seq[(l + 1)*es] , es);
                        
                        cpy(buf, &seq[(l + 1)*es], es);
                }

                if(buf != buffer)
                        free(buf);
        }

        is_sorted(arr, start, as, es, cmp);  
}


/**
 * @brief               Sorts a sequence in non-decreasing order using selection sort. 
 *                      
 * @param arr           Array.
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_selection_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq  = arr;
        /**
         * @brief We reserve 64 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 64 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[64] = {0};
        char *buf  = buffer;
        if(es > 64)
                buf = malloc(sizeof(char) *es);
        r2_int64 l = 0;
        r2_int64 k = 0;
        if(buf != NULL){
                for(r2_uint64 j = start; j < as - 1; ++j){
                        k = j;
                        for(r2_uint64 l = j + 1; l < as; ++l)
                                if(cmp(&seq[l*es], &seq[k*es]) < 0)
                                        k = l;
                        if(k != j){
                                cpy(&seq[j*es], buf, es);
                                cpy(&seq[k*es], &seq[j*es],es);
                                cpy(buf, &seq[k*es], es); 
                        }
                }

                if(buf != buffer)
                        free(buf);
        }
        
        is_sorted(arr, start, as, es, cmp);  
}

/**
 * @brief               Sorts a sequence in non-decreasing order using bubble sort. 
 *                     
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_bubble_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq  = arr;
        /**
         * @brief We reserve 64 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 64 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[64] = {0};
        char *buf  = buffer;
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        r2_uint16 swap = FALSE;
        r2_int64 l = 0;
        if(buf != NULL){
                for(r2_uint64 j = start; j < as; ++j){
                        swap = FALSE;
                        for(r2_uint64 k = start; k < as -1; ++k){
                                if(cmp(&seq[k*es], &seq[(k+1)*es]) > 0){
                                        cpy(&seq[k*es], buf, es); 
                                        cpy(&seq[(k+1)*es], &seq[k*es], es);
                                        cpy(buf, &seq[(k+1)*es], es);
                                        swap = TRUE;
                                }
                        }

                        if(swap == FALSE)
                                break;
                }
        }
        if(buf != buffer)
                free(buf);
        
        is_sorted(arr, start, as, es, cmp);  
}

/**
 * @brief               Sorts a sequence in non-decreasing order using shell sort. 
 *                      
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_shell_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq  = arr;
        /**
         * @brief We reserve 64 bytes for a single element of the array.  
         * Our assumption is that a single element is less than 64 bytes. 
         * However we dynamically allocate more memory if our assumption is wrong.
         */
        char buffer[64] = {0};
        char *buf  = buffer;
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        r2_int64 h = 1; 
        r2_int64 l = 0;
        while(h <= (as - start) /3)
                h = h * 3 + 1;

        for(;h > 0; h/=3){
                for(r2_uint64 i = start + h; i < as; ++i){
                        cpy(&seq[i*es], buf, es);
                        l = i - h; 
                        for(;l >= (r2_int64)start && cmp(&seq[l*es], buf) > 0;l-=h)
                                cpy(&seq[l*es], &seq[(l+h)*es], es);
                        
                        cpy(buf, &seq[(l + h)*es], es);
                }
        }

        if(buf != buffer)
                free(buf);
        
        is_sorted(arr, start, as, es, cmp);  
}

/**
 * @brief               Sorts a sequence in non-decreasing order using mergesort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_merge_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq = malloc(es * as); 
        assert(seq != NULL);
        merge_sort(arr, seq, start, as -1, es, cmp);
        is_sorted(arr, start, as, es, cmp);  
        free(seq);
}

/**
 * @brief               Sorts a sequence in non-decreasing order using a modified mergesort. 
 *                      The key difference in this merge sort is that we use cut off point 
 *                      for small subarrays. What this means is that we don't recurse on smaller
 *                      subarrays instead we use insertion sort to sort those arrays.
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_merge_sort_mod(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp) 
{
        char *seq = malloc(es * as); 
        assert(seq != NULL);
        merge_sort_mod(arr, seq, start, as -1, es, cmp);
        is_sorted(arr, start, as, es, cmp);   
        free(seq);
}

/**
 * @brief               Sorts a sequence in non-decreasing order using bottom up mergesort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_bmerge_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq = malloc(es * as); 
        assert(seq != NULL);
        bmerge_sort(arr, seq, start, as, es, cmp);
        is_sorted(arr, start, as, es, cmp);  
        free(seq);
}

/**
 * @brief               This function merges the sub arrays into a sorted array.
 * 
 * @param seq           Seq. 
 * @param aux           Auxillary Array.
 * @param start         Start.
 * @param mid           Middle.
 * @param end           End.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static void merge(char *seq, char *aux, r2_uint64 start, r2_uint64 mid, r2_uint64 end, r2_uint64 es, r2_cmp cmp)
{
        /**
         * @brief Copy contents of seq into aux.
         * 
         */
        memcpy(&aux[start*es], &seq[start*es], (end - start + 1)*es);
        for(r2_uint64 j = start, k = mid + 1, l = start; l <= end; ++l){  
                if(j <= mid && k <= end){
                        if(cmp(&aux[j*es], &aux[k*es]) <= 0){
                                cpy(&aux[j*es], &seq[l*es], es);
                                ++j;
                        }else{
                                cpy(&aux[k*es], &seq[l*es], es);
                                ++k;    
                        }
                }else if(k <= end){
                        cpy(&aux[k*es], &seq[l*es], es);
                        ++k; 
                }else{
                        cpy(&aux[j*es], &seq[l*es], es);
                        ++j;   
                }
        }
}

/**
 * @brief               Helper function for mergesort.
 * 
 * @param arr           Array.
 * @param seq           Sequence. 
 * @param start         Start of array.
 * @param as            Array size.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static void merge_sort(void *arr, void *seq, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        if(as <= start) 
                return; 
        r2_uint64 mid = start + (as - start) / 2;
        merge_sort(arr, seq, start, mid, es, cmp);
        merge_sort(arr, seq, mid + 1, as, es, cmp); 
        merge(arr, seq, start, mid, as, es, cmp);
}

/**
 * @brief               Helper function for mergesort mod.
 * 
 * @param arr           Array.
 * @param seq           Sequence. 
 * @param start         Start of array.
 * @param as            Array size.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static void merge_sort_mod(void *arr, void *seq, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        if(as <= start) 
                return; 
        else if((as - start) <= CUT_OFF){
                r2_shell_sort(arr, start, as + 1, es, cmp);
                return;
        }
                
        r2_uint64 mid = start + (as - start) / 2;
        merge_sort_mod(arr, seq, start, mid, es, cmp);
        merge_sort_mod(arr, seq, mid + 1, as, es, cmp); 
        merge(arr, seq, start, mid, as, es, cmp);      
}

/**
 * @brief               Helper function for bottom up merge sort.
 * 
 * @param arr           Array.
 * @param seq           Sequence. 
 * @param start         Start of array.
 * @param as            Array size.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static void bmerge_sort(void *arr, void *seq, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        r2_uint64 mid = 0;
        r2_uint64 end = 0; 
        for(r2_uint64 i = 1; i < as; i <<= 1){                
                for(r2_uint64 low = start; low < as - i; low += 2*i){
                        mid = low + i - 1;
                        end = low + i*2 - 1;
                        if(as -1 < end)
                                end = as - 1;
                        merge(arr, seq, low, mid, end, es, cmp);
                }
        } 
}


/**
 * @brief               Sorts a sequence in non-decreasing order using bottom up mergesort with insertion sort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_bmerge_sort_mod(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char *seq = malloc(es * as); 
        assert(seq != NULL);
        bmerge_sort_mod(arr, seq, start, as, es, cmp);
        free(seq);
        is_sorted(arr, start, as, es, cmp);  
}


/**
 * @brief               Helper function for bottom up merge sort.
 * 
 * @param arr           Array.
 * @param seq           Sequence. 
 * @param start         Start of array.
 * @param as            Array size.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static void bmerge_sort_mod(void *arr, void *seq, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        r2_uint64 i = CUT_OFF;
        r2_uint64 low = start;
        r2_uint64 mid = 0;
        r2_uint64 end = 0; 
        if(as <= CUT_OFF){
                r2_shell_sort(arr, start, as, es, cmp);
                return;
        }

        do{
                mid = low + i - 1;
                end = low + i*2 - 1;
                if(as - 1 < end)
                        end = as - 1;
                r2_shell_sort(arr, low, end + 1, es, cmp); 
                low += 2*i;
        }while(end != as - 1);
        
        for(;i < as; i <<= 1){
                for(low = start; low < as - i; low += 2*i){
                        mid = low + i - 1;
                        end = low + i*2 - 1;
                        if(as - 1 < end)
                                end = as - 1;
                        merge(arr, seq, low, mid, end, es, cmp);                                       
                }
        }        
}

/**
 * @brief               Sorts a sequence in non-decreasing order using quicksort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_quick_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        quick_sort(arr, start, as - 1, es, cmp);
        is_sorted(arr, start, as, es, cmp);
}


/**
 * @brief               Helper function for quick sort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
static void quick_sort(void *arr, r2_int64 start, r2_int64 as, r2_uint64 es, r2_cmp cmp)
{
        if(start >= as)
                return;
        
        r2_int64 mid  = hoare(arr, start, as, es, cmp);
        quick_sort(arr, start, mid -1, es, cmp); 
        quick_sort(arr, mid + 1, as, es, cmp);
}



/**
 * @brief               Hoare partition function for quicksort.
 * 
 * @param arr           Array.
 * @param start         Start
 * @param end           End.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static r2_int64 hoare(char *arr, r2_int64 start, r2_int64 end, r2_uint64 es, r2_cmp cmp)
{
        char buffer[64] = {0};
        char *buf  = buffer;
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        r2_int64 l  = start; 
        r2_int64 r  = end-1;
        if(buf != NULL){
                while(l <= r){
                        while(l <= r && cmp(&arr[l*es], &arr[end*es]) <= 0)
                                ++l;

                        while(r >= l && cmp(&arr[r*es], &arr[end*es]) >= 0)
                                --r;
                 
                        if(l < r){
                                cpy(&arr[r*es], buf, es); 
                                cpy(&arr[l*es], &arr[r*es], es);
                                cpy(buf, &arr[l*es], es);
                        }
                }
                cpy(&arr[l*es], buf, es); 
                cpy(&arr[end*es], &arr[l*es], es);
                cpy(buf, &arr[end*es], es);
        }

        if(buf != buffer)
                free(buf);

        return l;
}

/**
 * @brief               Lomuto partition function for quicksort.
 * 
 * @param arr           Array.
 * @param start         Start
 * @param end           End.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static r2_int64 lomuto(char*arr, r2_uint64 start, r2_uint64 end, r2_int64 es, r2_cmp cmp)
{ 
        char buffer[64] = {0}; 
        char *buf   = buffer; 
        if(es > 64)
                buf = malloc(sizeof(char) *es);
        r2_int64 i = start -1; 
        for(r2_uint64 j = start; j <= end - 1; ++j){
                if(cmp(&arr[j*es], &arr[end*es]) <= 0){
                        ++i; 
                        cpy(&arr[i*es], buf, es); 
                        cpy(&arr[j*es], &arr[i*es], es);
                        cpy(buf, &arr[j*es], es);
                }
        }

        ++i;
        cpy(&arr[i*es], buf, es); 
        cpy(&arr[end*es], &arr[i*es], es);
        cpy(buf, &arr[end*es], es); 

        if(buf != buffer)
                free(buf);
        return i; 
}

/**
 * @brief               A partition function that tries to find the largest sorted sub array.
 *                      
 *                      
 * @param arr           Array.
 * @param start         Start
 * @param end           End.
 * @param es            Element size.
 * @param cmp           A comparison callback function.
 */
static r2_int64 newell(char *arr, r2_int64 start, r2_int64 end, r2_int64 es, r2_cmp cmp)
{
        char buffer[64] = {0}; 
        char *buf   = buffer; 
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        r2_int64 cl = start; 
        r2_int64 cr = start;
        r2_int64 ml;
        r2_int64 mr;
        for(r2_int64 left = start; left <= (end - 1); ++left){
                ml = mr = left;
                while(left <= (end -1) && cmp(&arr[left*es], &arr[(left+1)*es]) <= 0){
                        ++mr;
                        ++left;
                } 
                
                if((cr - cl + 1) <  (mr - ml + 1)){
                        cl = ml; 
                        cr = mr;
                }
        }

        if((end - start + 1) == (cr - cl + 1))
                return start + (end - start)/2;

        r2_int64 rl = end; 
        r2_int64 re = end;
        for(r2_int64 right = end; right > start; --right){
                ml = mr = right; 
                while(right > start && cmp(&arr[right*es], &arr[(right-1)*es]) <= 0){
                        --mr;
                        --right;
                }

                if((rl - re + 1) < (ml - mr + 1)){
                        rl = ml; 
                        re = mr;
                }
        }

        if((end - start + 1) == (rl - re + 1)){
                for(r2_int64 left = start, right = end; left < right; ++left, --right){
                        cpy(&arr[left*es], buf, es); 
                        cpy(&arr[right*es], &arr[left*es],  es); 
                        cpy(buf, &arr[right*es], es);
                }
                return start + (end - start)/2;
        }

        r2_int64 pivot = cr; 
        if((cr - cl + 1) <= (rl - re + 1))
                pivot = re;
 
        cpy(&arr[pivot*es], buf, es);
        cpy(&arr[end*es], &arr[pivot*es], es); 
        cpy(buf, &arr[end*es], es);

        r2_int64 l  = start; 
        r2_int64 r  = end-1;
        if(buf != NULL){
                while(l <= r){
                        while(l <= r && cmp(&arr[l*es], &arr[end*es]) <= 0)
                                ++l;

                        while(r >= l && cmp(&arr[r*es], &arr[end*es]) >= 0)
                                --r;
                 
                        if(l < r){
                                cpy(&arr[r*es], buf, es); 
                                cpy(&arr[l*es], &arr[r*es], es);
                                cpy(buf, &arr[l*es], es);
                        }
                }
                cpy(&arr[l*es], buf, es); 
                cpy(&arr[end*es], &arr[l*es], es);
                cpy(buf, &arr[end*es], es);
        }

        if(buf != buffer)
                free(buf);

        return l;
}

/**
 * @brief               Finds the correct position of where an element should be inserted
 *                      in a heap. 
 * 
 * @param arr           Array.
 * @param start         Start.
 * @param root          Root.
 * @param es            Element size. 
 * @param cmp           A comparison callback function.
 */
static void bubble_up(char *arr, r2_uint64 start, r2_uint64 root, r2_uint64 es, r2_cmp cmp)
{
        char buffer[64] = {0}; 
        char *buf   = buffer; 
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        assert(buf != NULL);

        r2_uint64 parent = root/2;
        while(root != start && cmp(&arr[parent*es], &arr[root*es]) < 0){
                cpy(&arr[root*es], buf, es);
                cpy(&arr[parent*es], &arr[root*es], es);
                cpy(buf, &arr[parent*es], es);
                root = parent; 
                parent = root/2;
        }

        if(buf != buffer)
                free(buf);
}

/**
 * @brief               Finds the correct position of where an element should be inserted
 *                      in a heap. 
 * 
 * @param arr           Array.
 * @param root          Root.
 * @param end           End.
 * @param es            Element size.
 * @param cmp           A comparison callback function. 
 */
static void bubble_down(char *arr, r2_uint64 root, r2_uint64 end, r2_uint64 es, r2_cmp cmp)
{
        char buffer[64] = {0}; 
        char *buf   = buffer; 
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        assert(buf != NULL);

        r2_uint64 l = 0;
        r2_uint64 r = 0;
        r2_uint64 largest = root;
        while(root < end){
                l = 2*root + 1; 
                r = 2*root + 2;
                        
                if(l <= end && cmp(&arr[l*es], &arr[largest*es]) > 0)
                        largest = l;

                if(r <= end && cmp(&arr[r*es], &arr[largest*es]) > 0)
                        largest = r;

                if(cmp(&arr[largest*es], &arr[root*es]) <= 0)
                        break;
                
                cpy(&arr[root*es], buf, es);
                cpy(&arr[largest*es], &arr[root*es], es);
                cpy(buf, &arr[largest*es], es);
                root = largest;
                
        }

        if(buf != buffer)
                free(buf);
}

/**
 * @brief               Sorts a sequence in non-decreasing order using heapsort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_heap_sort(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        char buffer[64] = {0}; 
        char *buf   = buffer; 
        if(es > 64)
                buf = malloc(sizeof(char) *es);

        assert(buf != NULL);

        char *seq = arr;
        /**
         * @brief Bottom up heap construction
         * 
         */
        for(r2_int64 i = (as-1)/2; i >= (r2_int64)start; --i)
                bubble_down(arr, i, as-1, es, cmp);
        
        for(r2_int64 i = as-1; i > (r2_int64)start; --i){
                cpy(&seq[i*es], buf, es);
                cpy(&seq[start*es], &seq[i*es], es);
                cpy(buf, &seq[start*es], es);
                bubble_down(seq, start, i-1, es, cmp);
        }

        is_sorted(arr, start, as, es, cmp);
        if(buf != buffer)
                free(buf);
}

/**
 * @brief               Helper function for quick sort. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
static void quick_sort_mod(void *arr, r2_int64 start, r2_int64 as, r2_uint64 es, r2_cmp cmp)
{
        if(as - start <= CUT_OFF){
                r2_shell_sort(arr, start, as + 1, es, cmp);
                return;
        }
                
        r2_int64 mid  = hoare(arr, start, as, es, cmp);
        quick_sort(arr, start, mid -1, es, cmp); 
        quick_sort(arr, mid + 1, as, es, cmp);
}

/**
 * @brief               Sorts a sequence in non-decreasing order using quicksort with shell short. 
 *             
 *          
 * @param arr           Array. 
 * @param start         Start.
 * @param as            Array size. Example if we are sorting 10 numbers then array size should be 10.
 * @param es            Element size. Example if we're sorting an array of 4 byte integers
 *                      then es should be equal to 4.
 * @param cmp           A callback comparison function that compares two elements a and b.
 */
void r2_quick_sort_mod(void *arr, r2_uint64 start, r2_uint64 as, r2_uint64 es, r2_cmp cmp)
{
        quick_sort_mod(arr, start, as - 1, es, cmp);
        is_sorted(arr, start, as, es, cmp);
}


/**
 * @brief       Copies src into dest.
 * 
 * @param src   Src. 
 * @param dest  Dest.   
 * @param size  Number of bytes.
 */
static inline void cpy(void *src, void *dest, r2_uint64 size)
{
        memmove(dest, src, size);
}


static void is_sorted(void *arr, r2_uint64 start ,r2_uint64 size, r2_uint64 ez,r2_cmp cmp)
{
        char *seq = arr;
        for(r2_uint64 i = start; i < size -1; ++i)
                assert(cmp(&seq[i*ez], &seq[(i+1)*ez]) <= 0);
}