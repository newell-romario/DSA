#ifndef R2_SORT_H_
#define R2_SORT_H_ 
#include "r2_types.h"
/**
 * @brief In this section we implement the most common sorting algorithms with some optimization if any. 
 * Sorting is a fundamental problem in computer science and is usually the first step in many algorithms. 
 * Before moving any further we need to define what sorting is? Sorting refers to re-arranging a sequence into 
 * either a monotonically non-drecreasing sequence or monotonically decreasing sequence. We can sort a sequence
 * using various sorting algorithms such as insertion sort, selection sort, bubble sort, shell sort, 
 * merge sort, and quick sort. Each sorting algorithm performs differently against specific inputs and each 
 * algorithm has certain properties which are viewed as important. Some sorting algorithms are in-place
 * while some aren't and some are stable while some aren't. An in-place sorting algorithm uses a constant amount of 
 * extra memory. A stable sorting algorithm maintains the order of elements in the sequence when both elements are
 * equal. Example if x and y are equal and x appears before y in the unsorted input then in the sorted output x must appear 
 * before y for the algorithm to be stable. Below we will examine the mentioned sorting algorithms along with certain 
 * properties.
 * 
 * 
 * 
 */
void r2_insertion_sort(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_selection_sort(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_bubble_sort(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_shell_sort(void *, r2_uint64, r2_uint64, r2_cmp);
void r2_merge_sort(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_merge_sort_mod(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_bmerge_sort(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
void r2_bmerge_sort_mod(void *, r2_uint64, r2_uint64, r2_uint64, r2_cmp);
#endif