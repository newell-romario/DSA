#include "r2_unionfind.h"
#include <stdlib.h>
#include <assert.h>

static struct r2_set *r2_gset(struct r2_universe *, r2_uc *, r2_uint64);
/**
 * @brief                       Creates an universe that holds multiple sets.
 * 
 * @param cmp                   A callback comparison function used to find sets in the universe.
 * @param fk                    A callback function used to free elements in the set when we're are destroying the universe.
 * @return struct r2_universe*  Returns a universe, else NULL.
 */
struct r2_universe* r2_create_universe(r2_cmp cmp, r2_fk fk)
{
        struct r2_universe *u = NULL; 
        struct r2_robintable *sets = r2_create_robintable(1, 1, 0, 0, .55, cmp, NULL, NULL, NULL, fk, NULL);
        if(sets != NULL){
                u = malloc(sizeof(struct r2_universe));
                if(u != NULL){
                        u->nsets = 0; 
                        u->sets = sets;
                }else   r2_destroy_robintable(sets);
        }

        return u;
}

/**
 * @brief                       Destroys the universe. 
 * 
 * @param u                     Universe.
 * @return struct r2_universe* Returns NULL whenever universe is properly destroyed.
 */
struct r2_universe* r2_destroy_universe(struct r2_universe *u)
{
        assert(r2_destroy_robintable(u->sets) == NULL); 
        free(u); 
        return NULL;
}


/**
 * @brief              Makes a set with key being the leader of set.         
 * 
 * @param u            Universe.
 * @param sn           Set name.
 * @param len          Length.
 * @return r2_uint16   Returns TRUE upon successful set creation, else FALSE. 
 */
r2_uint16 r2_makeset(struct r2_universe *u, r2_uc *sn, r2_uint64 len)
{
        r2_uint16 SUCCESS =  TRUE;
        struct r2_entry e = {.key = NULL, .data = NULL, .length = 0}; 
        r2_robintable_get(u->sets, sn, len, &e); 
        if(e.key == NULL){
                struct r2_set *s = malloc(sizeof(struct r2_set));
                if(s != NULL){
                        s->parent = s;
                        s->sn     = sn; 
                        s->tsize  = 1;
                        r2_robintable_put(u->sets, sn, s, len);
                        if(r2_robintable_put(u->sets, sn, s, len) != TRUE){
                                SUCCESS = FALSE;
                                free(s);
                        }else
                                ++u->nsets; 
                }else SUCCESS = FALSE;
        }

        return SUCCESS;
}

/**
 * @brief              Returns set containing key.
 * 
 * @param u            Universe.
 * @param sn           Set name.
 * @param len          Length. 
 * @return r2_uc*      Returns key the set is a part of, else NULL if the set isn't a part of the universe.
 */
r2_uc* r2_findset(struct r2_universe *u, r2_uc *sn, r2_uint64 len)
{
        r2_uc *k = NULL;
        struct r2_entry e; 
        struct r2_set *s = NULL;
        struct r2_set *p = NULL; 
        struct r2_set *r = NULL;
        r2_robintable_get(u->sets, sn, len, &e);
        if(e.key != NULL){
                s = e.data;
                p = s->parent;
                /**
                 * @brief Do path compresion for quicker finds the in the future.
                 * 
                 */
                while(s->parent != s){
                        s = s->parent;
                        p = s; 
                }

                s = e.data; 
                while(s->parent != s){
                        r = s; 
                        s = s->parent;
                        r->parent = p;
                }
               k = p->sn;
        }
        return k;
}

/**
 * @brief               Unions two sets.
 * 
 * @param u             Universe.
 * @param s             Set. 
 * @param slen          Length.
 * @param t             Set. 
 * @param tlen          Length.
 * @return r2_uint16    Returns TRUE upon successfully union, else FALSE.
 */
r2_uint16 r2_unionset(struct r2_universe *u, r2_uc *s, r2_uint64 slen, r2_uc *t, r2_uint64 tlen)
{
        r2_uint16 SUCCESS = FALSE; 
        struct r2_set *cluster[2] = {r2_gset(u, s,slen), r2_gset(u, t, tlen)}; 
        if(cluster[0]  == cluster[1])
                goto CLEANUP;
        

        if(cluster[0]->tsize < cluster[1]->tsize){
                cluster[0]->parent = cluster[1];
                cluster[1]->tsize  =  cluster[0]->tsize + cluster[1]->tsize;
        }      
        else{
                cluster[1]->parent = cluster[0];
                cluster[0]->tsize  =  cluster[1]->tsize + cluster[0]->tsize;
        }
                
        --u->nsets;
        SUCCESS = TRUE;
        CLEANUP:
                return SUCCESS;
}
/**
 * @brief               Determines if two sets are subset of the same set.
 * 
 * @param u             Universe.
 * @param s             Set. 
 * @param slen          Length.
 * @param t             Set. 
 * @param tlen          Length.
 * @return r2_uint16    Returns TRUE upon successfully union, else FALSE.
 */
r2_uint16 r2_sameset(struct r2_universe *u, r2_uc *s, r2_uint64 slen, r2_uc *t, r2_uint64 tlen)
{
        return r2_findset(u, s, slen) == r2_findset(u, t, tlen);
}

/**
 * @brief              Returns set containing key.
 * 
 * @param u            Universe.
 * @param key          Set.
 * @param len          Length. 
 * @return r2_uc*      Returns the set containing key, else NULL if the set isn't a part of the universe.
 */
static struct r2_set *r2_gset(struct r2_universe *u, r2_uc *sn, r2_uint64 len)
{
        r2_uc *k = NULL;
        struct r2_entry e; 
        struct r2_set *s = NULL;
        struct r2_set *p = NULL; 
        struct r2_set *r = NULL;
        r2_robintable_get(u->sets, sn, len, &e);
        if(e.key != NULL){
                s = e.data;
                p = s->parent;
                /**
                 * @brief Do path compresion for quicker finds the in the future.
                 * 
                 */
                while(s->parent != s){
                        s = s->parent;
                        p = s; 
                }

                s = e.data; 
                while(s->parent != s){
                        r = s; 
                        s = s->parent;
                        r->parent = p;
                }
        }
        return p;
}