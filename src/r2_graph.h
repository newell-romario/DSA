#ifndef R2_GRAPH_H_
#define R2_GRAPH_H_
#include "r2_types.h"
#include "r2_hash.h"
#include "r2_list.h"
#include "r2_heap.h"
typedef r2_ldbl(*r2_relax)(r2_ldbl, r2_ldbl);

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
        struct r2_robintable *eat;/*edge attributes*/
        r2_uint16 nat;/*mirrors nat attribute in graph*/ 
}; 
typedef r2_dbl(*r2_weight)(struct r2_edge *);

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
        r2_uint16 nat;/*mirors nat attribute in graph*/ 
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
     r2_uint16 nat; /*do not delete attribute for edge or vertex in subgraph whenever TRUE*/
};

struct r2_graph* r2_create_graph(r2_cmp, r2_cmp, r2_fk, r2_fk, r2_fk);
struct r2_graph* r2_destroy_graph(struct r2_graph *); 
r2_uint16 r2_graph_add_vertex(struct r2_graph *, r2_uc *, r2_uint64);
r2_uint16 r2_graph_del_vertex(struct r2_graph *, r2_uc*, r2_uint64);
struct r2_vertex* r2_graph_get_vertex(struct r2_graph*, r2_uc*, r2_uint64);
r2_uint16 r2_graph_add_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64);
r2_uint16 r2_graph_del_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64);
struct r2_edge*  r2_graph_get_edge(struct r2_graph *, r2_uc*, r2_uint64,  r2_uc*, r2_uint64);
r2_uint16 r2_graph_add_attributes(struct r2_graph *, r2_uc *, void *, r2_uint64); 
r2_uint16 r2_graph_del_attributes(struct r2_graph *, r2_uc *, r2_uint64);
void* r2_graph_get_attributes(struct r2_graph *, r2_uc *, r2_uint64);
r2_uint16 r2_vertex_add_attributes(struct r2_vertex *, r2_uc *, void *, r2_uint64, r2_cmp); 
r2_uint16 r2_vertex_del_attributes(struct r2_vertex *, r2_uc *, r2_uint64, r2_cmp);
void*  r2_vertex_get_attributes(struct r2_vertex *, r2_uc *, r2_uint64, r2_cmp);
r2_uint16   r2_edge_add_attributes(struct r2_edge*, r2_uc *, void *, r2_uint64, r2_cmp);
r2_uint16   r2_edge_del_attributes(struct r2_edge*, r2_uc *, r2_uint64, r2_cmp);
void*  r2_edge_get_attributes(struct r2_edge*, r2_uc *, r2_uint64, r2_cmp);

/**************************************************Graph Algorithms*********************************************/
struct r2_forest{
        r2_uint64 ncount;/*number of components*/
        struct r2_graph **tree;/*different components*/
};

void r2_graph_bfs(struct r2_graph *, struct r2_vertex *, r2_act, void *);
void r2_graph_dfs(struct r2_graph *, struct r2_vertex *, r2_act, void *);
struct r2_graph* r2_graph_transpose(struct r2_graph *);
struct r2_graph* r2_graph_path_tree(struct r2_graph *,  struct r2_vertex *, struct r2_vertex *);
struct r2_list* r2_graph_bipartite_set(struct r2_graph *, r2_uint16);
struct r2_list* r2_graph_dfs_traversals(struct r2_graph *, struct r2_vertex *, r2_uint16);
struct r2_list* r2_graph_topological_sort(struct r2_graph *);
struct r2_list* r2_graph_topological_sort_edges(struct r2_graph *);
struct r2_list* r2_graph_get_paths(struct r2_graph *, struct r2_vertex *, struct r2_vertex *); 
struct r2_list* r2_graph_get_paths_edges(struct r2_graph *, struct r2_vertex *, struct r2_vertex *);
struct r2_list* r2_graph_path_get_edges(struct r2_graph *, struct r2_list *);
r2_uint16 r2_graph_has_path(struct r2_graph *, struct r2_vertex *, struct r2_vertex *); 
r2_uint16 r2_graph_has_cycle(struct r2_graph *);
r2_uint16 r2_graph_strongly_connected(struct r2_graph *);
r2_uint16 r2_graph_is_bipartite(struct r2_graph *);
r2_uint16 r2_graph_is_connected(struct r2_graph *);
r2_uint16 r2_graph_is_biconnected(struct r2_graph *);
struct r2_graph* r2_graph_bfs_tree(struct r2_graph *, struct r2_vertex *); 
struct r2_graph* r2_graph_dfs_tree(struct r2_graph *, struct r2_vertex *);
struct r2_list*  r2_graph_children(struct r2_graph *, struct r2_vertex *); 
struct r2_vertex* r2_graph_parent(struct r2_graph *, struct r2_vertex *);
struct r2_forest* r2_graph_cc(struct r2_graph *);
struct r2_forest* r2_graph_destroy_cc(struct r2_forest *);
struct r2_forest* r2_graph_tscc(struct r2_graph *);
struct r2_forest* r2_graph_kcc(struct r2_graph *);
struct r2_forest* r2_graph_bcc(struct r2_graph *);
struct r2_list* r2_graph_articulation_points(struct r2_graph *);
struct r2_list* r2_graph_bridges(struct r2_graph *);
struct r2_graph* r2_graph_dijkstra(struct r2_graph *, r2_uc *, r2_uint64,  r2_weight);
struct r2_graph* r2_graph_bellman_ford(struct r2_graph *, r2_uc *, r2_uint64, r2_weight);
struct r2_graph* r2_graph_shortest_dag(struct r2_graph *, r2_uc *,  r2_uint64, r2_weight);
r2_dbl r2_graph_dist_from_source(struct r2_graph *, r2_uc *, r2_uint64);
/*************************************************Graph Algorithms*******************************************/
#endif 