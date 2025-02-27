#ifndef R2_UNIONFIND_H_
#define R2_UNIONFIND_H_
#include "r2_types.h"
#include "r2_hash.h"

/**
 * @brief Represents a set in our universe. 
 * 
 */
struct r2_set{
        /**
         * Parent of our set. To implement our set we use the disjoint tree
         * based implementation. In the disjoint tree implementation
         * a set is either subtree (subset) of another tree or it's own tree. 
         * The parent represents whether we are either. Whenever parent 
         * is pointing to it's self then it's not subtree of any other tree.
         *  
         */
        struct r2_set *parent;
        r2_uc *sn; /*set name*/
        r2_uint64 tsize;/*number of nodes in our tree*/
};

/**
 * @brief Following mathematical convention a set is a part of an universe and a universe contains 
 * all sets.
 * 
 */
struct r2_universe{
        struct r2_robintable *sets;/*All sets in the universe*/
        r2_uint64 nsets;/*number of sets in the universe*/
};

struct r2_universe* r2_create_universe(r2_cmp, r2_fk);
struct r2_universe* r2_destroy_universe(struct r2_universe *);
r2_uint16 r2_makeset(struct r2_universe *, r2_uc *, r2_uint64);
r2_uc* r2_findset(struct r2_universe *, r2_uc *, r2_uint64); 
r2_uint16 r2_unionset(struct r2_universe *, r2_uc *, r2_uint64, r2_uc *, r2_uint64);
r2_uint16 r2_sameset(struct r2_universe *, r2_uc *, r2_uint64, r2_uc *, r2_uint64);
#endif