#include "r2_graph.h"
#include "r2_arrstack.h"
#include "r2_queue.h"
#include "r2_list.h"
#include "r2_heap.h"
#include "r2_unionfind.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
/********************File scope functions************************/
static void r2_free_edge_data(void *);
static void r2_free_vertex_data(void *);
static void r2_free_vertex(struct r2_graph *, struct r2_vertex *);
static void r2_free_edge(struct r2_graph *,  struct r2_edge *);
static void r2_free_list(void *);
static r2_int16 r2_cmp_edge(const void *, const void *);
static struct r2_vertex* r2_create_vertex(r2_cmp, r2_uint16);
static struct r2_edge*   r2_create_edge(r2_uint16);
static struct r2_graph*  r2_graph_components(struct r2_graph *, struct r2_vertex *, struct r2_robintable *, r2_int64);
static void action(void *, void*);
static struct r2_graph* r2_graph_build_tscc(struct r2_graph *, struct r2_vertex *, struct r2_robintable *, r2_uint64 *);
static struct r2_graph* r2_graph_build_bcc(struct r2_graph *, struct r2_arrstack *, struct r2_edge *);
static r2_int16 vat_cmp(const void *, const void *);
static r2_int16 wcmp(const void *, const void *);
static struct r2_graph* r2_graph_build_spt(struct r2_graph *, struct r2_graph *, struct r2_vertex *, r2_weight);
static r2_uint16 r2_graph_detect_negative_cycle(struct r2_graph *, struct r2_robintable *, r2_weight);
/**
 * WHITE - We have not started to process the adjacency list of this vertex.
 * GREY  - We have started to process this vertex but haven't completed processing. 
 * BLACK - We have completed processing the vertex.
 * YELLOW - A vertex is already apart of a component
 */
const r2_uint16 WHITE = 0; 
const r2_uint16 GREY  = 1;
const r2_uint16 BLACK = 2; 
const r2_uint16 YELLOW = 3;


struct r2_dist{
        struct r2_vertex *vertex; 
        r2_dbl dist;        
};


/********************File scope functions************************/

/**
 * @brief                       Creates an empty graph. 
 *                             
 * 
 * @param  vcmp                 A comparison callback function used to compare vertices.
 * @param  gcmp                 A comparison callback function used to compare graph attributes.
 * @param  fv                   A callback function used to free memory used by a vertex key.
 * @param  fk                   A callback function used to free memory used by attribute key.
 * @param  fd                   A callback function used to free memory used by attribute data.
 * @return struct r2_graph*     Returns an empty graph, else NULL.
 */
struct r2_graph* r2_create_graph(r2_cmp vmcp, r2_cmp gcmp, r2_fk fv, r2_fk fk, r2_fk fd)
{
        struct r2_graph *graph = malloc(sizeof(struct r2_graph)); 
        if(graph != NULL){
                graph->nvertices = 0; 
                graph->nedges    = 0; 
                graph->gat       = r2_create_robintable(1, 1, 0, 0, 0, gcmp, NULL, NULL, NULL, fk, fd);
                graph->vertices  = r2_create_robintable(1, 1, 0, 0, .75, vmcp, NULL, NULL, NULL, fv, r2_free_vertex_data);
                graph->vlist     = r2_create_list(NULL, NULL, NULL);
                graph->elist     = r2_create_list(NULL, NULL, NULL);
                graph->vcmp      = vmcp;
                graph->gcmp      = gcmp;
                graph->fv        = fv; 
                graph->fk        = fk;
                graph->fd        = fd;
                graph->nat       = FALSE;
                /**
                 * All metadata related to the graph is important. 
                 * If we can't allocate necessary structures for the metadata,
                 * we consider graph construction a failure and release memory acquired so
                 * far and return.
                 */
                if(graph->vertices == NULL  || graph->elist == NULL || graph->gat == NULL || graph->vlist == NULL){
                        if(graph->vertices != NULL)
                                r2_destroy_robintable(graph->vertices);
                       
                        if(graph->elist != NULL)
                                r2_destroy_list(graph->elist); 

                        if(graph->vlist != NULL)
                                r2_destroy_list(graph->vlist);
                        if(graph->gat != NULL)
                                r2_destroy_robintable(graph->gat);
                        free(graph); 
                        graph = NULL; 
                }
        }
        return graph;
}

/**
 * @brief                       Destroys a graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_graph*     Returns NULL whenever successfully destroyed.
 */
struct r2_graph* r2_destroy_graph(struct r2_graph *graph)
{
        if(graph->nat == FALSE)
                r2_destroy_robintable(graph->gat); 
        struct r2_listnode *head = r2_listnode_first(graph->elist);
        struct r2_edge *edge = NULL;
        struct r2_vertex *vertex = NULL;
        while(head != NULL){
                edge = head->data; 
                if(edge->nat == FALSE && edge->eat != NULL)
                        r2_destroy_robintable(edge->eat);
                free(edge);
                head = head->next;
        }

        head = r2_listnode_first(graph->vlist); 
        while(head != NULL){
                vertex = head->data; 
                
                vertex->edges->fd = NULL;
                if(vertex->vat != NULL && vertex->nat == FALSE){
                        r2_fd fd = vertex->vat->fd;
                        vertex->vat->fd = free;
                        r2_vertex_del_attributes(vertex, "0xdfs", 5, vat_cmp);
                        vertex->vat->fd = fd;
                        r2_destroy_robintable(vertex->vat); 
                }
                r2_destroy_robintable(vertex->edges); 
                r2_destroy_list(vertex->elist); 
                r2_destroy_list(vertex->out);
                r2_destroy_list(vertex->in);
                free(vertex);
                head = head->next;
        }

        r2_destroy_list(graph->elist); 
        r2_destroy_list(graph->vlist);
        graph->vertices->fd = NULL;
        r2_destroy_robintable(graph->vertices);
        free(graph);
        return NULL; 
}

/**
 * @brief                       Adds vertex to graph.
 *                              If vertex already exists we safely ignore it.
 * 
 * @param graph                 Graph.
 * @param vk                    Vertex key.
 * @param len                   Key length.
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE.
 */
r2_uint16 r2_graph_add_vertex(struct r2_graph *graph, r2_uc *vk, r2_uint64 len)
{
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, vk, len); 
        r2_uint16 SUCCESS = FALSE;
        if(vertex == NULL){
                vertex = r2_create_vertex(graph->vcmp, graph->nat);
                if(vertex != NULL){
                        vertex->vkey  = vk; 
                        vertex->len   = len;
                        if(r2_robintable_put(graph->vertices, vk, vertex, len) != TRUE ||
                        r2_list_insert_at_back(graph->vlist, vertex) != TRUE){
                                r2_free_vertex(graph, vertex);
                                return SUCCESS;
                        } 
                        ++graph->nvertices;
                        vertex->pos = r2_listnode_last(graph->vlist);
                        SUCCESS = TRUE;
                }         
        }else SUCCESS = TRUE;

        return SUCCESS;
}


/**
 * @brief                       Finds a vertex in the graph.
 * 
 * @param graph                 Graph.
 * @param vk                    Vertex key.
 * @param len                   Key length.
 * @return struct r2_vertex*    Returns a vertex, else NULL.
 */
struct r2_vertex* r2_graph_get_vertex(struct r2_graph *graph, r2_uc *vk, r2_uint64 len)
{
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_robintable_get(graph->vertices, vk, len, &entry); 
        return entry.data;
}   


/**
 * @brief                       Removes a vertex from graph.
 * 
 * @param graph                 Graph.
 * @param vkey                  Vertex key.
 * @param len                   Key length.
 * @return r2_uint16            Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_graph_del_vertex(struct r2_graph *graph, r2_uc *vkey, r2_uint64 len)
{
        r2_uint16 SUCCESS = FALSE;
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, vkey, len); 
        if(vertex != NULL){
                r2_free_vertex(graph, vertex);
                SUCCESS = TRUE;
        }

        return SUCCESS;
}

/**
 * @brief                       Adds an edge to graph. 
 *                              The endpoints of the edge doesn't have to exist. We create the endpoints
 *                              and subsequently the edge and add it to the graph.
 *                              
 * 
 * @param graph                 Graph.
 * @param src                   Origin.
 * @param slen                  Length.
 * @param dest                  Destination.
 * @param dlen                  Length.
 * @return r2_uint16            Returns TRUE upon successful insertion, else FALSE.
 */
r2_uint16 r2_graph_add_edge(struct r2_graph *graph, r2_uc *src, r2_uint64 slen,  r2_uc* dest, r2_uint64 dlen)
{
        r2_uint16 SUCCESS = FALSE;
        /*Stores src and destination keys and lengths*/
        r2_uc *keys[2] = {src, dest}; 
        r2_uint64 len[2] = {slen, dlen};

        /*Represents source and destination vertex*/
        struct r2_vertex *vertex[2] = {NULL, NULL};

        /*Represents whether source or destination vertex was just created*/ 
        struct r2_vertex *created_vertex[2]  = {NULL, NULL};

        /*Checking existence of edge*/
        struct r2_edge *edge = r2_graph_get_edge(graph, src, slen, dest, dlen); 
        if(edge == NULL){
                /*Creating edge*/
                edge = r2_create_edge(graph->nat);  
                if(edge != NULL){
                        /*Getting or adding vertices to graph*/
                        vertex[0] = r2_graph_get_vertex(graph, src,  slen); 
                        vertex[1] = r2_graph_get_vertex(graph, dest, dlen);
                        for(r2_uint64 i = 0; i < 2; ++i)
                                if(vertex[i] == NULL){
                                        if(r2_graph_add_vertex(graph, keys[i], len[i]) != TRUE)
                                                goto CLEANUP;
                                        vertex[i] = r2_graph_get_vertex(graph, keys[i], len[i]);
                                        created_vertex[i] = vertex[i];
                                }
                        if(r2_robintable_put(vertex[0]->edges, dest, edge, dlen) != TRUE)
                                goto CLEANUP;

                        /*Adding edge*/
                        edge->src  = vertex[0]; 
                        edge->dest = vertex[1];
                        ++vertex[0]->nedges;

                        /*Adding edge to vertex elist*/
                        if(r2_list_insert_at_back(vertex[0]->elist, edge) != TRUE)
                                goto CLEANUP; 
                        /*Adding edge to graph elist*/
                        if(r2_list_insert_at_back(graph->elist, edge) != TRUE)
                                goto CLEANUP; 
                        /*Adding dest to vertex out list*/
                        if(r2_list_insert_at_back(vertex[0]->out, vertex[1]) != TRUE)
                                goto CLEANUP; 
                        /*Adding src to dest in list*/   
                        if( r2_list_insert_at_back(vertex[1]->in, vertex[0]) != TRUE)
                                     goto CLEANUP;   
                        
                        ++graph->nedges;
                        edge->pos[0] = r2_listnode_last(vertex[0]->elist);
                        edge->pos[1] = r2_listnode_last(graph->elist);
                        edge->pos[2] = r2_listnode_last(vertex[0]->out);
                        edge->pos[3] = r2_listnode_last(vertex[1]->in);

                }else 
                        goto CLEANUP; 

                SUCCESS = TRUE;
                goto FINAL;
        }else{
                SUCCESS = TRUE; 
                goto FINAL;
        }

        CLEANUP: 
                r2_free_edge(graph, edge); 
                for(r2_uint64 i = 0; i < 2; ++i)
                        if(created_vertex[i] != NULL)
                                r2_free_vertex(graph, created_vertex[i]);
        FINAL:
                return SUCCESS;
}


/**
 * @brief                       Gets an edge from the graph.
 * 
 * @param graph                 Graph.
 * @param src                   Origin Key.
 * @param slen                  Origin Length.
 * @param dest                  Destination key.
 * @param dlen                  Destination length.
 * @return struct r2_edge*      Returns an edge, else NULL.
 */
struct r2_edge* r2_graph_get_edge(struct r2_graph *graph,  r2_uc *src, r2_uint64 slen,  r2_uc *dest, r2_uint64 dlen)
{
        struct r2_vertex *origin  = r2_graph_get_vertex(graph, src, slen); 
        struct r2_entry entry = {.data = NULL, .key = NULL, .length = 0}; 
        if(origin != NULL)
                r2_robintable_get(origin->edges, dest, dlen, &entry); 
        
        return entry.data;
}

/**
 * @brief                       Gets an edge from the graph.
 * 
 * @param graph                 Graph.
 * @param src                   Origin Key.
 * @param slen                  Origin Length.
 * @param dest                  Destination key.
 * @param dlen                  Destination Length.
 * @return r2_uint16            Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_graph_del_edge(struct r2_graph *graph,  r2_uc *src, r2_uint64 slen,  r2_uc *dest, r2_uint64 dlen)
{
        r2_uint16 SUCCESS = FALSE;
        struct r2_edge *edge  = r2_graph_get_edge(graph, src, slen, dest, dlen); 
        if(edge != NULL){
                r2_free_edge(graph, edge);
                SUCCESS = TRUE;
        }
        return SUCCESS;
}

/**
 * @brief               Adds attribute.
 * 
 * @param graph         Graph.
 * @param key           Key.
 * @param data          Data.
 * @param len           Length.
 * @return r2_uint16    Returns TRUE upon successful insertion, else FALSE.
 */
r2_uint16 r2_graph_add_attributes(struct r2_graph *graph, r2_uc *key, void *data, r2_uint64 len)
{
    return r2_robintable_put(graph->gat, key, data, len);
}

/**
 * @brief               Gets attribute.
 * 
 * @param graph         Graph.
 * @param key           Key.
 * @param len           Length.
 * @return void*        Returns data stored at key.
 */
void* r2_graph_get_attributes(struct r2_graph *graph, r2_uc *key, r2_uint64 len)
{
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_robintable_get(graph->gat, key, len, &entry); 
        return entry.data;
}


/**
 * @brief               Deletes attribute from graph.
 * 
 * @param graph         Graph.
 * @param key           Key.
 * @param len           Length.
 * @return r2_uint16    Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_graph_del_attributes(struct r2_graph *graph, r2_uc *key, r2_uint64 len)
{
     return r2_robintable_del(graph->gat, key, len);
}

/**
 * @brief                       Creates an empty vertex.
 * @param cmp                   Comparison callback
 * @param nat                   IF TRUE the vertex is created without the attribute hash table. 
 *  
 * @return struct r2_vertex*    Returns an empty vertex, else NULL.
 */
static struct r2_vertex* r2_create_vertex(r2_cmp cmp,  r2_uint16 nat)
{
        struct r2_vertex *vertex = malloc(sizeof(struct r2_vertex));
        if(vertex != NULL){
                vertex->vkey            = NULL; 
                vertex->len             = 0; 
                vertex->pos             = NULL; 
                vertex->in              = r2_create_list(NULL, NULL, NULL);  
                vertex->out             = r2_create_list(NULL, NULL, NULL); 
                vertex->elist           = r2_create_list(NULL, NULL, NULL); 
                vertex->edges           = r2_create_robintable(1, 1, 0, 0, .75, cmp, NULL, NULL, NULL, NULL, r2_free_edge_data);
                vertex->nedges          = 0;
                vertex->vat             = nat == TRUE? NULL : r2_create_robintable(1, 1, 0, 0, .75, NULL, NULL, NULL, NULL, NULL, NULL);
                vertex->nat             = nat;
                if(vertex->in == NULL || vertex->out == NULL || vertex->elist == NULL   || vertex->edges == NULL || (nat == FALSE && vertex->vat == NULL)){
                        /**
                         * All metadata related to the vertex is important. 
                         * If we fail to allocated memory for any metadata, then construction 
                         * is deemed as a failure and we free all memory acquired.
                         * 
                         */
                        if(vertex->in != NULL)
                                r2_destroy_list(vertex->in);

                        if(vertex->out != NULL) 
                                r2_destroy_list(vertex->out);

                        if(vertex->elist != NULL) 
                                r2_destroy_list(vertex->elist); 

                        if(vertex->edges != NULL)
                                r2_destroy_robintable(vertex->edges); 

                        if(nat == FALSE && vertex->vat != NULL)
                                r2_destroy_robintable(vertex->vat);   

                        free(vertex); 
                        vertex = NULL;         
                }
        } 
        return vertex; 
}

/**
 * @brief               Adds vertex attributes.
 * 
 * @param vertex        Vertex.
 * @param key           Key.
 * @param data          Data.
 * @param len           Length.
 * @param cmp           A comparison callback function used to compare keys.
 * @return r2_uint16    Returns TRUE upon successful insertion, else FALSE.
 */
r2_uint16 r2_vertex_add_attributes(struct r2_vertex *vertex, r2_uc *key, void *data, r2_uint64 len, r2_cmp cmp)
{
        vertex->vat->kcmp = cmp; 
        return r2_robintable_put(vertex->vat, key, data, len);
}

/**
 * @brief               Gets vertex attribute.
 * 
 * @param vertex        Vertex.
 * @param key           Key.
 * @param len           Length.
 * @param cmp           A comparison callback function.
 * @return void*        Returns data stored at key.
 */
void* r2_vertex_get_attributes(struct r2_vertex *vertex, r2_uc *key, r2_uint64 len, r2_cmp cmp)
{
        vertex->vat->kcmp = cmp;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_robintable_get(vertex->vat, key, len, &entry); 
        return entry.data;
}

/**
 * @brief               Deletes attribute.
 * 
 * @param vertex        Vertex.
 * @param key           Key.
 * @param len           Length.
 * @param cmp           A comparison callback function.
 * @return r2_uint16    Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_vertex_del_attributes(struct r2_vertex *vertex, r2_uc *key, r2_uint64 len, r2_cmp cmp)
{
        vertex->vat->kcmp = cmp;
        return r2_robintable_del(vertex->vat, key, len);
}

/**
 * @brief                       Creates an empty edge.
 * @param nat                   If TRUE edge is created without attribute hash table.
 * @return struct r2_edge*      Returns an empty edge, else NULL.
 */
static struct r2_edge*  r2_create_edge(r2_uint16 nat)
{
        struct r2_edge *edge = malloc(sizeof(struct r2_edge)); 
        if(edge != NULL){
                edge->src         = NULL; 
                edge->dest        = NULL; 
                edge->nat         = nat;
                for(r2_uint64 i = 0; i < 4; ++i)
                        edge->pos[i] = NULL; 
                edge->eat         = nat == TRUE? NULL : r2_create_robintable(1, 1, 0, 0, .75, NULL, NULL, NULL, NULL, NULL, NULL); 
                /**
                 * All metadata is important. Failure to allocate requisite memory is considered
                 * a failure to create edge.
                 */
                if(nat == FALSE && edge->eat == NULL){
                        free(edge); 
                        edge = NULL;
                }
        }
        return edge;
}

/**
 * @brief               Adds attribute to edge.
 * 
 * @param edge          Edge.
 * @param key           Key.
 * @param data          Data.
 * @param len           Length.
 * @param cmp           A comparison callback function.
 * @return r2_uint16    Returns TRUE upon successful insertion, else NULL.
 */
r2_uint16 r2_edge_add_attributes(struct r2_edge *edge, r2_uc *key, void *data, r2_uint64 len, r2_cmp cmp)
{
        edge->eat->kcmp = cmp;
        return r2_robintable_put(edge->eat, key, data, len); 
}

/**
 * @brief               Gets attribute.
 * 
 * @param edge          Edge.
 * @param key           Key.
 * @param len           Length.
 * @param cmp           A comparison callback function.
 * @return void*        Returns data stored at key.
 */
void* r2_edge_get_attributes(struct r2_edge *edge, r2_uc *key, r2_uint64 len, r2_cmp cmp)
{ 
        edge->eat->kcmp = cmp;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_robintable_get(edge->eat, key, len, &entry);
        return entry.data;
}

/**
 * @brief               Deletes attribute.
 * 
 * @param edge          Edge.
 * @param key           Key.
 * @param len           Length.
 * @param cmp           A comparison callback function.
 * @return r2_uint16    Returns TRUE upon successful deletion, else FALSE.
 */
r2_uint16 r2_edge_del_attributes(struct r2_edge *edge, r2_uc *key, r2_uint64 len, r2_cmp cmp)
{
        edge->eat->kcmp = cmp;
        r2_robintable_del(edge->eat, key, len);
}

/**
 * @brief               Helper function used when deleting a vertex.
 * 
 * @param graph         Graph.
 * @param vertex        Vertex.
 */
static void r2_free_vertex(struct r2_graph *graph, struct r2_vertex *vertex)
{
        /*Removing vertex from graph list of vertices*/
        if(vertex->pos != NULL){
                r2_list_delete(graph->vlist, vertex->pos);
                --graph->nvertices;
        }

        /*Freeing edges*/
        struct r2_listnode *node = r2_listnode_first(vertex->elist); 
        while(node != NULL){
                r2_free_edge(graph, node->data);
                node = r2_listnode_first(vertex->elist);
        }

        /*Need to check all vertices that are incident on this vertex*/
        struct r2_vertex *src    = NULL; 
        struct r2_edge *edge     = NULL;
        node = r2_listnode_first(vertex->in);
        while(node != NULL){
                src  = node->data;
                edge = r2_graph_get_edge(graph, src->vkey, src->len, vertex->vkey, vertex->len);
                r2_free_edge(graph, edge);
                node = r2_listnode_first(vertex->in);
        }
 
        r2_robintable_del(graph->vertices, vertex->vkey, vertex->len);
}

/**
 * @brief               Helper function used when freeing an edge.
 * 
 * @param graph         Graph.
 * @param edge          Edge.
 */
static void r2_free_edge(struct r2_graph *graph,  struct r2_edge *edge)
{
        struct r2_vertex *src  = edge->src; 
        struct r2_vertex *dest = edge->dest;

        if(edge->pos[0] != NULL)
                r2_list_delete(src->elist, edge->pos[0]);
        
        if(edge->pos[1] != NULL){
                r2_list_delete(graph->elist, edge->pos[1]); 
                --graph->nedges;
        }
                
        if(edge->pos[2] != NULL)
               r2_list_delete(src->out, edge->pos[2]);

        if(edge->pos[3] != NULL)
               r2_list_delete(dest->in, edge->pos[3]);
        
        if(src != NULL){
                r2_robintable_del(src->edges, dest->vkey, dest->len);
                --src->nedges;
        }
}



/********************************************Graph Algorithms*************************************************/
/**
 * @brief               Checks if a graph has a cycle.
 * 
 * @param graph         Graph.
 * @return r2_uint16    Returns TRUE if graph has cycle, else FALSE.
 */
r2_uint16 r2_graph_has_cycle(struct r2_graph *graph)
{
        r2_uint16 CYCLE = FALSE;
        /*Stores current edge being processed*/
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);

       /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL)
                goto CLEANUP; 
        
        struct r2_listnode *head   = NULL; 
        struct r2_listnode *vertex = NULL;
        struct r2_vertex   *source = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_edge     *edge   = NULL;
        struct r2_entry    entry   = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint16 *vstate = NULL;
        r2_int64 count = -1;
        
        vertex  = r2_listnode_first(graph->vlist);
        while(vertex != NULL){
                source = vertex->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate    = entry.data; 
                if(vstate == NULL){
                        vstate    = &state[++count];
                        *vstate   = GREY; 
                        if(r2_robintable_put(processed, source->vkey, vstate, source->len) != TRUE)
                                goto CLEANUP;
                
                        if(source->elist != NULL)
                                head  = r2_listnode_first(source->elist);
                        do{
                                while(head != NULL){
                                        edge = head->data;
                                        dest = edge->dest;
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        if(vstate == NULL){
                                                vstate    = &state[++count];
                                                *vstate   = GREY;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE ||
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE)
                                                        goto CLEANUP;

                                                source  = dest;
                                                head    = r2_listnode_first(source->elist);
                                                if(head == NULL)
                                                        break;

                                                continue;       
                                        }else if(*vstate == GREY){
                                                CYCLE = TRUE;
                                                goto CLEANUP;
                                        }
                                        head = head->next; 
                                }

                                
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                vstate    = entry.data; 
                                *vstate   = BLACK; 
                                head      = r2_arrstack_top(stack);
                                if(head != NULL){
                                        edge      = head->data; 
                                        source    = edge->src;
                                        head      = head->next;
                                }else
                                        source = NULL;
                                r2_arrstack_pop(stack);              
                        }while(source != NULL);
                }
                vertex = vertex->next;
        }
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);

       return CYCLE;           
}

/**
 * @brief                       Creates the transpose of graph.
 *                              Our transpose graph doesn't contain any graph, vertex, and edge attributes.                        
 * 
 * @param graph                 Graph.
 * @return struct r2_graph*     Returns the graph representing the tranpose, else NULL.
 */
struct r2_graph* r2_graph_transpose(struct r2_graph *graph)
{
        struct r2_graph *transpose = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd); 
        if(transpose != NULL){
                transpose->nat = TRUE;
                struct r2_vertex   *src    = NULL;
                struct r2_vertex   *dest   = NULL;
                struct r2_listnode *head   = r2_listnode_first(graph->elist);
                struct r2_edge     *edge   = NULL;
                struct r2_robintable *att[3] = {NULL};
                while(head != NULL){
                        edge  = head->data;
                        src   = edge->src; 
                        dest  = edge->dest;
                        if(r2_graph_add_edge(transpose, dest->vkey, dest->len, src->vkey, src->len) != TRUE){
                                transpose = r2_destroy_graph(transpose); 
                                break;
                        }
                        att[0] = src->vat; 
                        att[1] = dest->vat;
                        att[2] = edge->eat;
                        src  = r2_graph_get_vertex(transpose, src->vkey, src->len); 
                        dest = r2_graph_get_vertex(transpose, dest->vkey, dest->len);
                        edge = r2_graph_get_edge(transpose, dest->vkey, dest->len, src->vkey, src->len);
                        
                        src->vat  = att[0];
                        dest->vat = att[1];
                        edge->eat = att[2];
                        head  = head->next;
                }

                /*It's possible that the graph has no edges but vertices. 
                Handling this possibility.*/
                head = r2_listnode_first(graph->vlist);
                while(head != NULL){
                        src = head->data; 
                        att[0] = src->vat;
                        if(r2_graph_add_vertex(transpose, src->vkey, src->len) != TRUE){
                                transpose = r2_destroy_graph(transpose); 
                                break;
                        } 

                        src = r2_graph_get_vertex(transpose, src->vkey, src->len);
                        src->vat = att[0];
                        head = head->next;
                }
        }
        return transpose;
}

/**
 * @brief             Performs breadth first search on graph.
 * 
 * @param graph       Graph.
 * @param source      Source.
 * @param action      Action peformed on each vertex.
 * @param arg         Arguments passed to action.
 */
void r2_graph_bfs(struct r2_graph *graph, struct r2_vertex *source, r2_act action, void *arg)
{
        r2_int16 FAILED = FALSE;

        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = NULL;
        struct r2_vertex   *dest   = NULL; 
        struct r2_edge     *edge   = NULL;
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;   
        
        /*Initializing queue with source vertex*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        /*Updating source state in hash table*/
        state[count] = GREY; 
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        
        if(action != NULL)
                action(source, arg);

        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                head   = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is WHITE.
                         */
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                if(action != NULL)
                                        action(dest, arg);
                                
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                r2_queue_enqueue(queue, dest) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }

                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE && count != graph->nvertices);
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
        assert(FAILED == FALSE);
}

/**
 * @brief               Checks if a graph is strongly connected
 * 
 * @param graph         Graph.
 * @return r2_uint16    Returns TRUE if graph is strongly connected, else FALSE.
 */
r2_uint16 r2_graph_strongly_connected(struct r2_graph *graph)
{
        r2_uint16 CONNECTED = FALSE;
        r2_uint64 nvertices[2] = {0};
        struct r2_vertex *source    = r2_listnode_first(graph->vlist)->data;
        struct r2_graph  *transpose = r2_graph_transpose(graph);
        if(transpose != NULL){
                r2_graph_bfs(graph, source, action, &nvertices[0]); 
                source = r2_graph_get_vertex(transpose, source->vkey, source->len);
                r2_graph_bfs(transpose, source, action, &nvertices[1]);
                CONNECTED =  nvertices[0] == nvertices[1];
                r2_destroy_graph(transpose);
        }
        return CONNECTED;
}

/**
 * @brief               Performs depth first search on graph.
 * 
 * @param graph         Graph.
 * @param vertex        Source.
 * @param action        Action performed on each vertex in preorder.
 * @param arg           Argument to be passed to action.
 */
void r2_graph_dfs(struct r2_graph *graph, struct r2_vertex *source, r2_act action, void *arg)
{
        r2_int16 FAILED = FALSE;
        /*Stores current edge being processed*/
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
       /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count = -1;

        /*Updating source state in hash table*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        state[++count] = GREY;
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        if(action != NULL)
                action(source, arg);

        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);

        r2_uint16 *vstate = NULL;
        do{
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data; 
                        if(vstate == NULL){
                                /*Perform action*/
                                if(action != NULL)
                                        action(dest, arg);
                                
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE 
                                || r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                
                                source  = dest;
                                head    = r2_listnode_first(source->elist);
                                if(head == NULL)
                                        break;
                                continue;          
                        }
                        head = head->next; 
                }

                
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate    = entry.data;
                *vstate   = BLACK; 
                head      = r2_arrstack_top(stack);
                if(head != NULL){
                        edge      = head->data; 
                        source    = edge->src;
                        head      = head->next;
                }else source = NULL;
                r2_arrstack_pop(stack);
        }while(source != NULL);
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
        
        assert(FAILED == FALSE);       
}

/**
 * @brief                       Performs DFS on a graph and store the vertices in either preorder, 
 *                              postorder, or reverse postorder.
 *                              
 * @param graph                 Graph.
 * @param source                Source.
 * @param order                 Order. 0 == preorder,  1 == postorder, 2 == reverse postorder.
 * @return struct r2_list*      Returns the list vertices.
 */
struct r2_list* r2_graph_dfs_traversals(struct r2_graph *graph, struct r2_vertex *source, r2_uint16 order)
{
        r2_int16 FAILED = FALSE;
        r2_uint64 ENTIRE_GRAPH = source == NULL? TRUE : FALSE; /*Perform DFS on entire graph when true*/
        /*Stores current edge being processed*/
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        /*Stores vertices in specific order*/
        struct r2_list *list = r2_create_list(NULL, NULL, NULL);
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL || list == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
        
        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count = -1;
        
        struct r2_listnode *cur = r2_listnode_first(graph->vlist);
        r2_uint16 *vstate = NULL;
        while(cur != NULL){
                if(ENTIRE_GRAPH == TRUE)
                        source = cur->data;

                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        state[++count] = GREY;
                        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }

                        /*preorder*/
                        if(order == 0){
                                if(r2_list_insert_at_back(list, source) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                } 
                        }

                        if(source->elist != NULL)
                                head  = r2_listnode_first(source->elist);

                        do{
                                while(head != NULL){
                                        edge = head->data;
                                        dest = edge->dest;
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        if(vstate == NULL){
                                                /*preorder*/
                                                if(order == 0){
                                                        if(r2_list_insert_at_back(list, dest) != TRUE){
                                                                FAILED = TRUE; 
                                                                goto CLEANUP;
                                                        }     
                                                }

                                                vstate    = &state[++count];
                                                *vstate   = GREY; 
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;          
                                                }

                                                source = dest;
                                                head   = source->elist != NULL? r2_listnode_first(source->elist) : NULL;
                                                if(head == NULL)
                                                        break;

                                                continue;     
                                        }
                                        head = head->next; 
                                }

                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                vstate    = entry.data;
                                *vstate   = BLACK; 
                                head      = r2_arrstack_top(stack);
                                /*postorder*/
                                struct r2_listnode node = {.data = source}; 
                                if(order == 1){
                                        if(r2_list_insert_at_back(list, source) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                }      
                                /*reverse postorder*/
                                else if(order == 2){
                                        if(r2_list_insert_at_front(list, source) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;                                                        
                                        }
                                }

                                if(head != NULL){
                                        edge      = head->data; 
                                        source    = edge->src;
                                        head      = head->next;
                                }else source = NULL;
                                r2_arrstack_pop(stack);
                                   
                        }while(source!= NULL);
                }
                if(ENTIRE_GRAPH == FALSE)
                        break;
                cur = cur->next;
        }
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
        
        assert(FAILED == FALSE); 

        return list;                
}

/**
 * @brief                       Performs a topological sort on graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_list*      Returns the ordering of vertices, else NULL if the graph has a cycle.
 */
struct r2_list* r2_graph_topological_sort(struct r2_graph *graph)
{
        r2_uint16 FAILED = FALSE;
        /*stores the vertices in topological order*/
        struct r2_list  *top   = r2_create_list(NULL, NULL, NULL); 
        /*stores the vertices being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_robintable *indegree = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        r2_uint64 *count = malloc(sizeof(r2_uint64) * graph->nvertices);
        if(top == NULL || queue == NULL || indegree == NULL || count == NULL || graph->nvertices == 0){
                FAILED = TRUE;
                goto CLEANUP;
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_queuenode *node = NULL;
        struct r2_vertex *source  = NULL;
        struct r2_vertex *dest    = NULL;
        struct r2_edge    *edge   = NULL;
        struct r2_entry entry; 
        r2_uint64 i = 0; 
        r2_uint64 nvertices = 0;
        while(head != NULL){
                source   = head->data; 
                count[i] = source->in->lsize; 
                if(count[i] == 0){
                        if(r2_queue_enqueue(queue, source) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }else{
                        if(r2_robintable_put(indegree, source->vkey, &count[i], source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }
                i++;
                head  = head->next; 
        }

        r2_uint64 *in = NULL; 
        while(r2_queue_empty(queue) != TRUE){
                source = r2_queue_front(queue)->data;
                if(r2_list_insert_at_back(top, source) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                head  = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge   = head->data;
                        dest   = edge->dest; 
                        r2_robintable_get(indegree, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                in = entry.data; 
                                *in = *in -1; 
                                if(*in == 0){
                                        if(r2_queue_enqueue(queue, dest) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        if(r2_robintable_del(indegree, dest->vkey, dest->len) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }

                                }
                        }
                        head = head->next;
                }
                r2_queue_dequeue(queue);
        }

        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue);
                
                if(indegree != NULL){
                        if(indegree->nsize != 0)
                                top = r2_destroy_list(top);
                        r2_destroy_robintable(indegree);
                }
                        
                if(count != NULL)
                        free(count);
                
                if(FAILED == TRUE && top != NULL)
                        top = r2_destroy_list(top);

        return top;
}

/**
 * @brief                       Performs a topological sort on graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_list*      Returns the ordering of edges, else NULL if the graph has a cycle.
 */
struct r2_list* r2_graph_topological_sort_edges(struct r2_graph *graph)
{
        r2_uint16 FAILED = FALSE;
        /*stores the vertices in topological order*/
        struct r2_list  *top   = r2_create_list(NULL, NULL, NULL); 
        /*stores the vertices being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_robintable *indegree = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        r2_uint64 *count = malloc(sizeof(r2_uint64) * graph->nvertices);
        if(top == NULL || queue == NULL || indegree == NULL || count == NULL || graph->nvertices == 0){
                FAILED = TRUE;
                goto CLEANUP;
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_queuenode *node = NULL;
        struct r2_vertex *source  = NULL;
        struct r2_vertex *dest    = NULL;
        struct r2_edge *edge      = NULL; 
        struct r2_entry entry; 
        r2_uint64 i = 0; 
        r2_uint64 nvertices = 0;
        while(head != NULL){
                source = head->data; 
                count[i] = source->in->lsize; 
                if(count[i] == 0){
                        if(r2_queue_enqueue(queue, source) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }else{
                        if(r2_robintable_put(indegree, source->vkey, &count[i], source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }
                i++;
                head  = head->next; 
        }

        r2_uint64 *in = NULL; 
        while(r2_queue_empty(queue) != TRUE){
                source = r2_queue_front(queue)->data;
                head  = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest; 
                        r2_robintable_get(indegree, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                in = entry.data; 
                                *in = *in -1; 
                                if(*in == 0){
                                        if(r2_list_insert_at_back(top, edge) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        if(r2_queue_enqueue(queue, dest) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        if(r2_robintable_del(indegree, dest->vkey, dest->len) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                }
                        }
                        head = head->next;
                }
                r2_queue_dequeue(queue);
        }

        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue);
                
                if(indegree != NULL){
                        if(indegree->nsize != 0)
                                top = r2_destroy_list(top);
                        r2_destroy_robintable(indegree);
                }      
                if(count != NULL)
                        free(count);
                
                if(FAILED == TRUE && top != NULL)
                        top = r2_destroy_list(top);

        return top;        
}


/**
 * @brief                      Checks if a path exists.
 * 
 * @param graph                Graph.
 * @param src                  Source.
 * @param dest                 Destination.
 * @return r2_uint16           Returns TRUE if path exists, else FALSE.
 */
r2_uint16 r2_graph_has_path(struct r2_graph *graph, struct r2_vertex *src, struct r2_vertex *dest)
{
        r2_uint16 PATH  = FALSE;
        r2_int16 FAILED = FALSE;
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
                
        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *destination = NULL;
        struct r2_edge *edge = NULL;
        struct r2_vertex *source   = src;
        struct r2_entry  entry     = {.key = NULL, .data = NULL, .length  = 0};
        struct r2_key a = {.key = 0, .len = 0}; 
        struct r2_key b = {.key = dest->vkey, .len = dest->len};
        r2_uint64 count = 0;
        

        state[count] = GREY; 
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        
        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                head   = r2_listnode_first(source->elist);
                a.key = source->vkey; 
                a.len = source->len;
                if(graph->vcmp(&a, &b) == 0){
                        PATH = TRUE; 
                        break;
                }
                while(head != NULL){
                        edge = head->data;
                        destination = edge->dest; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is white.
                         */
                        r2_robintable_get(processed, destination->vkey, destination->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, destination->vkey, vstate, destination->len) != TRUE ||  
                                r2_queue_enqueue(queue, destination) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE && count != graph->nvertices);

        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);

        assert(FAILED == FALSE);
        return PATH;
}

/**
 * @brief                       Lists all the paths from the src to dest. 
 *                              This function returns a list of lists. Each element in the list is 
 *                              a list that represents a unique simple path from src to dest. 
 * @param graph                 Graph.
 * @param src                   Source.
 * @param dest                  Destination.
 * @return struct r2_list*      Returns a list of paths, else NULL.
 */
struct r2_list* r2_graph_get_paths(struct r2_graph *graph, struct r2_vertex *src, struct r2_vertex *dest)
{
        r2_uint16 FAILED        = FALSE;
        struct r2_list   *paths = r2_create_list(NULL, NULL, r2_free_list); 
        
        /*Stores the list of edges on the current path*/
        struct r2_vertex **path = malloc(sizeof(struct r2_vertex *) * graph->nvertices);
        /*Stores current edge being processed*/
        struct r2_arrstack *stack    = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *onpath = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 

        if(stack == NULL || path == NULL || onpath == NULL || paths == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        struct r2_vertex   *source       = src; 
        struct r2_vertex   *destination  = dest;
        struct r2_vertex   *current      = NULL;
        struct r2_edge     *edge         = NULL;
        struct r2_listnode *head         = NULL; 
        struct r2_entry    entry         = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count = -1;

        /*Adding source to path*/
        path[++count] = source;
        if(r2_robintable_put(onpath, source->vkey, &path[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        
        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);
        
        do{
                while(head != NULL){
                        edge  = head->data;
                        dest  = edge->dest;
                        r2_robintable_get(onpath, dest->vkey, dest->len, &entry);
                        current = entry.data; 
                        if(current == NULL && dest != destination){
                                path[++count] = dest;
                                if(r2_robintable_put(onpath, dest->vkey, &path[count], dest->len) != TRUE 
                                || r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                
                                source  = dest;
                                head    = source->elist != NULL? r2_listnode_first(source->elist) : NULL;
                                if(head == NULL)
                                        break;
                                continue;          
                        }else if(dest == destination){
                                path[count + 1] = dest;
                                struct r2_list *npath = r2_create_list(NULL, NULL, NULL); 
                                for(r2_uint16 i = 0; i <= count + 1; ++i)
                                        if(r2_list_insert_at_back(npath, path[i]) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }

                                if(r2_list_insert_at_back(paths,  npath) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }            
                        }
                        head = head->next; 
                }
                
                if(r2_robintable_del(onpath, source->vkey, source->len) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }
                head      = r2_arrstack_top(stack);
                if(head != NULL){
                        edge      = head->data; 
                        source    = edge->src;
                        head = head->next;
                        --count;
                }else source = NULL;
                r2_arrstack_pop(stack); 
        }while(source != NULL);
        CLEANUP:
                if(FAILED == TRUE && paths != NULL){
                        paths = r2_destroy_list(paths);
                }

                if(path != NULL)
                        free(path); 
                
                if(onpath != NULL)
                        r2_destroy_robintable(onpath);

                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
        return paths;
}

/**
 * @brief                       Returns the SET of edges. 
 *                              This function basically finds all the edges between src and dest. 
 *                              Returns a set with no duplicates.
 * @param graph                 Graph.
 * @param src                   Source.
 * @param dest                  Destination.
 * @return struct r2_list*      Returns a list of edges, else NULL.
 */
struct r2_list* r2_graph_get_paths_edges(struct r2_graph *graph, struct r2_vertex *src, struct r2_vertex *dest)
{
        r2_uint16 FAILED = FALSE;
        struct r2_list *paths = r2_graph_get_paths(graph, src, dest);
        struct r2_list *edges = r2_create_list(NULL, NULL, NULL);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, r2_cmp_edge, NULL, NULL, NULL, NULL, NULL); 
        if(paths == NULL || edges == NULL || processed == NULL || graph->nvertices == 0){
                FAILED  = TRUE;
                goto CLEANUP;
        }
        struct r2_listnode *path = r2_listnode_first(paths); 
        struct r2_listnode *prev = NULL;
        struct r2_listnode *cur  = NULL; 
        struct r2_edge *edge     = NULL;
        struct r2_entry e;
        while(path != NULL){
                cur = r2_listnode_first(path->data); 
                while(cur != NULL){
                        prev = cur->prev;
                        if(prev != NULL){
                                src  = prev->data;
                                dest = cur->data;
                                edge = r2_graph_get_edge(graph, src->vkey, src->len, dest->vkey, dest->len); 
                                r2_robintable_get(processed, (r2_uc *)edge, sizeof(struct r2_edge *), &e); 
                                if(e.key == NULL){
                                        if(r2_list_insert_at_back(edges, edge) != TRUE ||
                                        r2_robintable_put(processed, (r2_uc *)edge, edge ,sizeof(struct r2_edge *)) != TRUE){
                                                FAILED  = TRUE;
                                                goto CLEANUP;
                                        }
                                }
                        }
                        cur = cur->next;
                }
                path = path->next;
        }

        CLEANUP:
                if(FAILED == TRUE && edges != NULL)
                        r2_destroy_list(edges);
                
                if(paths != NULL)
                        r2_destroy_list(paths); 
                
                if(processed  != NULL)
                        r2_destroy_robintable(processed);

        return edges;
}

/**
 * @brief                       Returns the edges that make up a path.
 * 
 * @param graph                 Graph.
 * @param path                  Path.
 * @return struct r2_list*      Returns a list of the edges.
 */
struct r2_list* r2_graph_path_get_edges(struct r2_graph *graph, struct r2_list *path)
{
        struct r2_list *edges = r2_create_list(NULL, NULL, NULL); 
        struct r2_edge *edge = NULL; 
        struct r2_listnode *head = NULL;
        struct r2_listnode *prev = NULL;
        struct r2_vertex *src; 
        struct r2_vertex *dest;
        if(edges != NULL){
                if(path != NULL)
                        head = r2_listnode_first(path);
                while(head != NULL){
                     
                        prev = head->prev;
                        if(prev != NULL){
                                src  = prev->data;
                                dest = head->data;
                                edge = r2_graph_get_edge(graph, src->vkey, src->len, dest->vkey, dest->len); 
                                if(r2_list_insert_at_back(edges, edge) != TRUE){
                                        edges = r2_destroy_list(edges); 
                                        break;
                                }          
                        }
                        head = head->next;
                }
        }
        return edges;
}

/**
 * @brief                       Builds the path tree starting from source to destination.
 * 
 * @param graph                 Graph.
 * @param src                   Source.
 * @param dest                  Dest.
 * @return struct r2_graph*     Returns path tree, else NULL.
 */
struct r2_graph* r2_graph_path_tree(struct r2_graph *graph,  struct r2_vertex *src, struct r2_vertex *dest)
{
        r2_uint16 FAILED = FALSE; 
        struct r2_graph *path = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd); 
        struct r2_list  *edges = r2_graph_get_paths_edges(graph, src, dest);
        if(path == NULL || edges == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        path->nat = TRUE;
        struct r2_listnode *head = r2_listnode_first(edges); 
        struct r2_edge *edge     = NULL; 
        while (head != NULL){
                edge = head->data;
                src  = edge->src;
                dest = edge->dest;
                if(r2_graph_add_edge(path, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                        FAILED = TRUE;
                        goto CLEANUP;
                }
                head = head->next;
        }
        
        CLEANUP:
                if(edges != NULL)
                        r2_destroy_list(edges); 
                
                if(FAILED == TRUE && path != NULL)
                        path = r2_destroy_graph(path);
                

        return path; 
}

/**
 * @brief                       Performs BFS on graph.
 *                              The BFS tree is a really a subgraph of the original graph. Additonally, 
 *                              a tree is a rooted graph and because of this, it's more natural to 
 *                              represent the tree as a graph.  N.B Adding any attributes to this graph 
 *                              affects the original graph.      
 *      
 * @param graph                 Graph.
 * @param source                Source.
 * @return struct r2_graph*     Returns BFS tree, else NULL.
 */
struct r2_graph* r2_graph_bfs_tree(struct r2_graph *graph, struct r2_vertex *source)
{
        r2_int16 FAILED = FALSE;

        struct r2_graph *bfs   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || bfs == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        bfs->gat = graph->gat;
        bfs->nat = TRUE;
        struct r2_listnode *head   = NULL;
        struct r2_vertex   *src    = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_edge     *edge   = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        struct r2_robintable *att[3];
        r2_uint64 count = 0;   
        
        /*Initializing queue with source vertex*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        /**
         * @brief Insert source into graph. Source may not have any outgoing edges.
         * 
         */
        if(r2_graph_add_vertex(bfs, source->vkey, source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        src = r2_graph_get_vertex(bfs, source->vkey, source->len); 
        src->vat = source->vat;
        

        /*Updating source state in hash table*/
        state[count] = GREY; 
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                att[0]  = source->vat;
                head   = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is WHITE.
                         */
                        att[1] = dest->vat;
                        att[2] = edge->eat;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                r2_queue_enqueue(queue, dest) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }

                                if(r2_graph_add_edge(bfs, source->vkey, source->len, dest->vkey, dest->len)  != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                src  = r2_graph_get_vertex(bfs, source->vkey, source->len);
                                dest = r2_graph_get_vertex(bfs, dest->vkey, dest->len);
                                edge = r2_graph_get_edge(bfs, src->vkey, src->len, dest->vkey, dest->len);
                                src->vat  = att[0];
                                dest->vat = att[1];
                                edge->eat = att[2];
                        }        
                        head = head->next; 
                }

                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE && count != graph->nvertices);
        
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                if(bfs != NULL && FAILED == TRUE)
                        bfs = r2_destroy_graph(bfs);

        return bfs;       
}


/**
 * @brief                       Performs a DFS on graph. 
 *                              The DFS tree is a really a subgraph of the original graph. Additonally, 
 *                              a tree is a rooted graph and because of this choose it's more natural to 
 *                              represent the tree has a graph.  N.B Adding any attributes to this graph 
 *                              affects the original graph.       
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @return struct r2_dfstree*   Returns DFS tree, else NULL.
 */
struct r2_graph* r2_graph_dfs_tree(struct r2_graph *graph, struct r2_vertex *source)
{
        r2_int16 FAILED = FALSE;
        struct r2_graph *dfs = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);

        /*Stores current edge being processed*/
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL || dfs == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        dfs->nat = TRUE; 
        dfs->gat = graph->gat;
        struct r2_listnode *head  = NULL; 
        struct r2_listnode *src   = NULL;
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        struct r2_vertex *v[2];
        struct r2_robintable *att = NULL; 
        r2_int64 count = -1;

        /*Updating source state in hash table*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        att = source->vat; 
        state[++count] = GREY;
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /**
         * @brief Insert source into graph. Source may not have any outgoing edges.
         * 
         */
        if(r2_graph_add_vertex(dfs, source->vkey, source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        
        dest = r2_graph_get_vertex(dfs, source->vkey, source->len); 
        dest->vat = att;

        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);

        r2_uint16 *vstate = NULL;
        do{
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data; 
                        if(vstate == NULL){
                                att = edge->eat;
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE 
                                || r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }

                                if(r2_graph_add_edge(dfs, source->vkey, source->len, dest->vkey, dest->len) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                v[0] = r2_graph_get_vertex(dfs, source->vkey, source->len);
                                v[1] = r2_graph_get_vertex(dfs, dest->vkey, dest->len);
                                edge = r2_graph_get_edge(graph, source->vkey, source->len, dest->vkey, dest->len);
                                v[0]->vat = source->vat;
                                v[1]->vat = dest->vat;
                                edge->eat = att;
                                source  = dest;
                                head    = source->elist != NULL? r2_listnode_first(source->elist) : NULL;
                                if(head == NULL)
                                        break;
                                continue;          
                        }
                        head = head->next; 
                }

                if(head == NULL){
                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                        vstate    = entry.data;
                        *vstate   = BLACK; 
                        head      = r2_arrstack_top(stack);
                        if(head != NULL){
                                edge      = head->data; 
                                source    = edge->src;
                                head      = head->next;
                        }else source = NULL;
                        r2_arrstack_pop(stack);
                }    
        }while(source != NULL);
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                
                if(dfs != NULL && FAILED == TRUE)
                        dfs = r2_destroy_graph(dfs);
        assert(FAILED == FALSE);  
        return dfs;             
}

/**
 * @brief                       Returns the parent of source.
 *                              N.B Please only pass the DFS/BFS tree. Anyting else
 *                              results in inaccurate data.
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @return struct r2_vertex*    Returns vertex. NULL represents no parent exists which means source is the root of the tree.
 */
struct r2_vertex*  r2_graph_parent(struct r2_graph *graph, struct r2_vertex *source)
{
        struct r2_vertex *parent = NULL; 
        if(r2_listnode_first(source->in) != NULL)
                parent = r2_listnode_first(source->in)->data;
        
        return parent;
}

/**
 * @brief                       Returns the children of source.
 *                              N.B Please only pass the DFS/BFS tree. Anyting else
 *                              results in inaccurate data. Please do not call
 *                              r2_destroy_list on list. Calling this breaks the graph.
 * 
 * @param graph                 Graph
 * @param source                Source
 * @return struct r2_list*      Returns list of children, empty list means source is a leaf.
 */
struct r2_list*  r2_graph_children(struct r2_graph *graph, struct r2_vertex *source)
{
        return source->out;
}

/**
 * @brief                       Checks if a graph is bipartite.
 * 
 * @param graph                 Graph.
 * @return struct r2_graph*     Returns TRUE if graph has a bipartite matching, else FALSE.
 */
r2_uint16 r2_graph_is_bipartite(struct r2_graph *graph)
{
        r2_uint16 FAILED    = FALSE;
        r2_uint16 BIPARTITE = TRUE;
        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
         * @brief Using state and hash table to store the set the vertex is a part of.
         *  Uses GREY AND WHITE to represent sets.
         *  
         */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = r2_listnode_first(graph->vlist);
        struct r2_edge *edge       = NULL;
        struct r2_listnode *cur    = NULL;
        struct r2_vertex   *dest   = NULL; 
        struct r2_vertex   *source = NULL;
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count   = -1;   
        r2_uint16 *curset = NULL;
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                source  = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        state[++count] = WHITE; 
                        if(r2_queue_enqueue(queue, source) != TRUE || 
                        r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                                FAILED = TRUE;
                                goto CLEANUP;
                        }
                
                        do{
                                source = r2_queue_front(queue)->data;
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                curset = entry.data;
                                cur    = r2_listnode_first(source->elist);
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        /**
                                         * @brief Determining appropriate set.
                                         * 
                                         */
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data;
                                        if(vstate == NULL){
                                                vstate    = &state[++count];
                                                *vstate   = !(*curset); 
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_queue_enqueue(queue, dest) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                        }else if(*vstate == *curset){
                                                BIPARTITE = FALSE;
                                                goto CLEANUP;
                                        }        
                                        cur = cur->next; 
                                }
                                r2_queue_dequeue(queue);
                        }while(r2_queue_empty(queue) != TRUE);
                }
                head = head->next;
        }

        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                if(FAILED == TRUE)
                        BIPARTITE = FALSE;
           
        return BIPARTITE;
}

/**
 * @brief                       Returns all the vertices in a set 0 or 1. 
 * 
 * @param graph                 Graph. 
 * @param set                   Set.
 * @return struct r2_list*      Returns list containing vertices, else NULL.
 */
struct r2_list* r2_graph_bipartite_set(struct r2_graph *graph, r2_uint16 set)
{
        r2_uint16 FAILED      = FALSE;
        struct r2_list *group = NULL;
        r2_uint16 *state      = NULL;
        struct r2_queue*queue = NULL;
        struct r2_robintable *processed = NULL;
        if(r2_graph_is_bipartite(graph) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }
               
        group = r2_create_list(NULL, NULL, NULL);
        /*Holds vertices that are currently being processed*/
        queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
         * @brief Using state and hash table to store the set the vertex is a part of.
         *  Uses GREY AND WHITE to represent sets.
         *  
         */
        state = malloc(sizeof(r2_uint16) * graph->nvertices);
        processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || group == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = r2_listnode_first(graph->vlist);
        struct r2_edge     *edge   = NULL;
        struct r2_listnode *cur    = NULL;
        struct r2_vertex   *dest   = NULL; 
        struct r2_vertex   *source = NULL;
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count    = -1;   
        r2_uint16 *curset = NULL;
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                source  = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        state[++count] = WHITE; 
                        if(r2_queue_enqueue(queue, source) != TRUE || 
                        r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                                FAILED = TRUE;
                                goto CLEANUP;
                        }
                        if(state[count] == set)
                                if(r2_list_insert_at_back(group, source) != TRUE){
                                        FAILED = TRUE;
                                        goto CLEANUP;   
                                }
                        do{
                                source = r2_queue_front(queue)->data;
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                curset = entry.data;
                                cur    = r2_listnode_first(source->elist);
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        /**
                                         * @brief Determining appropriate set.
                                         * 
                                         */
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data;
                                        if(vstate == NULL){
                                                vstate    = &state[++count];
                                                *vstate   = !(*curset); 
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_queue_enqueue(queue, dest) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                                if(state[count] == set)
                                                        if(r2_list_insert_at_back(group, dest) != TRUE){
                                                                FAILED = TRUE;
                                                                goto CLEANUP;   
                                                        }
                                        }else if(*vstate == *curset){
                                                FAILED = TRUE;
                                                goto CLEANUP;
                                        }        
                                        cur = cur->next; 
                                }
                                r2_queue_dequeue(queue);
                        }while(r2_queue_empty(queue) != TRUE);
                }
                head = head->next;
        }
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);

                if(FAILED == TRUE && group != NULL)
                        group = r2_destroy_list(group);
    
        return group;
}

/**
 * @brief                               Finds the connected components in graph.
 *                                      
 *                                   
 * @param graph                         Graph.
 * @return struct r2_components*        Returns all the connected components in the graph, else NULL. 
 */
struct r2_forest* r2_graph_cc(struct r2_graph *graph)
{
        r2_int16 FAILED = FALSE;
        r2_uint64 count = 0;
        r2_int64 ID = -1;
        struct r2_graph *cc = NULL; /*component*/
        struct r2_graph **tree = NULL;
        struct r2_forest *forest = NULL;
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_int64 *state = malloc(sizeof(r2_int64) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list *components = r2_create_list(NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || components == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        } 

        /*Initialize state of all vertices*/
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        struct r2_vertex *source =  NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        while(head != NULL){
                source = head->data;
                state[count] = WHITE; 
                if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                        FAILED = TRUE;
                        goto CLEANUP;
                }
                head = head->next;
                ++count;
        }

        /*Processing graph*/
        head  = r2_listnode_first(graph->vlist);
        while(head != NULL){
                source = head->data; 
                r2_robintable_get(processed, source->vkey, source->len, &entry); 
                vstate = entry.data;
                if(*vstate == WHITE){
                        *vstate = (r2_uint64)source; 
                        cc = r2_graph_components(graph, source, processed, ID--);
                        if(cc == NULL){
                                FAILED = TRUE;
                                goto CLEANUP;

                        }

                        if(r2_list_insert_at_back(components, cc) != TRUE){
                                r2_destroy_graph(cc); 
                                FAILED = TRUE;
                                goto CLEANUP;
   
                        }
                        ++count;
                }
                head = head->next;
        }
        
        forest = malloc(sizeof(struct r2_forest));
        if(forest == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        tree = malloc(sizeof(struct r2_graph *) * components->lsize);
        if(tree != NULL){
                head = r2_listnode_first(components);
                count = 0;
                while(head != NULL){
                        tree[count++] = head->data;
                        head = head->next;
                }

                forest->tree = tree; 
                forest->ncount = count;
        }else FAILED = TRUE;

        CLEANUP:
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                
                if(FAILED == TRUE && components != NULL){   
                        /*Cleaning up components found so far*/
                        head = r2_listnode_first(components);
                        while(head != NULL){
                                r2_destroy_graph(head->data);
                                head = head->next;
                        }   
                }
                if(components != NULL)
                        r2_destroy_list(components);   

                if(FAILED == TRUE && forest != NULL)
                      forest =  r2_graph_destroy_cc(forest);

        return forest;
}

/**
 * @brief                               Helper function for connected components.
 * 
 * @param graph                         Graph.
 * @param source                        Source.
 * @param processed                     Hash table containing state of each vertices in the graph.
 * @param id                            Unique identifier for each component
 * @return struct r2_graph*             Returns BFS tree, else NULL.
 */
static struct r2_graph* r2_graph_components(struct r2_graph *graph, struct r2_vertex *source,  struct r2_robintable *processed, r2_int64 id)
{
        r2_int16 FAILED = FALSE;
        struct r2_graph *bfs   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        if(queue == NULL || bfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        bfs->gat = graph->gat;
        bfs->nat = TRUE;
        struct r2_listnode *head   = NULL;
        struct r2_vertex   *src    = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_edge     *edge   = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        struct r2_robintable *att[3];
        
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        
        /**
         * @brief Insert source into graph. Source may not have any outgoing edges.
         * 
         */
        if(r2_graph_add_vertex(bfs, source->vkey, source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        src = r2_graph_get_vertex(bfs, source->vkey, source->len); 
        src->vat = source->vat;

        r2_int64 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                att[0] = source->vat;
                head   = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data; 
                        dest = edge->dest; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is WHITE.
                         */
                        att[1] = dest->vat;
                        att[2] = edge->eat;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data;
                        if(*vstate == WHITE){
                                *vstate = id; 
                                if(r2_queue_enqueue(queue, dest) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }
                        
                        if(*vstate == id){
                                if(r2_graph_add_edge(bfs, source->vkey, source->len, dest->vkey, dest->len)  != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                src  = r2_graph_get_vertex(bfs, source->vkey, source->len);
                                dest = r2_graph_get_vertex(bfs, dest->vkey, dest->len);
                                edge = r2_graph_get_edge(bfs, src->vkey, src->len, dest->vkey, dest->len);
                                src->vat  = att[0];
                                dest->vat = att[1];
                                edge->eat = att[2];
                        }        
                        head = head->next; 
                }
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;
                *vstate = id;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE);
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(bfs != NULL && FAILED == TRUE)
                        bfs = r2_destroy_graph(bfs);
        return bfs;       
}


/**
 * @brief                       Destroys forest.
 * 
 * @param forest                Forest.
 * @return struct r2_forest*    Returns NULL whenever forest is destroy properly.
 */
struct r2_forest* r2_graph_destroy_cc(struct r2_forest *forest)
{
        for(r2_uint64 i = 0; i < forest->ncount; ++i)
                assert(r2_destroy_graph(forest->tree[i]) == NULL);
        
        if(forest->tree != NULL)
                free(forest->tree);
        free(forest); 
        return NULL; 
}

/**
 * @brief               Checks if an undirected graph is connected.
 *                      IF a directed graph is passed to it, it will 
 *                      return a FALSE positive. For directed graph use the
 *                      function is strongly connected.
 * 
 * @param graph
 * @return r2_uint16    Returns TRUE if connected, else FALSE.
 */
r2_uint16 r2_graph_is_connected(struct r2_graph *graph)
{
        r2_uint64 count = 0; 
        r2_graph_bfs(graph, NULL, action, &count);
        return graph->nvertices == count;
}


/**
 * Strongly Connected Components
 *
 * Finding the strongly connected components of a graph is fundamental graph operation. A strongly 
 * connected components in it's simplest form is a subset of a graph where every vertex in
 * that subset is reachable by every other vertex in the subset. Otherwise stated if we picked any two random vertices v, w
 * in a strongly connected component C there must exist a path between v and w and w and v. Finding strongly connected 
 * components means we have some redundancy in the graph i.e if we deleted an egde from the path v to w, we would still be connected.
 * 
 * Finding the strongly connected component can be acheived by using 3 three algoritms mainly the S. R. Kosaraju, Tarjan's algorithm
 * and path-based strong component algorithm. 
 * 
 * All these algorithms rely on depth first search for finding the strongly connected components. We'll most be looking at 
 * S. R. Kosaraju and Tarjan algorithm. 
 * 
 * _____________________________________________________________
 * Tarjan Algorithm
 * _____________________________________________________________
 * Tarjan strongly connected component algorithm works by identifying the leader of a strong component and
 * then by identifying the followers. When a depth first search is peformed on a graph it produces critical information
 * about each vertex that is a part of the search. Each vertex receives a start and end time. 
 * The start time signifies when the processing of that vertex was started and the end time signifies when the vertex
 * finished processing. Tarjan made a smart observation and noticed that the start time of a vertex  can be used to determine the 
 * strongly connected component it is a part of.
 * 
 * A leader of a strong component as defined by Tarjan is a vertex with the earliest start time in that component. It naturally follows
 * that the vertex with earliest start time will have the latest end time. Every root of a depth first tree is the 
 * leader of that component.
 * 
 * Another way of defining the leader of a strong component is that a vertex v can only be the leader if and only if 
 * within it's strong component there exist no cycle which contains v and a vertex x such that in the cycle x contains an earlier start time
 * than v. Basically, if v is the leader of the component then it must be the root because if another vertex exists with an earlier
 * start time then v clearly cannot be the root. 
 * 
 * 
 * When performing the depth first search on a graph, Tarjan strongly connected algorithm also tags each each
 * vertex v with the field low. v.low is initially set to the start time of the vertex and thereafter 
 * v.low is updated by a retreating edge or after each children w is processed.  v.low = min(v.low, w.low) i.e.
 * if we have the back/retreating edge (v, w) or when the w is finished processing we set v.low to that of w.low if 
 * w.low is earlier than v.low. 
 * 
 * After v is finished processing  if v.low != v.start then we push v onto a stack, otherwise 
 * we pop the stack until the top of the stack is less than v.low.
 * 
 * 
 */

/**
 * @brief                       Finds the strongly connected components of a digraph using Tarjan algorithm.
 * 
 * @param graph                 Graph.
 * @return struct r2_forest*    Returns strongly connected components, else NULL.
 */
struct r2_forest* r2_graph_tscc(struct r2_graph *graph)
{
        r2_uint16 FAILED    = FALSE;
        struct r2_graph *cc = NULL;
        struct r2_forest *forest = malloc(sizeof(struct r2_forest));/*stores the connected components*/
        struct r2_list  *trees   = r2_create_list(NULL, NULL, NULL);/*stores a connected component*/
        r2_dbl *pre  = malloc(sizeof(r2_dbl) * graph->nvertices);/*stores preorder number of each vertex*/
        r2_dbl *low  = malloc(sizeof(r2_dbl) * graph->nvertices);/*stores low number of each vertex*/
        r2_uint64 *state = malloc(sizeof(r2_uint64) * graph->nvertices);/*stores the state of each vertex*/
        struct r2_arrstack *stack       =  r2_arrstack_create_stack(0, NULL, NULL, NULL);/*store the current vertex depth first search is on*/
        struct r2_robintable *processed =  r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        if(pre == NULL || low == NULL || state == NULL || stack == NULL || processed == NULL || forest == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        
        struct r2_listnode *head     = r2_listnode_first(graph->vlist); 
        struct r2_listnode *cur      = NULL;
        struct r2_vertex   *source   = NULL; 
        struct r2_vertex   *dest     = NULL;
        struct r2_edge     *edge     = NULL;
        struct r2_entry    entry     = {.key = NULL, .data = NULL, .length  = 0};
        r2_int64 count = -1;
        r2_uint64 *vstate = NULL;
        r2_uint64 pos[2] = {0, 0};
        while(head != NULL){
                source = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data; 
                if(vstate == NULL){
                        state[++count] = GREY; 
                        pre[count] = low[count] = count;
                        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }

                        cur = r2_listnode_first(source->elist);
                        do{
                                while(cur != NULL){
                                        /*Get source index*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry); 
                                        vstate = entry.data; 
                                        pos[1] = (vstate - state);

                                        edge = cur->data; 
                                        dest = edge->dest; 
                                        /*Get destination index*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data;
                                        pos[0] = (vstate - state);

                                        if(vstate == NULL){
                                                vstate = &state[++count]; 
                                                *vstate = GREY; 
                                                pre[count] = low[count] = count;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE 
                                                || r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                } 

                                                source = dest; 
                                                cur    =  r2_listnode_first(source->elist);
                                                if(cur == NULL)
                                                        break;
                                                continue;
                                        }
                                        /**
                                         * If we have reached here it is possible that we have back, forward or cross edge.
                                         * We need to consider the case of a forward or back edge. If we have a back edge
                                         * we need to check if we can reach an ancestor of source. If we have a forward 
                                         * edge we still need to check if we can use that forward edge to reach some ancestor higher than
                                         * the one we can reach from source.
                                         */
                                        if(*vstate != BLACK)
                                                low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]];
                                        cur = cur->next;
                                }

                                if(cur == NULL){
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate       = entry.data;
                                        pos[1]       = (vstate - state);
                                        /**
                                         * @brief Checking if source is the leader of its component and creating 
                                         * sub graph.
                                         * 
                                         */
                                        if(low[pos[1]] == pre[pos[1]]){
                                                *vstate = YELLOW;
                                                cc = r2_graph_build_tscc(graph, source, processed, state);
                                                if(cc == NULL){
                                                        FAILED = TRUE;
                                                        goto CLEANUP;
                                                }
                                                if(r2_list_insert_at_back(trees, cc) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                        }

                                        cur  = r2_arrstack_top(stack);
                                        if(cur != NULL){
                                                edge   = cur->data; 
                                                source = edge->src;
                                                dest   = edge->dest;
                                                /**
                                                 * @brief We are retreating up the tree. It is possible that our descendant has a back edge 
                                                 * that points to an ancestor of source. Check for this possibility.
                                                 * 
                                                 */
                                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                                vstate = entry.data;        
                                                pos[1] = (vstate - state);

                                                r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                                vstate = entry.data;
                                                pos[0] = (vstate - state);
                                                if(*vstate != BLACK)
                                                        low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]]; 
                                                cur = cur->next;
                                        }else source = NULL;
                                        r2_arrstack_pop(stack);  
                                }
                        }while(source != NULL);
                }
                head = head->next;
        }

        forest->ncount = trees->lsize;
        forest->tree   = malloc(sizeof(struct r2_graph *) * trees->lsize);
        if(forest->tree == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        head = r2_listnode_first(trees);
        count = 0;
        while(head != NULL){
                forest->tree[count++] = head->data;
                head = head->next; 
        }

        CLEANUP:
                if(pre != NULL)
                        free(pre);
                
                if(low != NULL)
                        free(low); 
                
                if(state != NULL)
                        free(state); 
                
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                
                if(processed != NULL)
                        r2_destroy_robintable(processed);
                
                if(FAILED == TRUE && forest != NULL)
                        forest = r2_graph_destroy_cc(forest);

                if(FAILED == TRUE && trees != NULL){
                        head = r2_listnode_first(trees);
                        while(head != NULL){
                                r2_destroy_graph(head->data);
                                head = head->next;
                        }
                }

                if(trees != NULL)
                        r2_destroy_list(trees);

        return forest;
}

/**
 * @brief                      Builds the strongly connected component starting from source.
 * 
 * @param graph                Graph.
 * @param source               Source.      
 * @param processed            Stores all vertices processed so far.
 * @param state                Stores the state of all vertices so far.
 * @return struct r2_graph*    Returns connected component, else NULL.
 */
static struct r2_graph* r2_graph_build_tscc(struct r2_graph *graph, struct r2_vertex *source, struct r2_robintable *processed, r2_uint64 *state)
{
        r2_int16 FAILED = FALSE;
        struct r2_graph *bfs    = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue  = r2_create_queue(NULL, NULL, NULL);
        struct r2_list  *vertices = r2_create_list(NULL, NULL, NULL);
        if(queue == NULL || bfs == NULL || vertices == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        bfs->nat = TRUE; 
        bfs->gat = graph->gat;
        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *src   = NULL;
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 *vstate = NULL;
        struct r2_robintable *att[3];
        r2_uint16 ADD_EDGE = FALSE;
        r2_uint16 ENQUEUE  = FALSE;

        /*Initializing queue with source vertex*/
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        /*We need to insert the leader in this component*/
        if(r2_graph_add_vertex(bfs, source->vkey, source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        src = r2_graph_get_vertex(bfs, source->vkey, source->len);
        src->vat = source->vat;
        do{
                source = r2_queue_front(queue)->data;
                att[0] = source->vat;
                
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;               
                if(r2_list_insert_at_back(vertices, vstate) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                /*Processing source edge list*/
                head   = r2_listnode_first(source->elist);
                while(head != NULL){

                        ADD_EDGE = FALSE;
                        ENQUEUE  = FALSE;
                        edge = head->data;
                        dest = edge->dest;

                        att[1] = dest->vat;
                        att[2] = edge->eat;

                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate  = entry.data;
                        /**
                         * @brief If vertex is not apart of any component then it must be a part of this 
                         *        component. Think of our implmentation as the S. R. Kosaraju implicit. 
                         *        We start from the source and go down the tree. We can encounter components
                         *        which would already be black. We don't have to worry about components 
                         *        that haven't been reached as yet simple because we can't reach ancestor 
                         *        of source. If thiS is the case then source can't be a the leader of the 
                         *        component which holds for Tarjan algorithm.
                         */
                        if(*vstate != BLACK){
                                ADD_EDGE = TRUE;
                                if(*vstate != YELLOW)
                                        ENQUEUE = TRUE;     
                        }

                        if(ENQUEUE == TRUE){
                                if(r2_queue_enqueue(queue, dest) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                *vstate  = YELLOW;
                        }

                        if(ADD_EDGE == TRUE){
                                if(r2_graph_add_edge(bfs, source->vkey, source->len, dest->vkey, dest->len) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                src  = r2_graph_get_vertex(bfs, source->vkey, source->len);
                                dest = r2_graph_get_vertex(bfs, dest->vkey, dest->len);
                                edge = r2_graph_get_edge(bfs, source->vkey, source->len, dest->vkey, dest->len);
                                src->vat  = att[0];
                                dest->vat = att[1];
                                edge->eat = att[2];  
                        }
                        head = head->next; 
                }
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;
                *vstate = YELLOW;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE);
        
        head = r2_listnode_first(vertices);
        while(head != NULL){
                vstate = head->data; 
                *vstate = BLACK;
                head = head->next; 
        }
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 

                if(bfs != NULL && FAILED == TRUE)
                        bfs = r2_destroy_graph(bfs);
                
                if(vertices != NULL)
                        r2_destroy_list(vertices);
        return bfs;         
}


/**
 * @brief                       Finds the strongly connected components in a digraph using S. R. Kosaraju algorithm.
 * 
 * @param graph                 Graph.
 * @return struct r2_forest*    Returns forest, else NULL.
 */
struct r2_forest* r2_graph_kcc(struct r2_graph *graph)
{
        r2_int16 FAILED = FALSE;
        r2_uint64 count = 0;
        r2_int64 ID = -1;
        struct r2_graph *cc        = NULL; /*component*/
        struct r2_graph **tree     = NULL;
        struct r2_forest *forest   = NULL;
        struct r2_list *topsort    = r2_graph_dfs_traversals(graph, NULL, 2);
        struct r2_graph *transpose = r2_graph_transpose(graph);
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint64 *state = malloc(sizeof(r2_uint64) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list *components = r2_create_list(NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || components == NULL || topsort == NULL || transpose == NULL || graph->nvertices == 0){
                FAILED = TRUE;  
                goto CLEANUP; 
        } 

        /*Initialize state of all vertices*/
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        struct r2_vertex *source =  NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        while(head != NULL){
                source = head->data;
                state[count] = WHITE; 
                if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                        FAILED = TRUE;
                        goto CLEANUP;
                }
                head = head->next;
                ++count;
        }

        /*Processing graph*/
        head  = r2_listnode_first(topsort);
        while(head != NULL){
                source = head->data; 
                r2_robintable_get(processed, source->vkey, source->len, &entry); 
                vstate = entry.data;
                if(*vstate == WHITE){
                        source  = r2_graph_get_vertex(transpose, source->vkey, source->len);
                        *vstate = ID--; 
                        cc = r2_graph_components(transpose, source, processed, *vstate);
                        if(cc == NULL){
                                FAILED = TRUE;
                                goto CLEANUP;
                        }

                        if(r2_list_insert_at_back(components, cc) != TRUE){
                                r2_destroy_graph(cc); 
                                FAILED = TRUE;
                                goto CLEANUP;
                        }
                }
                head = head->next;
        }
        
        forest = malloc(sizeof(struct r2_forest));
        if(forest == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        tree = malloc(sizeof(struct r2_graph *) * components->lsize);
        if(tree != NULL){
                head = r2_listnode_first(components);
                count = 0;
                while(head != NULL){
                        tree[count++] = head->data;
                        head = head->next;
                }

                forest->tree = tree; 
                forest->ncount = count;
        }else FAILED = TRUE;

        CLEANUP:
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                
                if(FAILED == TRUE && components != NULL){   
                        /*Cleaning up components found so far*/
                        head = r2_listnode_first(components);
                        while(head != NULL){
                                r2_destroy_graph(head->data);
                                head = head->next;
                        }   
                }

                if(components != NULL)
                        r2_destroy_list(components);  

                if(topsort != NULL)
                        r2_destroy_list(topsort); 

                if(transpose != NULL)
                        r2_destroy_graph(transpose);

                if(FAILED == TRUE && forest != NULL)
                      forest =  r2_graph_destroy_cc(forest);

        return forest;
}


/**
 * A bi-connected graph is any graph in which the removal of a vertex doesn't disconnect the graph. 
 * Another way of stating this, is all paths are distinct else from the source and destination vertex. 
 * 
 * If a graph is not bi-connected then it has biconnected components in where the removal of vertex increases
 * the number of components. In finding the bi-connected components we can identify sections in the graph where redunancies exist,
 * points of failure (articulation points and bridges).
 * 
 * We can use the ideas from the Tarjan Strongly Connected components to identify biconnected components.
 * 
 * To find the biconnected component we start with a DFS similar to the Tarjan strongly connected component algorithm. 
 * For each unexplored vertex v we assign a number the first time it is reached by DFS. If our unexplored vertex v leads to an 
 * explored vertex w that is not apart of a biconnected component and is not our immediate parent then v.low = min(v.low, w.low).
 * 
 * Once we are post visiting a vertex we need to check if the vertex is an articulation point. If any of v children
 * can reach an ancestor w of v then v can not be an articulation point simply because if we remove v then it's children
 * can still reach w. 
 * 
 */

/**
 * @brief                               Returns the forest of biconnected components of an undirected graph.
 *                                      
 * @param graph                         Graph.
 * @return struct r2_forest*            Returns biconnected components, else NULL.
 */
struct r2_forest* r2_graph_bcc(struct r2_graph *graph)
{
        r2_uint16 FAILED =  FALSE; 
        r2_int64 *state  =  malloc(sizeof(r2_int64) * graph->nvertices);
        r2_dbl *low = malloc(sizeof(r2_dbl) * graph->nvertices); 
        r2_dbl *pre = malloc(sizeof(r2_dbl) * graph->nvertices);
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_arrstack *edges = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .80, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list  *tree = r2_create_list(NULL, NULL, NULL);
        struct r2_graph *bcc  = NULL; 
        struct r2_forest *forest = malloc(sizeof(struct r2_forest));
        if(state == NULL || low == NULL || pre == NULL || stack == NULL || processed == NULL || tree == NULL || forest == NULL || edges == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_vertex *source = NULL; 
        struct r2_vertex *dest   = NULL; 
        struct r2_edge   *edge   = NULL;
        struct r2_listnode *cur  = NULL; 
        struct r2_listnode *head = r2_listnode_first(graph->vlist); 
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        r2_int64 count = -1; 
        r2_uint64 pos[2] = {0};
        while(head != NULL){
                source = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        vstate  = &state[++count];
                        *vstate = GREY; 
                        pre[count] = low[count] = count; 
                        if(r2_robintable_put(processed, source->vkey, vstate, source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        cur = r2_listnode_first(source->elist);
                        do{
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        
                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest and determining if we have already processed it.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        if(vstate == NULL){
                                                vstate  = &state[++count];
                                                *vstate = GREY; 
                                                pre[count] = low[count] = count;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE || r2_arrstack_push(edges, edge) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP; 
                                                }
                                                source = dest; 
                                                cur    = r2_listnode_first(source->elist); 
                                                if(cur == NULL)
                                                        break; 
                                                continue;
                                        }

                                        /**
                                         * Checking to see if any of our children can reach an ancestor higher than we can currently reach. 
                                         * We disregard the reverse edge (dest, source).
                                         * 
                                         */
                                        if(pre[pos[0]] <= pre[pos[1]]){
                                                if(r2_arrstack_empty(stack) != TRUE){
                                                        edge = ((struct r2_listnode *)r2_arrstack_top(stack))->data;
                                                        if(edge->src != dest || edge->dest != source)
                                                                low[pos[1]] = pre[pos[0]] < low[pos[1]]? pre[pos[0]] :  low[pos[1]];
                                                }
                                        }
                                        cur  = cur->next;
                                }
                                
                                cur = r2_arrstack_top(stack); 
                                if(cur != NULL){
                                        edge = cur->data; 
                                        source = edge->src; 
                                        dest   = edge->dest;

                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        
                                        
                                        low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]];
                                        /**
                                         * @brief We have found an articulation point. 
                                         * 
                                         */
                                        if(low[pos[0]] >= pre[pos[1]]){
                                               bcc =  r2_graph_build_bcc(graph, edges, edge);
                                               if(r2_list_insert_at_back(tree, bcc) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                               }
                                        }
                                        cur = cur->next;
                                }else source =  NULL;
                                r2_arrstack_pop(stack);
                        }while(source != NULL);              
                }
                head = head->next; 
        }

        forest->tree   = malloc(sizeof(struct r2_graph *) * tree->lsize);
        forest->ncount = tree->lsize;
        if(forest->tree == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        head = r2_listnode_first(tree); 
        count = 0; 
        while(head != NULL){
                forest->tree[count++] = head->data; 
                head = head->next;
        }
        
        CLEANUP: 
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pre != NULL)
                        free(pre); 

                if(low != NULL)
                        free(low);

                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);

                if(edges != NULL)
                        r2_arrstack_destroy_stack(edges);
                
                if(FAILED == TRUE && tree != NULL){
                        head = r2_listnode_first(tree);
                        while(head != NULL){
                                r2_destroy_graph(head->data);
                                head = head->next; 
                        }
                }

                if(tree != NULL)
                        r2_destroy_list(tree);
                
                if(FAILED == TRUE && forest != NULL)
                        r2_graph_destroy_cc(forest);
                
        return forest;
}

/**
 * @brief                               Helper function to build bi-connected component.
 * 
 * @param graph                         Graph.
 * @param edges                         Stack containing all edges explored.
 * @param bridge                        Base edge of the component.
 * @return struct r2_graph*             Returns the bi-connected component.
 */
static struct r2_graph* r2_graph_build_bcc(struct r2_graph *graph, struct r2_arrstack *edges, struct r2_edge *bridge)
{
        r2_uint16 FAILED       = FALSE;
        struct r2_graph *bcc   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        if(bcc == NULL || queue == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_entry entry  = {.key = NULL, .data = NULL, .length = 0};
        struct r2_edge *edge   = NULL; 
        struct r2_vertex *src  = NULL; 
        struct r2_vertex *dest = NULL; 
        struct r2_robintable *att[3] = {NULL};
        bcc->nat = TRUE; 
        bcc->gat = graph->gat;
        while(r2_arrstack_empty(edges) != TRUE){
                edge = r2_arrstack_top(edges); 
                src  = edge->src; 
                dest = edge->dest;

                att[0] = edge->eat; 
                att[1] = src->vat; 
                att[2] = dest->vat;

                if(r2_graph_add_edge(bcc, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                edge = r2_graph_get_edge(bcc, src->vkey, src->len, dest->vkey, dest->len);
                src  = r2_graph_get_vertex(bcc, src->vkey, src->len); 
                dest = r2_graph_get_vertex(bcc, dest->vkey, dest->len);
                edge->eat = att[0];
                src->vat  = att[1]; 
                dest->vat = att[2];
                edge = r2_arrstack_top(edges);
                r2_arrstack_pop(edges); 
                if(edge == bridge)
                        break;
        }

        /**
         * @brief Adding the remaining edges using a breadth first search.
         * 
         */
        struct r2_listnode *head = r2_listnode_first(bcc->vlist);
        src = head->data;
        src = r2_graph_get_vertex(graph, src->vkey, src->len);
        if(r2_queue_enqueue(queue, src) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        do{
                src  = r2_queue_front(queue)->data;
                att[1] = src->vat; 
                head = r2_listnode_first(src->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;

                        att[0] = edge->eat; 

                        att[2] = dest->vat;
                        /**
                         * @brief If dest is in bcc we can add this edge to bcc and add dest to queue. 
                         * If not that means the dest is isn't a part of this bi-connected component.
                         * 
                         */
                        r2_robintable_get(bcc->vertices, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                edge = r2_graph_get_edge(bcc, src->vkey, src->len, dest->vkey, dest->len);
                                if(edge == NULL){
                                        if(r2_queue_enqueue(queue, dest) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP; 
                                        }

                                        if(r2_graph_add_edge(bcc, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                                FAILED = TRUE;
                                                goto CLEANUP;
                                        } 

                                        edge = r2_graph_get_edge(bcc, src->vkey, src->len, dest->vkey, dest->len);
                                        src  = r2_graph_get_vertex(bcc, src->vkey, src->len); 
                                        dest = r2_graph_get_vertex(bcc, dest->vkey, dest->len);
                                        edge->eat = att[0];
                                        src->vat  = att[1]; 
                                        dest->vat = att[2];
                                }
                        }
                        head = head->next;
                }
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE);

        CLEANUP:
                if(FAILED == TRUE && bcc != NULL)
                        bcc = r2_destroy_graph(bcc);
                if(queue != NULL)
                        r2_destroy_queue(queue);
        return bcc;    
}

/**
 * @brief                       Determines if a graph is bi-connected.
 * 
 * @param graph                 Graph. 
 * @return r2_uint16            Returns TRUE if graph is biconnected, else FALSE.
 */
r2_uint16 r2_graph_is_biconnected(struct r2_graph *graph)
{
        r2_uint16 FAILED =  FALSE; 
        r2_uint64 BICONNECTED = 0;
        r2_int64 *state  =  malloc(sizeof(r2_int64) * graph->nvertices);
        r2_dbl *low = malloc(sizeof(r2_dbl) * graph->nvertices); 
        r2_dbl *pre = malloc(sizeof(r2_dbl) * graph->nvertices);
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        if(state == NULL || low == NULL || pre == NULL || stack == NULL || processed == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_vertex *source = NULL; 
        struct r2_vertex *dest   = NULL; 
        struct r2_edge   *edge   = NULL;
        struct r2_listnode *cur  = NULL; 
        struct r2_listnode *head = r2_listnode_first(graph->vlist); 
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        r2_int64 count = -1; 
        r2_uint64 pos[2] = {0};
        while(head != NULL){
                source = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        vstate  = &state[++count];
                        *vstate = GREY; 
                        pre[count] = low[count] = count; 
                        if(r2_robintable_put(processed, source->vkey, vstate, source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        cur = r2_listnode_first(source->elist);
                        do{
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        
                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest and determining if we have already processed it.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        if(vstate == NULL){
                                                vstate  = &state[++count];
                                                *vstate = GREY; 
                                                pre[count] = low[count] = count;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP; 
                                                }
                                                source = dest; 
                                                cur    = r2_listnode_first(source->elist); 
                                                if(cur == NULL)
                                                        break; 
                                                continue;
                                        }

                                        /**
                                         * Checking to see if any of our children can reach an ancestor higher than we can currently reach. 
                                         * We disregard the reverse edge (dest, source).
                                         * 
                                         */
                                        if(pre[pos[0]] <= pre[pos[1]]){
                                                if(r2_arrstack_empty(stack) != TRUE){
                                                        edge = ((struct r2_listnode *)r2_arrstack_top(stack))->data;
                                                        if(edge->src != dest || edge->dest != source)
                                                                low[pos[1]] = pre[pos[0]] < low[pos[1]]? pre[pos[0]] :  low[pos[1]];
                                                }
                                        }
                                        cur  = cur->next;
                                }
                                
                                cur = r2_arrstack_top(stack); 
                                if(cur != NULL){
                                        edge = cur->data; 
                                        source = edge->src; 
                                        dest   = edge->dest;

                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        
                                        
                                        low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]];
                                        /**
                                         * @brief We have found a potential articulation point. To know if the graph is bi-connected we can use a 
                                         * subtle trick by counting the leaders of biconnected components. A bi-connected graph has one bi-connected
                                         * component itself which has only one leader which is the root of the tree.
                                         * 
                                         * Now if more than one bi-connected component exists then we're going to discover multiple leaders better yet 
                                         * potential articulation points. If we discovered more than one then the graph can't be bi-connected.
                                         */
                                        if(low[pos[0]] >= pre[pos[1]]){
                                                BICONNECTED++;
                                                if(BICONNECTED > 1)
                                                        goto CLEANUP;
                                        }
                                        cur = cur->next;
                                }else source =  NULL;
                                r2_arrstack_pop(stack);
                        }while(source != NULL);              
                }
                head = head->next; 
        }

        CLEANUP: 
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pre != NULL)
                        free(pre); 

                if(low != NULL)
                        free(low);

                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                
        return BICONNECTED == 1 || BICONNECTED == 0;
}

/**
 * @brief                       Finds all the articulation points in an undirected graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_list*      Returns a list of articulation points, else empty list if no articulation points exists. 
 */
struct r2_list* r2_graph_articulation_points(struct r2_graph *graph)
{
        /*Contains all articulation points*/
        struct r2_list *artpoints = r2_create_list(NULL, NULL, NULL);
        r2_uint16 FAILED =  FALSE; 
        r2_int64 *state  =  malloc(sizeof(r2_int64) * graph->nvertices);
        r2_dbl *low = malloc(sizeof(r2_dbl) * graph->nvertices); 
        r2_dbl *pre = malloc(sizeof(r2_dbl) * graph->nvertices);
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        if(state == NULL || low == NULL || pre == NULL || stack == NULL || processed == NULL || graph->nvertices == 0 || artpoints == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }


        struct r2_vertex *cut[2] = {NULL}; 
        struct r2_vertex *source = NULL; 
        struct r2_vertex *dest   = NULL; 
        struct r2_edge   *edge   = NULL;
        struct r2_listnode *cur  = NULL; 
        struct r2_listnode *head = r2_listnode_first(graph->vlist); 
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        r2_int64 count   = -1; 
        r2_uint64 pos[2] = {0};
        r2_int64 *index[2]  = {NULL};
        while(head != NULL){
                source = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        vstate  = &state[++count];
                        *vstate = GREY; 
                        pre[count] = low[count] = count; 
                        if(r2_robintable_put(processed, source->vkey, vstate, source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        cur = r2_listnode_first(source->elist);
                        do{
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        
                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest and determining if we have already processed it.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        if(vstate == NULL){
                                                vstate  = &state[++count];
                                                *vstate = GREY; 
                                                pre[count] = low[count] = count;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP; 
                                                }
                                                source = dest; 
                                                cur    = r2_listnode_first(source->elist); 
                                                if(cur == NULL)
                                                        break; 
                                                continue;
                                        }

                                        /**
                                         * Checking to see if any of our children can reach an ancestor higher than we can currently reach. 
                                         * We disregard the reverse edge (dest, source).
                                         * 
                                         */
                                        if(pre[pos[0]] <= pre[pos[1]]){
                                                if(r2_arrstack_empty(stack) != TRUE){
                                                        edge = ((struct r2_listnode *)r2_arrstack_top(stack))->data;
                                                        if(edge->src != dest || edge->dest != source)
                                                                low[pos[1]] = pre[pos[0]] < low[pos[1]]? pre[pos[0]] :  low[pos[1]];
                                                }
                                        }
                                        cur  = cur->next;
                                }
                                
                                cur = r2_arrstack_top(stack); 
                                index[0] = index[1] = NULL;
                                cut[0] = cut[1] = NULL;
                                if(cur != NULL){
                                        edge = cur->data; 
                                        source = edge->src; 
                                        dest   = edge->dest;

                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        if(*vstate == BLACK){
                                                cut[0] = source;
                                                index[0] = vstate;
                                        }
                                        
                 
                                        /*Getting index of dest.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        if(*vstate == BLACK){
                                                cut[1] = dest;
                                                index[1] = vstate;
                                        }
                                        for(r2_uint16 i = 0; i < 2; ++i){
                                                if(index[i] != NULL && *index[i] == BLACK){
                                                        *index[i] = YELLOW;
                                                        if(r2_list_insert_at_back(artpoints, cut[i]) != TRUE){
                                                                FAILED = TRUE; 
                                                                goto CLEANUP;                                                         
                                                        }
                                                }
                                        }

                                        low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]];
                                        /**
                                         * @brief We have found a potential articulation point. Why do we call it a potential articulation point? 
                                         * For source to be considered an articulation point it must have incident edges in two different bi-connected components 
                                         * or it must be a part of two different cycles. Whenever we discover a new bi-connected component starting at source,
                                         * we set source state to BLACK to signify that we aren't sure if it's actually articulation point. Now, let's consider
                                         * three cases: 
                                         *      1) when source is the root with mutiple children.
                                         *              When source is the root with two children, we actually visit source more than once which means we are
                                         *              a part of multiple bi-connected components. Upon the first visit we set it to BLACK and on the second vist we set it
                                         *              to YELLOW to signify that we are an articulation point.
                                         * 
                                         *      2) when source is the root with one  child.
                                         *              When source is the root with one child then it can't be articulation point because we will only vist source once 
                                         *              which means we are a part of only one bi-connected component.
                                         * 
                                         *      3) when source is not the root.
                                         *              When source is not the root that means we have a both an ancestor and descendant. 
                                         *              The first time source is visited we're the leader of the bi-connected component which means we are BLACK. 
                                         *              There must have been a tree edge that has source has the endpoint hence we check the state of dest if it's BLACK.
                                         *              If it's BLACK then we're apart of another component and this component which means we're an articulation point
                                         *              YELLOW.
                                         *              
                                         */
                                        if(low[pos[0]] >= pre[pos[1]] && state[pos[1]] != YELLOW)
                                                state[pos[1]] = BLACK;

                                        cur = cur->next;
                                }else source =  NULL;
                                r2_arrstack_pop(stack);
                        }while(source != NULL);              
                }
                head = head->next; 
        }

        CLEANUP: 
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pre != NULL)
                        free(pre); 

                if(low != NULL)
                        free(low);

                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);

                if(FAILED == TRUE && artpoints != NULL)
                        artpoints = r2_destroy_list(artpoints);
                
        return artpoints;       
}

/**
 * @brief                       Finds all the bridges in an undirected graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_list*      Returns all brigdges in the graph, else empty list if no bridges exist.
 */
struct r2_list* r2_graph_bridges(struct r2_graph *graph)
{
        r2_uint16 FAILED =  FALSE; 
        r2_int64 *state  =  malloc(sizeof(r2_int64) * graph->nvertices);
        r2_dbl *low = malloc(sizeof(r2_dbl) * graph->nvertices); 
        r2_dbl *pre = malloc(sizeof(r2_dbl) * graph->nvertices);
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list *bridges = r2_create_list(NULL, NULL, NULL);
        if(state == NULL || low == NULL || pre == NULL || stack == NULL || processed == NULL || bridges == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_vertex *source = NULL; 
        struct r2_vertex *dest   = NULL; 
        struct r2_edge   *edge   = NULL;
        struct r2_listnode *cur  = NULL; 
        struct r2_listnode *head = r2_listnode_first(graph->vlist); 
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_int64 *vstate = NULL;
        r2_int64 count = -1; 
        r2_uint64 pos[2] = {0};
        while(head != NULL){
                source = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        vstate  = &state[++count];
                        *vstate = GREY; 
                        pre[count] = low[count] = count; 
                        if(r2_robintable_put(processed, source->vkey, vstate, source->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        cur = r2_listnode_first(source->elist);
                        do{
                                while(cur != NULL){
                                        edge = cur->data;
                                        dest = edge->dest; 
                                        
                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest and determining if we have already processed it.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        if(vstate == NULL){
                                                vstate  = &state[++count];
                                                *vstate = GREY; 
                                                pre[count] = low[count] = count;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP; 
                                                }
                                                source = dest; 
                                                cur    = r2_listnode_first(source->elist); 
                                                if(cur == NULL)
                                                        break; 
                                                continue;
                                        }

                                        /**
                                         * Checking to see if any of our children can reach an ancestor higher than we can currently reach. 
                                         * We disregard the reverse edge (dest, source).
                                         * 
                                         */
                                        if(pre[pos[0]] <= pre[pos[1]]){
                                                if(r2_arrstack_empty(stack) != TRUE){
                                                        edge = ((struct r2_listnode *)r2_arrstack_top(stack))->data;
                                                        if(edge->src != dest || edge->dest != source)
                                                                low[pos[1]] = pre[pos[0]] < low[pos[1]]? pre[pos[0]] :  low[pos[1]];
                                                }
                                        }
                                        cur  = cur->next;
                                }
                                cur = r2_arrstack_top(stack); 
                                if(cur != NULL){
                                        edge = cur->data; 
                                        source = edge->src; 
                                        dest   = edge->dest;

                                        /*Getting index of source*/
                                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                                        vstate = entry.data;
                                        pos[1] = vstate - state;
                                        
                                        /*Getting index of dest.*/
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        pos[0] = vstate - state;
                                        
                                        
                                        low[pos[1]] = low[pos[0]] < low[pos[1]]? low[pos[0]] :  low[pos[1]];
                                        /**
                                         * @brief Detecting if edge is a bridge. If dest can't 
                                         * traverse tree edges then a back edge to an ancestor of 
                                         * source then it's a bridge. We basically check the lowest
                                         * dest can reach and if it's lowest is not above source it's
                                         * a bridge.
                                         * 
                                         */
                                        if(low[pos[0]] > pre[pos[1]]){
                                                if(r2_list_insert_at_back(bridges, edge) != TRUE){
                                                        FAILED = TRUE;
                                                        goto CLEANUP;
                                                }
                                        }
                                        cur = cur->next;
                                }else source =  NULL;
                                r2_arrstack_pop(stack);
                        }while(source != NULL);              
                }
                head = head->next; 
        }

        CLEANUP: 
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pre != NULL)
                        free(pre); 

                if(low != NULL)
                        free(low);

                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                
                if(FAILED == TRUE && bridges != NULL)
                        bridges = r2_destroy_list(bridges);
        
        return bridges;
}

/**
 * @brief                       Finds the shortest path from source using Dijkstra shortest path algorithm.
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @param len                   Length. 
 * @param weight                A callback function that get's the weight for an edge. A simple and the recommended approach
 *                              is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
 *                              callback function to find that weight in edge->eat. 
 * @return struct r2_graph*     Returns shortest path tree, else NULL.
 */
struct r2_graph* r2_graph_dijkstra(struct r2_graph *graph, r2_uc *source, r2_uint64 len, r2_weight weight)
{
        r2_uint16 FAILED = FALSE;
        struct r2_vertex *src  = r2_graph_get_vertex(graph, source, len);
        struct r2_vertex *dest = src;
        struct r2_graph *spt   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_pq *pq  = r2_create_priority_queue(0, 0, wcmp, NULL, NULL);
        struct r2_dist *weights = malloc(sizeof(struct r2_dist) * graph->nvertices);
        
        if(spt == NULL || processed == NULL || pq == NULL || weights == NULL || graph->nvertices == 0 || src == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /**
         * @brief Initializes all vertices.
         * 
         */
        r2_uint64 count = 0;       
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        struct r2_locator *loc = NULL;
        while(head != NULL){
                src = head->data;
                weights[count].vertex = src;
                weights[count].dist   = INFINITY;
                if(dest == src)
                        weights[count].dist = 0;

                loc = r2_pq_insert(pq, &weights[count]);
                if(loc != NULL){
                        if(r2_robintable_put(processed, src->vkey, loc, src->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;   
                        }
                }
                else{
                        FAILED = TRUE; 
                        goto CLEANUP;  
                }
                ++count;
                head = head->next;
        }

        struct r2_dist* dist[2] = {NULL, NULL};
        struct r2_edge *edge = NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_dbl *w;
        do{
                loc = r2_pq_first(pq);
                dist[0] = loc->data;
                src = dist[0]->vertex;
                head = r2_listnode_first(src->elist);
                r2_pq_remove(pq, r2_pq_first(pq));
                r2_robintable_del(processed, src->vkey, src->len);
                while(head != NULL){
                        edge = head->data;
                        src  = edge->src;
                        dest = edge->dest;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                loc     = entry.data;
                                dist[1] = loc->data;
                                /**
                                 * @brief Perform relaxation
                                 * 
                                 */
                                if((dist[0]->dist + weight(edge)) < dist[1]->dist){
                                        dist[1]->dist = dist[0]->dist + weight(edge);
                                        r2_pq_adjust(pq, loc, 0);
                                }
                        }
                        head = head->next;
                }


                /*Adding distance attribute.*/
                if(dist[0]->dist != INFINITY){
                        /*Adding vertex to shortest path tree*/
                        if(r2_graph_add_vertex(spt, src->vkey, src->len)  != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP; 
                        }

                        w = malloc(sizeof(r2_dbl));
                        if(w != NULL){
                                *w  = dist[0]->dist;
                                src = r2_graph_get_vertex(spt, src->vkey, src->len);
                                if(r2_vertex_add_attributes(src, "0xdfs", w, 5, vat_cmp) != TRUE){
                                        free(w);
                                        FAILED = TRUE; 
                                        goto CLEANUP;  
                                }
                        }else{
                                FAILED = TRUE; 
                                goto CLEANUP;   
                        }
                }
        }while(r2_pq_empty(pq) != TRUE);
        spt = r2_graph_build_spt(graph, spt, r2_graph_get_vertex(graph, source, len), weight);
        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pq != NULL)
                        r2_destroy_priority_queue(pq); 
                
                if(weights != NULL)
                        free(weights);

                if(FAILED == TRUE && spt != NULL)
                        spt = r2_destroy_graph(spt);

        return spt;
}

/**
 * @brief                         Builds the shortest path tree.
 * 
 * @param graph                   Graph.
 * @param spt                     Shortest path tree.
 * @param source                  Source.
 * @param weight                  A callback function that get's the weight for an edge.
 * @return struct r2_graph*       Returns shortest path tree, else NULL.
 */
static struct r2_graph* r2_graph_build_spt(struct r2_graph *graph, struct r2_graph *spt, struct r2_vertex *source, r2_weight weight)
{
        r2_int16 FAILED = FALSE;
        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_uint16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP; 
        }


        struct r2_listnode *head   = NULL;
        struct r2_vertex   *src    = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_edge     *edge   = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;   
        
        /*Initializing queue with source vertex*/
        if(r2_queue_enqueue(queue, source) != TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        /*Updating source state in hash table*/
        state[count] = GREY; 
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        r2_uint16 *vstate = NULL;
        r2_dbl *dist[2] = {NULL};
        do{
                source  = r2_queue_front(queue)->data;
                src     = r2_graph_get_vertex(spt, source->vkey, source->len);
                dist[0] = r2_vertex_get_attributes(src,"0xdfs", 5, vat_cmp);          
                head    = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest; 

                        dest    = r2_graph_get_vertex(spt, dest->vkey, dest->len);
                        dist[1] = r2_vertex_get_attributes(dest,"0xdfs", 5, vat_cmp);
                        
                        /*Add edge to SPT.*/
                        if(*dist[0] + weight(edge) == *dist[1]){
                                if(r2_graph_add_edge(spt, source->vkey, source->len, dest->vkey, dest->len) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }

                        dest = edge->dest; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is WHITE.
                         */
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE || 
                                r2_queue_enqueue(queue, dest) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }

                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate  = entry.data;
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE);
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                if(spt != NULL && FAILED == TRUE)
                        spt = r2_destroy_graph(spt);
        return spt;       
}

/**
 * @brief                               Performs Bellman Ford shortest path algorithm on the graph.
 * 
 * @param graph                         Graph.
 * @param source                        Source.
 * @param len                           Length.
 * @param weight                        A callback function that get's the weight for an edge. A simple and the recommended approach
 *                                      is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
 *                                      callback function to find that weight in edge->eat.                        
 * @return struct r2_graph*             Returns the shortest path tree, else NULL if a negative cycle exists.
 */
struct r2_graph* r2_graph_bellman_ford(struct r2_graph *graph, r2_uc *source, r2_uint64 len, r2_weight weight)
{
        r2_uint16 FAILED = FALSE;
        r2_uint16 RELAX  = FALSE;
        struct r2_vertex *src  = r2_graph_get_vertex(graph, source, len);
        struct r2_vertex *dest = src;
        struct r2_graph *spt   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dist *weights = malloc(sizeof(struct r2_dist) * graph->nvertices);
        if(spt == NULL || processed == NULL  || weights == NULL || graph->nvertices == 0 || src == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /**
         * @brief Initializes all vertices.
         * 
         */
        r2_uint64 count = 0;       
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        while(head != NULL){
                src = head->data;
                weights[count].vertex = src;
                weights[count].dist   = INFINITY;
                if(dest == src)
                        weights[count].dist = 0;

                if(r2_robintable_put(processed, src->vkey, &weights[count], src->len) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;   
                }
                
                ++count;
                head = head->next;
        }

        struct r2_dist *dist[2] = {NULL};
        struct r2_edge *edge    = NULL; 
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_dbl *w; 
        head = r2_listnode_first(graph->vlist); 
        struct r2_listnode *cur = NULL;
        while(head != NULL){
                cur = r2_listnode_first(graph->elist); 
                RELAX = FALSE;
                while(cur != NULL){
                        edge = cur->data;
                        src  = edge->src; 
                        dest = edge->dest;

                        r2_robintable_get(processed, src->vkey, src->len, &entry);
                        dist[0] = entry.data;

                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        dist[1] = entry.data;
                        /**
                         * @brief Perform relaxation
                         * 
                         */
                        if((dist[0]->dist + weight(edge)) < dist[1]->dist){
                                dist[1]->dist = (dist[0]->dist + weight(edge));
                                RELAX = TRUE;
                        }
                        cur  = cur->next;
                }
                if(RELAX == FALSE)
                        break;
                head = head->next;
        }

        /**
         * Adding reachable vertices to graph
         */
        for(r2_uint64 i = 0; i < graph->nvertices; ++i){
                if(weights[i].dist != INFINITY){
                        src = weights[i].vertex;
                        if(r2_graph_add_vertex(spt, src->vkey, src->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;        
                        }
                        w = malloc(sizeof(r2_dbl));
                        if(w != NULL){
                                *w  = weights[i].dist;
                                src = r2_graph_get_vertex(spt, src->vkey, src->len);
                                if(r2_vertex_add_attributes(src, "0xdfs", w, 5, vat_cmp) != TRUE){
                                        free(w);
                                        FAILED = TRUE; 
                                        goto CLEANUP;  
                                }
                        }else{
                                FAILED = TRUE; 
                                goto CLEANUP;   
                        }
                }
        }
        if(r2_graph_detect_negative_cycle(graph, processed, weight) == TRUE){
                FAILED = TRUE; 
                goto CLEANUP;  
        }
        spt = r2_graph_build_spt(graph, spt, r2_graph_get_vertex(graph, source, len), weight);
        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(weights != NULL)
                        free(weights);

                if(FAILED == TRUE && spt != NULL)
                        spt = r2_destroy_graph(spt);

        return spt;     
}
/**
 * @brief                       Finds negative cycle after Bellman Ford algorithm.
 * 
 * @param graph                 Graph.
 * @param processed             Contains the distances of all vertices.
 * @param weight                A callback function that get's the weight for an edge. A simple and the recommended approach
 *                              is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
 *                              callback function to find that weight in edge->eat.              
 * @return r2_uint16            Returns TRUE upon negative cycle, else FALSE.
 */
static r2_uint16 r2_graph_detect_negative_cycle(struct r2_graph *graph, struct r2_robintable *processed, r2_weight weight)
{
        struct r2_edge *edge     = NULL;
        struct r2_listnode *head = r2_listnode_first(graph->elist);
        struct r2_vertex *src    = NULL; 
        struct r2_vertex *dest   = NULL; 
        struct r2_entry entry    = {.key = NULL, .data = NULL, .length = 0}; 
        struct r2_dist *dist[2]  = {NULL};

        while(head != NULL){
                edge = head->data; 
                src  = edge->src; 
                dest = edge->dest;
                r2_robintable_get(processed, src->vkey, src->len, &entry); 
                dist[0]  = entry.data;

                r2_robintable_get(processed, dest->vkey, dest->len, &entry); 
                dist[1]  = entry.data;
                if((dist[0]->dist + weight(edge)) < dist[1]->dist){
                        dist[1]->dist = (dist[0]->dist + weight(edge));
                        return TRUE;
                }

                head = head->next;
        }

        return FALSE;
}

/**
 * @brief                               Finds the shortest path in DAG (Directed Acyclic Graph).
 *                                      It's the user responsibility to ensure the graph does not contain a negative cycle.                                     
 * 
 * @param graph                         Graph.
 * @param source                        Source. 
 * @param len                           Length.
 * @param weight                        A callback function that get's the weight for an edge. A simple and the recommended approach
 *                                      is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
 *                                      callback function to find that weight in edge->eat.  
 * @return struct r2_graph*             Returns shortest path tree, else NULL.
 */
struct r2_graph* r2_graph_shortest_dag(struct r2_graph *graph, r2_uc *source,  r2_uint64 len, r2_weight weight)
{
        r2_uint16 FAILED = FALSE;
        struct r2_vertex *src  = r2_graph_get_vertex(graph, source, len);
        struct r2_vertex *dest = src;
        struct r2_graph *spt   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dist *weights = malloc(sizeof(struct r2_dist) * graph->nvertices);
        struct r2_list *topsort = r2_graph_dfs_traversals(graph, NULL,  2);
        if(spt == NULL || processed == NULL  || weights == NULL || graph->nvertices == 0 || src == NULL || topsort == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /**
         * @brief Initializes all vertices.
         * 
         */
        r2_uint64 count = 0;       
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        while(head != NULL){
                src = head->data;
                weights[count].vertex = src;
                weights[count].dist   = INFINITY;
                if(dest == src)
                        weights[count].dist = 0;

                if(r2_robintable_put(processed, src->vkey, &weights[count], src->len) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;   
                }
                
                ++count;
                head = head->next;
        }

        struct r2_dist *dist[2] = {NULL};
        struct r2_edge *edge    = NULL; 
        struct r2_entry entry   = {.key = NULL, .data = NULL, .length = 0};
        r2_dbl *w; 
        head = r2_listnode_first(topsort); 
        struct r2_listnode *cur = NULL;
        while(head != NULL){
                src = head->data;
                cur = r2_listnode_first(src->elist); 
                while(cur != NULL){
                        edge = cur->data;
                        src  = edge->src; 
                        dest = edge->dest;

                        r2_robintable_get(processed, src->vkey, src->len, &entry);
                        dist[0] = entry.data;

                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        dist[1] = entry.data;
                        /**
                         * @brief Perform relaxation
                         * 
                         */
                        if((dist[0]->dist + weight(edge)) < dist[1]->dist){
                                dist[1]->dist = (dist[0]->dist + weight(edge));
                        }
                        cur  = cur->next;
                }
                head = head->next;
        }

        /**
         * Adding reachable vertices to graph
         */
        for(r2_uint64 i = 0; i < graph->nvertices; ++i){
                if(weights[i].dist != INFINITY){
                        src = weights[i].vertex;
                        if(r2_graph_add_vertex(spt, src->vkey, src->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;        
                        }
                        w = malloc(sizeof(r2_dbl));
                        if(w != NULL){
                                *w  = weights[i].dist;
                                src = r2_graph_get_vertex(spt, src->vkey, src->len);
                                if(r2_vertex_add_attributes(src, "0xdfs", w, 5, vat_cmp) != TRUE){
                                        free(w);
                                        FAILED = TRUE; 
                                        goto CLEANUP;  
                                }
                        }else{
                                FAILED = TRUE; 
                                goto CLEANUP;   
                        }
                }
        }
        
        spt = r2_graph_build_spt(graph, spt, r2_graph_get_vertex(graph, source, len), weight);
        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(weights != NULL)
                        free(weights);

                if(FAILED == TRUE && spt != NULL)
                        spt = r2_destroy_graph(graph);

                if(topsort != NULL)
                        r2_destroy_list(topsort);
        return spt;
}

/**
 * @brief                       Finds the minimum spanning tree using Prim-Jarnik algorithm.
 * 
 *@param graph                  Graph.
* @param weight                 A callback function that get's the weight for an edge. A simple and the recommended approach
*                               is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
*                               callback function to find that weight in edge->eat.               
 * @return struct r2_graph*     Returns the minimum spanning tree, else NULL.
 */
struct r2_graph* r2_graph_mst_prim(struct r2_graph *graph, r2_weight weight)
{
        struct r2_mst{
                struct r2_vertex *vertex; 
                r2_dbl dist;
                struct r2_edge *edge;        
        };

        r2_uint16 FAILED = FALSE;
        struct r2_graph *mst = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_pq *pq  = r2_create_priority_queue(0, 0, wcmp, NULL, NULL);
        struct r2_mst *weights = malloc(sizeof(struct r2_mst) * graph->nvertices);
        if(mst == NULL || processed == NULL || pq == NULL || weights == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /**
         * @brief Initializes all vertices.
         * 
         */
        r2_uint64 count = 0;       
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        struct r2_locator *loc = NULL;
        struct r2_vertex *src  = NULL; 
        struct r2_vertex *dest = NULL;
        while(head != NULL){
                src = head->data;
                weights[count].vertex = src;
                weights[count].dist   = INFINITY;
                weights[count].edge   = NULL;
                if(count == 1)
                        weights[0].dist = 0;
                loc = r2_pq_insert(pq, &weights[count]);
                if(loc != NULL){
                        if(r2_robintable_put(processed, src->vkey, loc, src->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;   
                        }
                }
                else{
                        FAILED = TRUE; 
                        goto CLEANUP;  
                }
                ++count;
                head = head->next;
        }
        

        struct r2_mst* dist[2] = {NULL, NULL};
        struct r2_edge *edge = NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        do{
                loc = r2_pq_first(pq);
                dist[0] = loc->data;
                src = dist[0]->vertex;
                head = r2_listnode_first(src->elist);
                r2_pq_remove(pq, r2_pq_first(pq));
                r2_robintable_del(processed, src->vkey, src->len);
                while(head != NULL){
                        edge = head->data;
                        src  = edge->src;
                        dest = edge->dest;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                loc     = entry.data;
                                dist[1] = loc->data;
                                /**
                                 * @brief Perform relaxation
                                 * 
                                 */
                                if((weight(edge)) < dist[1]->dist){
                                        dist[1]->dist =  weight(edge);
                                        dist[1]->edge = edge;
                                        r2_pq_adjust(pq, loc, 0);
                                }
                        }
                        head = head->next;
                }

                /*Adding vertex to minimum spanning tree*/
                if(r2_graph_add_vertex(mst, src->vkey, src->len)  != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP; 
                }

                /*Adding edge*/
                if(dist[0]->edge != NULL){
                        edge = dist[0]->edge; 
                        src  = edge->src;
                        dest = edge->dest;
                        if(r2_graph_add_edge(mst, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                FAILED = TRUE; 
                                goto CLEANUP;     
                        }
                }
        }while(r2_pq_empty(pq) != TRUE); 
        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed); 
                
                if(pq != NULL)
                        r2_destroy_priority_queue(pq); 
                
                if(weights != NULL)
                        free(weights);

                if(FAILED == TRUE && mst != NULL)
                        mst = r2_destroy_graph(graph);

        return mst;
}

/**
 * @brief                       Finds the minimum spanning tree using Kruskal algorithm.
 * 
 *@param graph                  Graph.
* @param weight                 A callback function that get's the weight for an edge. A simple and the recommended approach
*                               is to store the edge weight as an attribute of the edge in edge->eat. Then the user provides a 
*                               callback function to find that weight in edge->eat.               
 * @return struct r2_graph*     Returns the minimum spanning tree, else NULL.
 */
struct r2_graph* r2_graph_mst_kruskal(struct r2_graph *graph, r2_weight weight)
{
        r2_uint16 FAILED = FALSE;
        struct r2_vertex *src   = NULL; 
        struct r2_vertex *dest  = NULL; 
        struct r2_edge *edge    = NULL;
        struct r2_graph *mst    = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_universe *set = r2_create_universe(graph->vcmp, NULL);
        struct r2_dist *weights = malloc(sizeof(struct r2_dist) * graph->nedges);
        struct r2_pq* pq = r2_create_priority_queue(0, 0, wcmp, NULL, NULL);
        if(mst == NULL || set == NULL ||  weights == NULL || pq == NULL || graph->nvertices == 0){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /*Make set*/
        struct r2_listnode *head = r2_listnode_first(graph->vlist); 
        while(head != NULL){
                src = head->data;
                if(r2_makeset(set, src->vkey, src->len) != TRUE || 
                r2_graph_add_vertex(mst, src->vkey, src->len) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                head = head->next;
        }

        /*Make heap*/
        head = r2_listnode_first(graph->elist);
        r2_uint64 i = 0;
        while(head != NULL){
                edge = head->data; 
                weights[i].vertex = (struct r2_vertex *)edge;/*we're using it store an edge.*/
                weights[i].dist   = weight(edge);
                if(r2_pq_insert(pq, &weights[i]) == NULL){
                        FAILED = TRUE; 
                        goto CLEANUP;  
                }
                ++i;
                head = head->next;
        }

        struct r2_locator  *loc = NULL;
        struct r2_dist *dist = NULL;
        for(;mst->nedges < graph->nvertices -1 && graph->nedges > 0 && r2_pq_empty(pq) != TRUE;){
                loc  = r2_pq_first(pq); 
                if(loc != NULL){
                        dist = loc->data; 
                        edge = (struct r2_edge *)dist->vertex;
                        r2_pq_remove(pq, loc);
                        src  = edge->src; 
                        dest = edge->dest; 
                        if(r2_sameset(set, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                /*add edge to mst*/
                                if(r2_unionset(set, src->vkey, src->len, dest->vkey, dest->len) != TRUE || 
                                r2_graph_add_edge(mst, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }
                }
        }
        CLEANUP:
                if(weights != NULL)
                        free(weights);
                
                if(pq != NULL)
                        r2_destroy_priority_queue(pq); 

                if(set != NULL)
                        r2_destroy_universe(set);
                
                if(FAILED == TRUE && mst != NULL)
                        mst = r2_destroy_graph(mst);
                
        return mst;
}

/**
 * @brief          Free list of lists.
 * 
 * @param list     List.
 */
static void r2_free_list(void *list)
{
       assert(r2_destroy_list(list) == NULL); 
}

/**
 * @brief                 Compares two edges
 * 
 * @param a               Src
 * @param b               Dest
 * @return r2_int16       Returns a == b => 0 , a > b => 1,  a < b => -1.
 */
static r2_int16 r2_cmp_edge(const void *a, const void *b)
{
        const struct r2_edge *src  = (struct r2_edge *)((struct r2_key*)a)->key; 
        const struct r2_edge *dest = (struct r2_edge *)((struct r2_key*)b)->key;
        return src == dest? 0 : 1;
}

static void action(void *a, void*b)
{
        r2_uint64 *c = b; 
        *c = *c + 1;
}

/**
 * @brief               Callback function comparison function for priority queue.
 * 
 * @param a
 * @param b 
 * @return r2_uint16 
 */
static r2_int16 wcmp(const void *a, const void *b)
{
        const struct r2_dist *c = (const struct r2_dist *)a; 
        const struct r2_dist *d = (const struct r2_dist *)b;

        if(c->dist <= d->dist)
                return 0;
        return 1;
}

/**
 * @brief       Frees edge along with edge attribute table
 * 
 * @param edge  Edge
 */
static void r2_free_edge_data(void *edge)
{
        struct r2_edge *e = edge; 
        if(e->eat != NULL && e->nat == FALSE)
                r2_destroy_robintable(e->eat); 
        
        free(e);
}

/**
 * @brief           Frees vertex along with vertex attribute table
 * 
 * @param vertex    Vertex
 */
static void r2_free_vertex_data(void *vertex)
{
        struct r2_vertex *v = vertex; 
        if(v->vat != NULL && v->nat == FALSE){
                r2_fd fd = v->vat->fd;
                v->vat->fd = free;
                r2_vertex_del_attributes(v, "0xdfs", 5, vat_cmp);
                v->vat->fd = fd;
                r2_destroy_robintable(v->vat); 
        }

        r2_destroy_robintable(v->edges); 
        r2_destroy_list(v->elist); 
        r2_destroy_list(v->out);
        r2_destroy_list(v->in);
        free(v);
}

static r2_int16 vat_cmp(const void *a, const void *b){
        char *c = ((struct r2_key *)a)->key; 
        char *d = ((struct r2_key *)b)->key;
        return strcmp(c, d);
}

/**
 * @brief                       Returns the distance of a vertex in shortest path tree.
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @param len                   Length.
 * @return r2_ldbl              Returns the distance if it exists, else INFINITY.
 */
r2_dbl r2_graph_dist_from_source(struct r2_graph *graph, r2_uc *source, r2_uint64 len)
{
        struct r2_vertex *src = r2_graph_get_vertex(graph, source, len);
        
        if(src == NULL)
                return INFINITY; 

        r2_dbl *dist = r2_vertex_get_attributes(src,"0xdfs", 5, vat_cmp); 
        return  *dist;
}

/**
 * @brief                   Creates copy of graph.
 * 
 * @param graph             Graph.
 * @return struct r2_graph* Returns copy of graph, else NULL.
 */
struct r2_graph* r2_graph_copy(const struct r2_graph *graph)
{
        struct r2_graph *copy = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_robintable *att[3] = {NULL};
        copy->nat = TRUE; 
        copy->gat = graph->gat;
        if(copy != NULL){
                struct r2_vertex *src   = NULL; 
                struct r2_vertex *dest  = NULL;
                struct r2_edge *edge    = NULL; 
                struct r2_listnode *head = r2_listnode_first(graph->elist); 
                while(head != NULL){
                        edge = head->data; 
                        src  = edge->src; 
                        dest = edge->dest; 
                        att[0] = edge->eat;
                        att[1] = src->vat;
                        att[2] = dest->vat;
                        if(r2_graph_add_edge(copy, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                copy = r2_destroy_graph(copy);
                                break;
                        }
                        
                        src  = r2_graph_get_vertex(copy, src->vkey, src->len);
                        dest = r2_graph_get_vertex(copy, dest->vkey, dest->len);
                        edge = r2_graph_get_edge(copy, src->vkey, src->len, dest->vkey, dest->len); 
                        edge->eat = att[0];
                        src->vat  = att[1];
                        dest->vat = att[2];
                        head = head->next;
                } 
        }

        return copy;
}

/**
 * @brief                       Creates transitive closure of graph.
 *                              
 * @param graph                 Graph.
 * @return struct r2_graph*     Returns transitive closure of graph, else NULL.
 */
struct r2_graph* r2_graph_transitive_closure(const struct r2_graph *graph)
{
        r2_uint16 FAILED  = FALSE;
        struct r2_graph *closure  = r2_graph_copy(graph); 
        if(closure == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_vertex *src  = NULL;  
        struct r2_vertex *dest = NULL; 
        struct r2_vertex *con  = NULL; 
        struct r2_listnode *head[3] = {NULL, NULL, NULL};

        for(head[0] = r2_listnode_first(closure->vlist); head[0] != NULL; head[0] = head[0]->next){
                con = head[0]->data;
                for(head[1] = r2_listnode_first(closure->vlist); head[1] != NULL; head[1] = head[1]->next){
                        src = head[1]->data;
                        for(head[2] = r2_listnode_first(closure->vlist); head[2] != NULL; head[2] = head[2]->next){
                                dest = head[2]->data;
                                if(src == dest){
                                        if(r2_graph_add_edge(closure, src->vkey, src->len, src->vkey, src->len) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        continue;
                                }

                                if(r2_graph_get_edge(closure, src->vkey, src->len, dest->vkey, dest->len) != NULL || 
                                (r2_graph_get_edge(closure, src->vkey, src->len, con->vkey, con->len) != NULL && 
                                r2_graph_get_edge(closure, con->vkey, con->len, dest->vkey, dest->len) != NULL)){
                                        if(r2_graph_add_edge(closure, src->vkey, src->len, dest->vkey, dest->len) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        continue;     
                                }
                        }
                }
        }
          

        CLEANUP:
                if(FAILED == TRUE)
                        closure = r2_destroy_graph(closure);

        return closure;
}