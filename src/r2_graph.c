#include "r2_graph.h"
#include "r2_arrstack.h"
#include "r2_queue.h"
#include "r2_list.h"
#include "r2_heap.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
/********************File scope functions************************/
/**
 * Stores the depth first information related to each vertex. 
 * This information helps us to perform tarjan strongly connected
 * component and also helps us to build the dfs tree explicitly.
 */
struct r2_dfsinfo{
        struct r2_vertex   *vertex;/*current vertex*/
        r2_int64  parent;/*parent*/
        r2_uint16 state;/*current state of vertex*/
        r2_uint64 start;/*start time*/
        r2_uint64 end;/*end time*/
        r2_ldbl low;/*lowest ancestor we can reach from the current vertex*/
        r2_ldbl dist;/*distance from parent*/
        r2_uint16 incomponent; /*TRUE when vertex is already in a component*/
};

static void r2_free_vertex(struct r2_graph *, struct r2_vertex *);
static void r2_free_edge(struct r2_graph *,  struct r2_edge *);
static void r2_free_list(void *);
static r2_int16 r2_cmp_edge(const void *, const void *);
static struct r2_vertex* r2_create_vertex(r2_cmp, r2_uint16);
static struct r2_edge*   r2_create_edge(r2_uint16);
static struct r2_graph*  r2_graph__components(struct r2_graph *, struct r2_vertex *, struct r2_robintable *);
static void r2_graph_tarjan_tree_components(r2_uint64 *, struct r2_vertex *, struct r2_robintable *, struct r2_arrstack *, struct r2_dfsinfo *, struct r2_graph *, struct r2_components *);
static struct r2_dfstree* r2_tarjan_followers(struct r2_dfsinfo *, struct r2_dfsinfo *, struct r2_arrstack *, struct r2_graph *);

/**
 * WHITE - We have not started to process the adjacency list of this vertex.
 * GREY  - We have started to process this vertex but haven't completed processing. 
 * BLACK - We have completed processing the vertex.
 */
const r2_uint16 WHITE = 0; 
const r2_uint16 GREY  = 1;
const r2_uint16 BLACK = 2; 
static r2_int16 cmp(const void *, const void *);
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
                graph->vertices  = r2_create_robintable(1, 1, 0, 0, 0, vmcp, NULL, NULL, NULL, fv, free);
                graph->vlist     = r2_create_list(NULL, NULL, NULL);
                graph->elist     = r2_create_list(NULL, NULL, NULL);
                graph->vcmp      = vmcp;
                graph->gcmp      = gcmp;
                graph->fv        = fv; 
                graph->fk        = fk;
                graph->fd        = fd;
                graph->vat       = FALSE;
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
        struct r2_vertex *vertex = NULL;
        while(r2_list_empty(graph->vlist) != TRUE){
                vertex = r2_listnode_first(graph->vlist)->data;
                r2_free_vertex(graph, vertex);
        }
        if(graph->vat == FALSE)
                r2_destroy_robintable(graph->gat); 
        
        r2_destroy_list(graph->elist); 
        r2_destroy_list(graph->vlist);
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
                vertex = r2_create_vertex(graph->vcmp, graph->vat);
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
        }
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
                edge = r2_create_edge(graph->vat);  
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
        }else goto FINAL;

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
 * @param vat                   IF TRUE the vertex is created without the attribute hash table. 
 *  
 * @return struct r2_vertex*    Returns an empty vertex, else NULL.
 */
static struct r2_vertex* r2_create_vertex(r2_cmp cmp,  r2_uint16 vat)
{
        struct r2_vertex *vertex = malloc(sizeof(struct r2_vertex));
        if(vertex != NULL){
                vertex->vkey            = NULL; 
                vertex->len             = 0; 
                vertex->pos             = NULL; 
                vertex->in              = r2_create_list(NULL, NULL, NULL);  
                vertex->out             = r2_create_list(NULL, NULL, NULL); 
                vertex->elist           = r2_create_list(NULL, NULL, NULL); 
                vertex->edges           = r2_create_robintable(1, 1, 0, 0, .75, cmp, NULL, NULL, NULL, NULL, free);
                vertex->nedges          = 0;
                vertex->vat             = vat == TRUE? NULL : r2_create_robintable(1, 1, 0, 0, .75, NULL, NULL, NULL, NULL, NULL, NULL);

                
                if(vertex->in == NULL || vertex->out == NULL || vertex->elist == NULL   || vertex->edges == NULL || (vat == FALSE && vertex->vat == NULL)){
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

                        free(vertex); 
                        vertex = NULL;   

                        if(vat == FALSE && vertex->vat != NULL)
                                r2_destroy_robintable(vertex->vat);         
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
 * @param vat                   If TRUE edge is created without attribute hash table.
 * @return struct r2_edge*      Returns an empty edge, else NULL.
 */
static struct r2_edge*  r2_create_edge(r2_uint16 vat)
{
        struct r2_edge *edge = malloc(sizeof(struct r2_edge)); 
        if(edge != NULL){
                edge->src         = NULL; 
                edge->dest        = NULL; 
                for(r2_uint64 i = 0; i < 4; ++i)
                        edge->pos[i] = NULL; 
                edge->eat         = vat == TRUE? NULL : r2_create_robintable(1, 1, 0, 0, .75, NULL, NULL, NULL, NULL, NULL, NULL); 
                /**
                 * All metadata is important. Failure to allocate requisite memory is considered
                 * a failure to create edge.
                 */
                if(vat == FALSE && edge->eat == NULL){
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
        /*Removing vertex from list*/
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
        struct r2_vertex *src  = NULL; 
        struct r2_edge *edge = NULL;
        node = r2_listnode_first(vertex->in);
        while(node != NULL){
                src  = node->data;
                edge = r2_graph_get_edge(graph, src->vkey, src->len, vertex->vkey, vertex->len);
                r2_free_edge(graph, edge);
                node = r2_listnode_first(vertex->in);
        }


        r2_destroy_robintable(vertex->edges); 
        if(graph->vat == FALSE)
                r2_destroy_robintable(vertex->vat);
        r2_destroy_list(vertex->elist); 
        r2_destroy_list(vertex->out);
        r2_destroy_list(vertex->in);
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
        
        if(graph->vat == FALSE)
                r2_destroy_robintable(edge->eat); 
        if(src != NULL){
                r2_robintable_del(src->edges, dest->vkey, dest->len);
                --src->nedges;
        }
}



/********************************************Graph Algorithms*************************************************/

/**
 * @brief               Checks with a graph has a cycle.
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
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
        r2_uint64 count = 0;
        
        vertex  = r2_listnode_first(graph->vlist);
        while(vertex != NULL){
                source = vertex->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate    = entry.data; 
                if(vstate == NULL){
                        vstate    = &state[count++];
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
                                                vstate    = &state[count++];
                                                *vstate   = GREY;
                                                if(r2_robintable_put(processed, dest->vkey, vstate, dest->len) != TRUE ||
                                                r2_arrstack_push(stack, edge->pos[0]) != TRUE)
                                                        goto CLEANUP;

                                                source  = dest;
                                                head    = source->elist != NULL? r2_listnode_first(source->elist) : NULL;
                                                if(head == NULL)
                                                        break;

                                                continue;       
                                        }else if(*vstate == GREY){
                                                CYCLE = TRUE;
                                                goto CLEANUP;
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
                                        }else
                                                source = NULL;
                                        
                                        r2_arrstack_pop(stack);
                                }    
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
        struct r2_graph *transpose = r2_create_graph(graph->vcmp, graph->gcmp, NULL, NULL, NULL); 
        if(transpose != NULL){
                transpose->vat = TRUE;
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
void r2_graph_bfs(struct r2_graph *graph, struct r2_vertex *source,r2_act action, void *arg)
{
        r2_int16 FAILED = FALSE;

        /*Holds vertices that are currently being processed*/
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = NULL;
        struct r2_vertex   *dest   = NULL; 
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

        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                head   = r2_listnode_first(source->out);
                if(action != NULL)
                        action(source, arg);
                while(head != NULL){
                        dest    = head->data; 
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
        struct r2_vertex *source    = r2_listnode_first(graph->vlist)->data;
        struct r2_graph  *transpose = r2_graph_transpose(graph);
        if(transpose != NULL){
                struct r2_graph *bfs[2]   = {r2_graph_bfs_tree(graph, source), 
                                             r2_graph_bfs_tree(transpose, r2_graph_get_vertex(transpose, source->vkey, source->len))};
                CONNECTED =  bfs[0]->nvertices == bfs[1]->nvertices;
                r2_destroy_graph(bfs[0]); 
                r2_destroy_graph(bfs[1]);
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;

        /*Updating source state in hash table*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        state[count] = GREY;
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
        
        assert(FAILED == FALSE);       
}

/**
 * @brief                       Performs DFS on a graph and store the vertices in either preorder, 
 *                              postorder, or reverse postorder.
 *                              
 * @param graph                 Graph.
 * @param source                Source.
 * @param order                 Order. order == preorder,  1 == postorder, order == reverse postorder
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL || list == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
        
        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        
        struct r2_listnode *cur = r2_listnode_first(graph->vlist);
        r2_uint16 *vstate = NULL;
        while(cur != NULL){
                if(ENTIRE_GRAPH == TRUE)
                        source = cur->data;

                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        state[count] = GREY;
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

                                if(head == NULL){
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
                                }    
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
        if(top == NULL || queue == NULL || indegree == NULL || count == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_queuenode *node = NULL;
        struct r2_vertex *source  = NULL;
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
                if(r2_list_insert_at_back(top, source) != TRUE){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                head  = r2_listnode_first(source->out);
                while(head != NULL){
                        source = head->data; 
                        r2_robintable_get(indegree, source->vkey, source->len, &entry);
                        if(entry.key != NULL){
                                in = entry.data; 
                                *in = *in -1; 
                                if(*in == 0){
                                        if(r2_queue_enqueue(queue, source) != TRUE){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        if(r2_robintable_del(indegree, source->vkey, source->len) != TRUE){
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
        if(top == NULL || queue == NULL || indegree == NULL || count == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_queuenode *node = NULL;
        struct r2_vertex *source  = NULL;
        struct r2_vertex *dest    = NULL;
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
                head  = r2_listnode_first(source->out);
                while(head != NULL){
                        dest = head->data; 
                        r2_robintable_get(indegree, dest->vkey, dest->len, &entry);
                        if(entry.key != NULL){
                                in = entry.data; 
                                *in = *in -1; 
                                if(*in == 0){
                                        if(r2_list_insert_at_back(top, r2_graph_get_edge(graph, source->vkey, source->len, dest->vkey, dest->len)) != TRUE){
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL ){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
                
        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *destination = NULL;
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
                head   = r2_listnode_first(source->out);
                a.key = source->vkey; 
                a.len = source->len;
                if(graph->vcmp(&a, &b) == 0){
                        PATH = TRUE; 
                        break;
                }
                while(head != NULL){
                        destination = head->data; 
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

        if(stack == NULL || path == NULL || onpath == NULL || paths == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        struct r2_vertex   *source       = src; 
        struct r2_vertex   *destination  = dest;
        struct r2_vertex   *current      = NULL;
        struct r2_edge     *edge         = NULL;
        struct r2_listnode *head         = NULL; 
        struct r2_entry    entry         = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;

        /*Adding source to path*/
        path[count] = source;
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

                if(head == NULL){
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
                        }else{
                                source = NULL;
                        }
                        r2_arrstack_pop(stack);
                }    
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
        if(paths == NULL || edges == NULL || processed == NULL){
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
 * @brief                       Performs BFS on graph.
 *                              The BFS tree is a really a subgraph of the original graph. Additonally, 
 *                              a tree is a rooted graph and because of this choose it's more natural to 
 *                              represent the tree has a graph.  N.B Adding any attributes to this graph 
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || bfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        bfs->gat = graph->gat;
        bfs->vat = TRUE;
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
                head   = r2_listnode_first(source->out);
                while(head != NULL){
                        dest = head->data; 
                        edge = r2_graph_get_edge(graph, source->vkey, source->len, dest->vkey, dest->len);
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
                                src  = r2_graph_get_vertex(bfs, dest->vkey, dest->len);
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL || dfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        dfs->vat = TRUE; 
        dfs->gat = graph->gat;
        struct r2_listnode *head  = NULL; 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;

        /*Updating source state in hash table*/
        source = source == NULL? r2_listnode_first(graph->vlist)->data : source;
        state[count] = GREY;
        if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                FAILED = TRUE; 
                goto CLEANUP;
        }


        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);

        r2_uint16 *vstate = NULL;
        struct r2_vertex *v[2];
        struct r2_robintable *att; 
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
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = r2_listnode_first(graph->vlist);
        struct r2_listnode *cur    = NULL;
        struct r2_vertex   *dest   = NULL; 
        struct r2_vertex   *source = NULL;
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count   = 0;   
        r2_uint16 *curset = NULL;
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                source  = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        state[count] = WHITE; 
                        if(r2_queue_enqueue(queue, source) != TRUE || 
                        r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                                FAILED = TRUE;
                                goto CLEANUP;
                        }
                
                        do{
                                source = r2_queue_front(queue)->data;
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                curset = entry.data;
                                cur    = r2_listnode_first(source->out);
                                while(cur != NULL){
                                        dest    = cur->data; 
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
 * @param graph                 Graph 
 * @param set                   set.
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
        state = malloc(sizeof(r2_int16) * graph->nvertices);
        processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL || group == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = r2_listnode_first(graph->vlist);
        struct r2_listnode *cur    = NULL;
        struct r2_vertex   *dest   = NULL; 
        struct r2_vertex   *source = NULL;
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count   = 0;   
        r2_uint16 *curset = NULL;
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                source  = head->data;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        state[count] = WHITE; 
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
                                cur    = r2_listnode_first(source->out);
                                while(cur != NULL){
                                        dest    = cur->data; 
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
struct r2_components* r2_graph_cc(struct r2_graph *graph)
{
        r2_int16 FAILED = FALSE;
        r2_uint64 count = 0;
        struct r2_graph *cc = NULL; /*component*/
        struct r2_components *forest = NULL;
        /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list *components = r2_create_list(NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || components == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        } 

        /*Initialize state of all vertices*/
        struct r2_listnode *head = r2_listnode_first(graph->vlist);
        struct r2_vertex *source =  NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0}; 
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                source = head->data;
                state[count] = WHITE; 
                if(r2_robintable_put(processed, source->vkey, &state[count], source->len) != TRUE){
                        FAILED = TRUE;
                        goto CLEANUP;
                }
                head = head->next;
        }

        /*Processing graph*/
        head = r2_listnode_first(graph->vlist);
        while(head != NULL){
                source = head->data; 
                r2_robintable_get(processed, source->vkey, source->len, &entry); 
                vstate = entry.data;
                if(*vstate == WHITE){
                        *vstate = GREY; 
                        cc = r2_graph_components(graph, source, processed);
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
        
        forest = malloc(sizeof(struct r2_components));
        if(forest == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        cc = malloc(sizeof(struct r2_graph) * components->lsize);
        if(cc != NULL){
                head = r2_listnode_first(components);
                count = 0;
                while(head != NULL){
                        cc[count++] = head->data;
                        head = head->next;
                }
                forest->forest = cc; 
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
 * @return struct r2_graph*             Returns BFS tree, else NULL.
 */
static struct r2_graph* r2_graph_components(struct r2_graph *graph, struct r2_vertex *source,  struct r2_robintable *processed)
{
        r2_int16 FAILED = FALSE;
        struct r2_graph *bfs   = r2_create_graph(graph->vcmp, graph->gcmp, graph->fv, graph->fk, graph->fd);
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        if(queue == NULL || bfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        bfs->gat = graph->gat;
        bfs->vat = TRUE;
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

        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                att[0]  = source->vat;
                head   = r2_listnode_first(source->out);
                while(head != NULL){
                        dest = head->data; 
                        edge = r2_graph_get_edge(graph, source->vkey, source->len, dest->vkey, dest->len);
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is WHITE.
                         */
                        att[1] = dest->vat;
                        att[2] = edge->eat;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data;
                        if(vstate == WHITE){
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
                                src  = r2_graph_get_vertex(bfs, dest->vkey, dest->len);
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
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE);
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                if(bfs != NULL && FAILED == TRUE)
                        bfs = r2_destroy_graph(bfs);
        return bfs;       
}



/**
 * @brief                               Check whether vertices are in the same connected components.
 * 
 * @param components                    Connected component.
 * @param src                           Src.
 * @param dest                          Dest.
 * @return struct r2_dfstree*           Returns the connected component when both src and dest is connected, else NULL.
 */
struct r2_dfstree* r2_graph_is_connected(struct r2_components *components, struct r2_vertex *src, struct r2_vertex *dest)
{
        struct r2_dfstree* component = NULL; 
        struct r2_entry entries[2];
        for(r2_uint64 i = 0; i < components->ncount; ++i){
                component = components->cc[i];
                entries[0].key = entries[0].data = NULL;
                entries[1].key = entries[1].data = NULL;
                r2_robintable_get(component->positions, src->vkey, src->len, &entries[0]); 
                r2_robintable_get(component->positions, dest->vkey, dest->len, &entries[1]); 
                if(entries[0].key != NULL && entries[1].key != NULL)
                        return component;
        }
        return NULL;
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
 * @brief                               Identifies the strongly connected components of the graph.
 * 
 * @param graph                         Graph.
 * @return struct r2_components*        Returns strongly connected components.
 */
struct r2_components* r2_graph_tarjan_strongly_connected_components(struct r2_graph *graph)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack   *followers   = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_robintable *processed   = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dfsinfo *vertices       = malloc(sizeof(struct r2_dfsinfo) * graph->nvertices); 
        struct r2_components *forest      = malloc(sizeof(struct r2_components));
        struct r2_dfstree **cc            = malloc(sizeof(struct r2_dfstree *) * graph->nvertices);
        if( processed == NULL || followers == NULL || forest == NULL || cc == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        //Initializing forest
        forest->cc = cc;
        forest->ncount = 0;
        forest->transpose = NULL;
        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *source = NULL;
        struct r2_entry    entry   = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 start   = 0; 
        while(head != NULL){
                source    = head->data; 
                entry.key = entry.data = NULL; 
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        /*Initializing root*/
                        vertices[start].vertex  = source;
                        vertices[start].start   = start;
                        vertices[start].low     = start; 
                        vertices[start].dist    = 0;
                        vertices[start].parent  = -1;
                        vertices[start].state   = GREY;
                        r2_robintable_put(processed, source->vkey, &vertices[start], source->len);
                        ++start;
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                        if(entry.key == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        r2_graph_tarjan_tree_components(&start, source, processed, followers, vertices, graph, forest);
                }
                head = head->next;
        }

        CLEANUP:
               if(followers != NULL)
                        r2_arrstack_destroy_stack(followers);

                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                
                if(vertices != NULL)
                        free(vertices);
                

        
        assert(FAILED == FALSE);            
        return forest;
}


/**
 * @brief                       Helper function to find the strong connected components.
 * 
 * @param time                  Time.
 * @param source                Source vertex.
 * @param processed             Hash table containing vertex processed so far.
 * @param followers             Followers of the component leader.
 * @param vertices              Vertices.
 * @param graph                 Graph.
 * @param forest                Forest.
 */
static void r2_graph_tarjan_tree_components(r2_uint64 *time, struct r2_vertex *source, struct r2_robintable *processed, struct r2_arrstack *followers, struct r2_dfsinfo *vertices, struct r2_graph *graph, struct r2_components *forest)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_listnode *head  = NULL;
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        

        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);

        struct r2_dfsinfo *root     = NULL;
        struct r2_dfsinfo *child    = NULL;
        struct r2_dfsinfo *vertex   = NULL; 
        do{
                r2_robintable_get(processed, source->vkey, source->len, &entry); 
                root = entry.data;
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vertex = entry.data; 
                        if(vertex == NULL){
                                vertices[*time].vertex = dest; 
                                vertices[*time].start  = *time; 
                                vertices[*time].low    = *time; 
                                vertices[*time].dist   = root->dist + 1;
                                vertices[*time].state  = GREY;
                                vertices[*time].parent = root->start;
                                r2_robintable_put(processed, dest->vkey, &vertices[*time], dest->len);
                                *time = *time + 1;
                                entry.key = entry.data = NULL;
                                r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                if(entry.key == NULL){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }

                                r2_arrstack_push(stack, edge->pos[0]);
                                if(r2_arrstack_top(stack) != edge->pos[0]){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }

                                source = dest;
                                head = head = r2_listnode_first(source->elist);
                                if(head == NULL)
                                        break;
                                
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                root = entry.data;    
                                continue;
                                     
                        }else if(vertex->state == GREY){
                                /*Compare low values*/
                                root->low = vertex->low < root->low? vertex->low : root->low;
                        }
                        head = head->next; 
                }

                if(head == NULL){
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                        root          = entry.data; 
                        root->state   = BLACK;
                        root->end     = *time; 
                        child = root;
                        /**
                         * If root is our leader then find 
                         * followers, else push it on our follower stack.
                         * 
                         */
                        if(root->low == root->start){
                             forest->cc[forest->ncount++] =   r2_tarjan_followers(vertices, root, followers, graph) ;
                        }else{
                                r2_arrstack_push(followers, root);
                                if(r2_arrstack_top(followers) != root){
                                        FAILED  = TRUE; 
                                        goto CLEANUP;
                                }
                        }

                        head = r2_arrstack_top(stack);
                        if(head != NULL){  
                                edge      = head->data; 
                                source    = edge->src;
                                entry.key = entry.data = NULL;
                                r2_robintable_get(processed, source->vkey, source->len, &entry);
                                root          = entry.data; 
                                //if(child->incomponent != TRUE)
                                root->low = child->low < root->low? child->low : root->low;
                        }
                        r2_arrstack_pop(stack);
                }    
        }while(head != NULL);

        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);

                assert(FAILED == FALSE);
}

/**
 * @brief                       Builds the dfstree for the strongly connected component.
 * 
 * @param vertices              Vertices.
 * @param root                  Root.
 * @param followers             Followers.
 * @param graph                 Graph.
 * @return struct r2_dfstree*   Returns dfstree, else NULL.
 */
static struct r2_dfstree* r2_tarjan_followers(struct r2_dfsinfo *vertices, struct r2_dfsinfo *root, struct r2_arrstack *followers, struct r2_graph *graph)
{
        /**
         * Initializing DFS tree
         * 
         */
        r2_uint64 low    = root->low;
        r2_uint64 count  = 0;
        r2_uint16 FAILED = FALSE;
        struct r2_dfstree    *dfs         = malloc(sizeof(struct r2_dfstree));
        struct r2_dfsnode    *tree        = malloc(sizeof(struct r2_dfsnode) * (followers->ncount  + 1));
        struct r2_robintable *positions   = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_vertex *source = root->vertex;

        if(tree == NULL || positions == NULL || dfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
        dfs->tree      = tree; 
        dfs->ncount    = 0; 
        dfs->positions = positions;

        tree[count].vertex = source; 
        tree[count].parent = -1; 
        tree[count].pos    = count; 
        tree[count].start  = root->start; 
        tree[count].end    = root->end;
        tree[count].state  = BLACK;
        tree[count].dist   = 0;
        dfs->ncount++;

        r2_robintable_put(positions, source->vkey, &tree[count], source->len);
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_robintable_get(positions, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        count++;
        root->low = INFINITY;
        root = r2_arrstack_top(followers); 
        while(root != NULL && root->low >= low){
                if(root->parent != -1){
                        source = vertices[root->parent].vertex;
                        entry.key = entry.data = NULL;
                        r2_robintable_get(positions, source->vkey, source->len, &entry);
                        struct r2_dfsnode *temp = entry.data;
                        tree[count].parent = temp->pos;
                        tree[count].dist   = temp->dist + 1; 
                }

                source = root->vertex;
                tree[count].vertex = source; 
                tree[count].pos    = count; 
                tree[count].start  = root->start; 
                tree[count].end    = root->end;
                tree[count].state  = BLACK;
                r2_robintable_put(positions, source->vkey, &tree[count], source->len);
                entry.key = entry.data = NULL;
                r2_robintable_get(positions, source->vkey, source->len, &entry);
                if(entry.key == NULL){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }
                ++count;
                ++dfs->ncount;
                root->low = INFINITY;
               // root->incomponent = INFINITY;
                r2_arrstack_pop(followers);
                root = r2_arrstack_top(followers); 
        }
        
        if(FAILED == TRUE && positions != NULL)
                r2_destroy_robintable(positions);

        if(FAILED == TRUE && tree != NULL)
                free(tree); 
        
        if(FAILED == TRUE && dfs != NULL){
                free(dfs); 
                dfs = NULL;
        }

        CLEANUP:
                assert(FAILED == FALSE);  
        return dfs;
}








/**
 * @brief                               Uses S. R. Kosaraju algorithm to compute the connected component. 
 *                                      For an in depth explanation of this algorithm check Introduction to 
 *                                      Algorithms page 598.
 * 
 * @param graph                         Graph.
 * @return struct r2_components*        Returns connected components, else NULL.
 */
struct r2_components* r2_graph_strongly_connected_components(struct r2_graph *graph)
{
        r2_int16 FAILED = FALSE;
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices); 
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dfstree **cc = malloc(sizeof(struct r2_dfstree *) *graph->nvertices);
        struct r2_components *forest    = malloc(sizeof(struct r2_components));
        if(state == NULL || processed == NULL || forest == NULL || cc == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        /*Initializing  connected components*/
        forest->ncount = 0;
        forest->cc = cc;
        forest->transpose = NULL;

        for(r2_uint64 i = 0; i < graph->nvertices; ++i)
                state[i] = WHITE;

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *src   = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        r2_uint16 *vstate = NULL;
        while(head != NULL){
                src = head->data; 
                r2_robintable_put(processed, src->vkey, &state[count], src->len);
                entry.key = entry.data = NULL;
                r2_robintable_get(processed, src->vkey, src->len, &entry);
                if(entry.key == NULL){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                head = head->next; 
                count++; 
        }

        struct r2_list  *topological_order = r2_graph_dfs_traversals(graph, NULL, 2); 
        struct r2_graph *transpose = r2_graph_transpose(graph);
        head  = r2_listnode_first(topological_order); 
        count = 0;
        while(head != NULL){
                src = head->data; 
                src = r2_graph_get_vertex(transpose, src->vkey, src->len);
                r2_robintable_get(processed, src->vkey, src->len, &entry);
                vstate = entry.data;
                if(*vstate == WHITE){
                        *vstate = GREY;
                        forest->cc[count] = r2_graph_dfs_tree_components(transpose, src,  processed);
                        if(forest->cc[count] == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        forest->ncount = ++count;
                }

                head = head->next;
        }
        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed); 

                if(FAILED == TRUE && forest != NULL)
                        free(forest); 
                
                if(state != NULL)
                        free(state);

                if(FAILED == TRUE && cc != NULL)
                        free(cc);



                if(topological_order != NULL)
                        r2_destroy_list(topological_order);
                


        assert(FAILED == FAILED);  
        return forest;
}




/**
 * @brief          Free list of lists
 * 
 * @param list 
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