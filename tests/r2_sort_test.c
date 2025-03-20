#include "r2_sort_test.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static r2_int16 int_cmp(const void *, const void *);
static r2_int16 char_cmp(const void *, const void *);
static r2_int16 double_cmp(const void *, const void *);
static void is_sorted(void *, r2_uint64, r2_uint64, r2_uint64 ,r2_cmp);
static void print_ints(r2_int64 *, r2_uint64); 
static void print_double(r2_dbl *, r2_uint64); 
static void print_char(r2_c *, r2_uint64);

static void test_insertion_sort()
{
        printf("\n--------------------------------Insertion Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9,  -6};
        void *seq[] = {unsorted, sorted, mixed};
        r2_uint64 size [] = {9, 10, 8};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], size[i]);
                r2_insertion_sort(seq[i], 0,  size[i], sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0 ,size[i], sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i],  size[i]);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_insertion_sort(alphabet, 0,26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_insertion_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0,10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        } 
        
        /*Testing insertion sort not starting from zero*/
        printf("\nInsertion Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_insertion_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_selection_sort()
{
        printf("\n--------------------------------Selection Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_selection_sort(seq[i], 0,10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_selection_sort(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0,26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_selection_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0,10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        }   
        
        /*Testing Selection sort not starting from zero*/
        printf("\nSelection Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_selection_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_r2_bubble_sort()
{
        printf("\n--------------------------------Bubble Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_bubble_sort(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0,10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_bubble_sort(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_bubble_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0,10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        }  
        
        /*Testing bubble sort not starting from zero*/
        printf("\nBubble Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_bubble_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_r2_shell_sort()
{
        printf("\n--------------------------------Shell Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_shell_sort(seq[i], 0,10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0,10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_shell_sort(alphabet, 0,26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_shell_sort(&vals[i], 0,10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0,10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        }     


        /*Testing Shell sort not starting from zero*/
        printf("\nShell Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_shell_sort(keys, 2, 5, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 2, 5, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_r2_merge_sort()
{
        printf("\n--------------------------------Merge Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_merge_sort(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_merge_sort(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_merge_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        } 
        
        /*Testing Merge sort not starting from zero*/
        printf("\nMerge Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_merge_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_r2_merge_sort_mod()
{
        printf("\n--------------------------------Merge Sort With Insertion Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_merge_sort_mod(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_merge_sort_mod(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_merge_sort_mod(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        } 
        
        /*Testing Merge sort not starting from zero*/
        printf("\nMerge Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_merge_sort_mod(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_bmerge_sort()
{
        printf("\n--------------------------------Bottom Up Merge Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_bmerge_sort(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_bmerge_sort(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0,26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_bmerge_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        }
        
        /*Testing Merge sort not starting from zero*/
        printf("\nBMerge Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_bmerge_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_bmerge_sort_mod()
{
        printf("\n--------------------------------Bottom Up Merge Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_bmerge_sort_mod(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_bmerge_sort_mod(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_bmerge_sort_mod(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        } 
        
        /*Testing Merge sort not starting from zero*/
        printf("\nMerge Sort with random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_bmerge_sort_mod(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);
}

static void test_r2_quick_sort()
{
        printf("\n--------------------------------Quick Sort----------------------------------------\n");
        r2_int64 unsorted[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        r2_int64 sorted[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        r2_int64 mixed[]    = {1, 10, 5, 4, 2, 10, 9, 8, 7, -6};
        void *seq[] = {unsorted, sorted, mixed};
        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_ints(seq[i], 10);
                r2_quick_sort(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                is_sorted(seq[i], 0, 10, sizeof(r2_int64), int_cmp);
                printf("\nAfter:");
                print_ints(seq[i], 10);
        }

        r2_c alphabet[] = {'a', 'c', 'f', 'b', 'z', 'm', 'x', 'g', 'e', 'r', 'q', 'j', 'k', 'v', 'u', 'w', 'y', 'h', 'i' ,'l', 'n', 'o', 'p', 'd', 's', 't'};
        printf("\nBefore:");
        print_char(alphabet, 26);
        r2_quick_sort(alphabet, 0, 26, sizeof(r2_c), char_cmp);     
        is_sorted(alphabet, 0, 26, sizeof(r2_c), char_cmp);
        printf("\nAfter:");
        print_char(alphabet, 26);

        r2_dbl vals[][10] = {{.12, 0.002, 5.14, -.0111, 3.14, 1.498, .451, -99, 100, 8},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                             {1, 10, 5, 4, 2, 10, 9, 8, 7, -6}};

        for(r2_uint16 i = 0; i < 3; ++i){
                printf("\nBefore:");
                print_double(vals[i], 10);
                r2_quick_sort(&vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                is_sorted(vals[i], 0, 10, sizeof(r2_dbl), double_cmp);
                printf("\nAfter:");
                print_double(vals[i], 10);
        } 
        
        /*Testing Merge sort not starting from zero*/
        printf("\nQuick Sort random start: ");
        r2_uint64 keys[] = {5, 1, 5, 2, 4, 3};
        r2_quick_sort(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        is_sorted(keys, 1, 6, sizeof(r2_uint64), int_cmp);
        print_ints(keys, 6);        
}

static void test_r2_sort_stats()
{
        FILE *fp = fopen("reverse_sorted.txt", "r"); 
        const r2_int64 size = 100000;
        r2_int64 *num = malloc(sizeof(r2_int64) * size); 
        r2_int64 line = 0;
        printf("\nreverse_sorted");
        while(line != size){
                fscanf(fp, "%lld", &num[line]);
                printf("\n%lld)%lld", line, num[line]);
                ++line;
        }

        clock_t before = clock(); 
        r2_merge_sort(num, 0, size, sizeof(r2_int64), int_cmp);
        double after = (double)(clock() - before) / CLOCKS_PER_SEC;
        is_sorted(num, 0, size, sizeof(r2_int64), int_cmp);
        fclose(fp);
        fp = fopen("results.txt", "w");
        for(r2_uint64 i = 0; i < size; ++i)
                fprintf(fp,"%lld\n", num[i]);

        printf("\nTesting");

        free(num);
        fclose(fp);
}

static void is_sorted(void *arr, r2_uint64 start ,r2_uint64 size, r2_uint64 ez,r2_cmp cmp)
{
        char *seq = arr;
        for(r2_uint64 i = start; i < size -1; ++i)
                assert(cmp(&seq[i*ez], &seq[(i+1)*ez]) <= 0);
}

static r2_int16 int_cmp(const void *a, const void *b)
{
        const r2_int64 *c = a; 
        const r2_int64 *d = b; 
        if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else    return 1;
}

static r2_int16 char_cmp(const void *a, const void *b)
{
        const r2_c *c = a; 
        const r2_c *d = b; 

        if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else    return 1;
}
static r2_int16 double_cmp(const void *a, const void *b)
{
        const r2_dbl *c = a; 
        const r2_dbl *d = b; 

        if(*c == *d)
                return 0; 
        else if(*c < *d)
                return -1; 
        else    return 1;
}
void r2_sort_test_run()
{
        test_insertion_sort();
        test_selection_sort();
        test_r2_bubble_sort();
        test_r2_shell_sort();
        test_r2_merge_sort();
        test_r2_merge_sort_mod();
        test_bmerge_sort();
        test_bmerge_sort_mod();
        test_r2_quick_sort();
        test_r2_sort_stats();
}

static void print_ints(r2_int64 *arr, r2_uint64 size)
{
        for(r2_uint64 i = 0; i < size; ++i)
                printf(" %lld", arr[i]);
}

static void print_double(r2_dbl *arr, r2_uint64 size)
{
        for(r2_uint64 i = 0; i < size; ++i)
                printf(" %.2lf", arr[i]);
}

static void print_char(r2_c *arr, r2_uint64 size)
{
        for(r2_uint64 i = 0; i < size; ++i)
                printf(" %c", arr[i]);
}