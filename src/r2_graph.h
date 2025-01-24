#ifndef R2_GRAPH_H_
#define R2_GRAPH_H_
#include "r2_types.h"
#include "r2_hash.h"
#include "r2_list.h"

struct r2_vertex;
/**
 * @brief  Represents an edge in an adjacency list. 
 * 
 */
struct r2_edge{    
        struct r2_vertex *src;/*origin vertex*/
        struct r2_vertex *dest;/*destination vertex*/
        /**
         * Keeps track of the positions in lists
         * [0] - edge pos in vertex elist 
         * [1] - edge pos in graph  elist
         * [2] - vertex out pos
         * [3] - vertex in pos
         */
        struct r2_listnode *pos[4];
        r2_ldbl  weight;/*weight*/
        struct r2_robintable *eat;/*edge attributes*/
}; 


/**
 * @brief Represents a vertex in a graph.
 * 
 */
struct r2_vertex{
        r2_uc *vkey;/*key*/
        r2_uint64 len;/*key length*/  
        struct r2_listnode *pos;/*keeps track of the position in list*/
        struct r2_list *in;/*in vertex*/
        struct r2_list *out;/*out degree*/    
        struct r2_list *elist;/*contains all the edges for this vertex*/
        struct r2_robintable *edges;/*edges*/
        struct r2_robintable *vat;/*vertex attributes*/  
        r2_uint64 nedges;/*number of edges*/     
};



/**
 * @brief Represents graph.
 * 
 */
struct r2_graph{
     r2_uint64 nvertices;/*number of vertices*/ 
     r2_uint64 nedges;/*number of edges*/  
     struct r2_robintable *gat;/*graph attributes*/
     struct r2_robintable *vertices;/*contains all vertices in the graph*/
     struct r2_list *vlist;/*contains all the vertices in the graph*/
     struct r2_list *elist;/*contains all the edges in the graph*/
     r2_cmp vcmp;/*comparison function used to compare vertices*/
     r2_cmp gcmp;/*comparison function used for graph attributes*/
     r2_fk  fv;/*callback function that releases the memory used by vkey*/
     r2_fk  fk;/*callback function that releases the memory used by a key in gat*/
     r2_fk  fd;/*callback function that releases the memory used by a value in data*/
};

struct r2_graph* r2_create_graph(r2_cmp, r2_cmp, r2_fk, r2_fk, r2_fk);
struct r2_graph* r2_destroy_graph(struct r2_graph *); 
void r2_graph_add_attributes(struct r2_graph *, r2_uc *, void *, r2_uint64); 
void* r2_graph_get_attributes(struct r2_graph *, r2_uc *, r2_uint64);
void r2_graph_del_attributes(struct r2_graph *, r2_uc *, r2_uint64);
struct r2_graph* r2_graph_add_vertex(struct r2_graph *, r2_uc *, r2_uint64);
struct r2_vertex* r2_graph_get_vertex(struct r2_graph*, r2_uc*, r2_uint64);
struct r2_graph* r2_graph_del_vertex(struct r2_graph *, r2_uc*, r2_uint64);
void   r2_vertex_add_attributes(struct r2_vertex *, r2_uc *, void *, r2_uint64, r2_cmp); 
void*  r2_vertex_get_attributes(struct r2_vertex *, r2_uc *, r2_uint64, r2_cmp);
void   r2_vertex_del_attributes(struct r2_vertex *, r2_uc *, r2_uint64, r2_cmp);
struct r2_graph* r2_graph_add_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64, r2_ldbl);
struct r2_edge*  r2_graph_get_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64);
struct r2_graph* r2_graph_del_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64);
void   r2_edge_add_attributes(struct r2_edge*, r2_uc *, void *, r2_uint64, r2_cmp); 
void*  r2_edge_get_attributes(struct r2_edge*, r2_uc *, r2_uint64, r2_cmp);
void   r2_edge_del_attributes(struct r2_edge*, r2_uc *, r2_uint64, r2_cmp);
/**************************************************Graph Algorithms******************************************************/
/**
 * In our graph implementation we would like to keep the tree resulting from a bfs
 * traversal. Our tree is really an array where parents and children live in the same array similar to
 * how a binary heap is built.
 * 
 * 
 */
struct r2_bfsnode{
        struct r2_vertex *vertex;
        r2_uint64 pos;/*position in the array*/
        r2_uint64 state;/*current state of vertex while processing*/
        r2_int64  parent;/*parent*/
        r2_ldbl   dist;/*distance from the source vertex*/
        /**
         * Marks the beginning and ending of the subarray that represents
         * our children. A node with both start and end set equal
         * means the node has no children. 
         */
        struct{ 
                r2_int64 start; 
                r2_int64 end;
        }children;
};

struct r2_bfstree{
        struct r2_bfsnode *tree;/*stores the tree*/
        struct r2_robintable *positions;/*notes the position of each vertex in the array*/
        r2_uint64 ncount;/*number of elements in the tree*/
};

struct r2_dfsnode{
        struct r2_vertex *vertex;
        r2_uint64 pos;/*position in the array*/
        r2_uint64 state;/*current state of vertex while processing*/
        r2_int64  parent;/*parent*/
        r2_ldbl   dist;/*distance from the source vertex*/
        r2_uint64 start;/*start time*/
        r2_uint64 end;/*finish time*/
};

struct r2_dfstree{
        struct r2_robintable *positions;/*notes the position of each vertex in the array*/
        struct r2_dfsnode    *tree;/*stores the tree*/
        r2_uint64 ncount;/*number of elemnts*/
};

struct r2_components{
        r2_uint64 ncount; /*number of components*/
        struct r2_dfstree **cc; /*connected components*/
        struct r2_graph *transpose;/*used for strongly connected components*/
};

struct r2_bipartite{
        struct r2_list *sets[2];
};

void r2_graph_bfs(struct r2_graph *, struct r2_vertex *, r2_act, void *);
void r2_graph_dfs(struct r2_graph *, struct r2_vertex *, r2_act, void *);
struct r2_bfstree* r2_graph_bfs_tree(struct r2_graph *, struct r2_vertex *); 
struct r2_dfstree* r2_graph_dfs_tree(struct r2_graph *, struct r2_vertex *);
struct r2_bfstree* r2_destroy_bfs_tree(struct r2_bfstree *); 
struct r2_dfstree* r2_destroy_dfs_tree(struct r2_dfstree *);
struct r2_list* r2_graph_has_path(struct r2_graph *, struct r2_vertex *, struct r2_vertex *); 
struct r2_list* r2_graph_bfs_has_path_tree(struct r2_bfstree *, struct r2_vertex *, struct r2_vertex *);
struct r2_list* r2_graph_dfs_has_path_tree(struct r2_dfstree *, struct r2_vertex *, struct r2_vertex *);
struct r2_list* r2_graph_dfs_tree_children(struct r2_dfstree *, struct r2_vertex *); 
struct r2_list* r2_graph_bfs_tree_children(struct r2_bfstree *, struct r2_vertex *);
struct r2_bfsnode* r2_graph_bfsnode_next(struct r2_bfstree *, struct r2_bfsnode *); 
struct r2_dfsnode* r2_graph_dfsnode_next(struct r2_dfstree *, struct r2_dfsnode *);
struct r2_bfsnode* r2_graph_bfsnode_prev(struct r2_bfstree *, struct r2_bfsnode *); 
struct r2_dfsnode* r2_graph_dfsnode_prev(struct r2_dfstree *, struct r2_dfsnode *);
struct r2_bfsnode* r2_graph_bfsnode_first(struct r2_bfstree *); 
struct r2_dfsnode* r2_graph_dfsnode_first(struct r2_dfstree *);
struct r2_components* r2_graph_connected_components(struct r2_graph *);
struct r2_components* r2_graph_destroy_components(struct r2_components *); 
struct r2_dfstree* r2_graph_is_connected(struct r2_components *, struct r2_vertex *, struct r2_vertex *);
struct r2_bipartite* r2_graph_is_bipartite(struct r2_graph *);
struct r2_bipartite* r2_graph_destroy_bipartite(struct r2_bipartite *);
r2_int16 r2_graph_has_cycle(struct r2_graph *);
struct r2_list *r2_graph_topological_sort(struct r2_graph *);
struct r2_components* r2_graph_tarjan_strongly_connected_components(struct r2_graph *);
struct r2_graph* r2_graph_transpose(struct r2_graph *);
struct r2_list* r2_graph_dfs_traversals(struct r2_graph *, r2_uint16);
struct r2_components* r2_graph_strongly_connected_components(struct r2_graph *);
r2_uint16 r2_graph_is_strong_connected(struct r2_graph *);
struct r2_dfstree* r2_graph_dijkstra(struct r2_graph *, r2_uc *, r2_uint64, r2_ldbl(*)(r2_ldbl, r2_ldbl));
/************************************Graph Algorithms*******************************************/
#endif 