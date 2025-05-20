#ifndef R2_STRING_H_
#define R2_STRING_H_
#include "r2_types.h"
r2_int64 r2_naive_substring(const r2_c*, const r2_c*);
r2_int64 r2_rabin_karp(const r2_c*, const r2_c*);
r2_int64 r2_naive_dfa(const r2_c *, const r2_c *); 
r2_int64 r2_kmp(const r2_c *, const r2_c *);
r2_int64 r2_bmh(const r2_c*, const r2_c*);
#endif