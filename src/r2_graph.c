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
static struct r2_vertex* r2_create_vertex(r2_cmp);
static struct r2_edge*   r2_create_edge();
static struct r2_dfstree* r2_graph_dfs_tree_components(struct r2_graph *, struct r2_vertex *, struct r2_robintable *);
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
 * @param  fv                   A callback function used to free memory used by a vertex.
 * @param  fk                   A callback function used to free memory used by a graph attribute key.
 * @param  fd                   A callback function used to free memory used by a graph attribute data.
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
                /**
                 * All metadata related to the graph is important. 
                 * If we can't allocate necessary structures for the metadata,
                 * we consider graph construction a failure and release memory acquired so
                 * far and return.
                 */
                if(graph->vertices == NULL || graph->gat == NULL || graph->elist == NULL || graph->vlist == NULL){
                        if(graph->gat != NULL)
                                r2_destroy_robintable(graph->gat);

                        if(graph->vertices != NULL)
                                r2_destroy_robintable(graph->vertices);
                       
                        if(graph->elist != NULL)
                                r2_destroy_list(graph->elist); 

                        if(graph->vlist != NULL)
                                r2_destroy_list(graph->vlist);

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
struct r2_graph* r2_destroy_graph(struct r2_graph *graph){

        struct r2_listnode *vertex = r2_listnode_first(graph->vlist);
        while(vertex != NULL){
                r2_free_vertex(graph, vertex->data);
                vertex = r2_listnode_first(graph->vlist);
        }
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
 * @return struct r2_graph*     Returns graph.
 */
struct r2_graph* r2_graph_add_vertex(struct r2_graph *graph, r2_uc *vk, r2_uint64 len)
{
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, vk, len); 
        if(vertex == NULL){
                vertex = r2_create_vertex(graph->vcmp);
                if(vertex != NULL){
                        vertex->vkey  = vk; 
                        vertex->len   = len;
                        r2_robintable_put(graph->vertices, vk, vertex, len);
                        /**
                         * Searching for the same vertex just added.
                         * 
                         */
                        struct r2_entry entry = {.data = NULL, .key = NULL, .length = 0};
                        r2_robintable_get(graph->vertices, vk, len ,&entry);
                        if(entry.key == vk){
                                /*Adding vertex to vlist.*/
                                r2_list_insert_at_back(graph->vlist, vertex); 
                                struct r2_listnode *rear = r2_listnode_last(graph->vlist);
                                /*Checking if insertion was successful.*/
                                if(rear != NULL && rear->data == vertex){
                                        ++graph->nvertices;
                                        /*Note position in list.*/
                                        vertex->pos = rear;
                                }else
                                        r2_free_vertex(graph, vertex);   
                        }     
                }         
        }
        return graph;
}


/**
 * @brief                       Finds a vertex in a graph.
 * 
 * @param graph                 Graph.
 * @param vk                    Vertex key.
 * @param len                   Key length.
 * @return struct r2_vertex*    Returns a vertex, else null_vertex.
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
 * @param len                   Length.
 * @return struct r2_graph*     Returns graph.
 */
struct r2_graph* r2_graph_del_vertex(struct r2_graph *graph, r2_uc *vkey, r2_uint64 len)
{
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, vkey, len); 
        if(vertex != NULL)
                r2_free_vertex(graph, vertex);
        return graph;
}

/**
 * @brief                       Adds an edge to graph. 
 *                              The endpoints (vertices) of the edge doesn't have to exist. We create the endpoints(vertices)
 *                              and subsequently the edge and add it to the graph.
 *                              
 * 
 * @param graph                 Graph.
 * @param src                   Origin.
 * @param slen                  Length.
 * @param dest                  Destination.
 * @param dlen                  Length.
 * @param weight                Edge weight.
 * @return struct r2_graph*     Returns graph.
 */
struct r2_graph* r2_graph_add_edge(struct r2_graph *graph, r2_uc *src, r2_uint64 slen,  r2_uc* dest, r2_uint64 dlen, r2_ldbl weight)
{
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
                edge = r2_create_edge();  
                if(edge != NULL){
                        /*Getting or adding vertices to graph*/
                        vertex[0] = r2_graph_get_vertex(graph, src,  slen); 
                        vertex[1] = r2_graph_get_vertex(graph, dest, dlen);
                        for(r2_uint64 i = 0; i < 2; ++i)
                                if(vertex[i] == NULL){
                                        graph     = r2_graph_add_vertex(graph, keys[i], len[i]);
                                        vertex[i] = r2_graph_get_vertex(graph, keys[i], len[i]);
                                        created_vertex[i] = vertex[i];
                                        if(vertex[i] == NULL)
                                                goto CLEANUP;
                                }
                        
                        /*Adding edge*/
                        edge->src  = vertex[0]; 
                        edge->dest = vertex[1];
                        edge->weight = weight; 
                        r2_robintable_put(vertex[0]->edges, dest, edge, dlen);
                        ++vertex[0]->nedges;
                        /*Checking if we successfully inserted the edge*/
                        struct r2_entry entry = {.data = NULL, .key = NULL, .length = 0};
                        r2_robintable_get(vertex[0]->edges, dest, dlen, &entry); 
                        if(entry.key == NULL)
                                goto CLEANUP; 

                        /*Adding edge to elist*/
                        r2_list_insert_at_back(vertex[0]->elist, edge);
                        struct r2_listnode *rear = r2_listnode_last(vertex[0]->elist);
                        edge->pos[0] = rear;
                        
                        /*Checking if insertion was successfully*/
                        if(rear == NULL || rear->data != edge)
                                goto CLEANUP; 
                        
                        r2_list_insert_at_back(graph->elist, edge);
                        ++graph->nedges;
                        rear = r2_listnode_last(graph->elist);
                        edge->pos[1] = rear;

                        /*Checking if insertion was successfully*/
                        if(rear == NULL || rear->data != edge)
                                goto CLEANUP; 
                        
                                
                        /*Add vertex metadata*/
                        r2_list_insert_at_back(vertex[0]->out, vertex[1]); 
                        rear = r2_listnode_last(vertex[0]->out);
                        edge->pos[2] = rear;

                        /*Checking if insertion was successfully*/
                        if(rear == NULL || rear->data != vertex[1])
                                 goto CLEANUP; 
                        
                        r2_list_insert_at_back(vertex[1]->in, vertex[0]); 
                        rear = r2_listnode_last(vertex[1]->in);
                        edge->pos[3] = rear;
                        /*Checking if insertion was successfully*/
                        if(rear == NULL || rear->data != vertex[0])
                                goto CLEANUP; 
                }else 
                        goto CLEANUP; 
                
                goto SUCCESS;
        }else goto SUCCESS;

        CLEANUP: 
                r2_free_edge(graph, edge); 
                for(r2_uint64 i = 0; i < 2; ++i)
                        if(created_vertex[i] != NULL)
                                r2_free_vertex(graph, created_vertex[i]);

        SUCCESS:
                return graph; 
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
 * @return struct r2_edge*      Returns graph.
 */
struct r2_graph*  r2_graph_del_edge(struct r2_graph *graph,  r2_uc *src, r2_uint64 slen,  r2_uc *dest, r2_uint64 dlen)
{
        struct r2_edge *edge  = r2_graph_get_edge(graph, src, slen, dest, dlen); 
        if(edge != NULL)
                r2_free_edge(graph, edge);

        return graph;
}

/**
 * @brief               Adds attribute.
 * 
 * @param graph         Graph.
 * @param key           Key.
 * @param data          Data.
 * @param len           Length.
 */
void r2_graph_add_attributes(struct r2_graph *graph, r2_uc *key, void *data, r2_uint64 len)
{
        r2_robintable_put(graph->gat, key, data, len);
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
 */
void r2_graph_del_attributes(struct r2_graph *graph, r2_uc *key, r2_uint64 len)
{
        r2_robintable_del(graph->gat, key, len);
}

/**
 * @brief                       Creates an empty vertex.
 * 
 * @return struct r2_vertex*    Returns an empty vertex, else NULL.
 */
static struct r2_vertex* r2_create_vertex(r2_cmp cmp)
{
        struct r2_vertex *vertex = malloc(sizeof(struct r2_vertex));
        if(vertex != NULL){
                vertex->vkey            = NULL; 
                vertex->len             = 0; 
                vertex->pos             = NULL; 
                vertex->in              = r2_create_list(NULL, NULL, NULL);  
                vertex->out             = r2_create_list(NULL, NULL, NULL); 
                vertex->elist           = r2_create_list(NULL, NULL, NULL); 
                vertex->edges           = r2_create_robintable(1, 1, 0, 0, .75,cmp, NULL, NULL, NULL, NULL, free);
                vertex->nedges          = 0;
                vertex->vat             = r2_create_robintable(1, 1, 0, 0, .75,NULL, NULL, NULL, NULL, NULL, NULL);
                if(vertex->in == NULL || vertex->out == NULL || vertex->elist == NULL || vertex->vat == NULL   || vertex->edges == NULL){
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

                        if(vertex->vat != NULL)
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
 */
void r2_vertex_add_attributes(struct r2_vertex *vertex, r2_uc *key, void *data, r2_uint64 len, r2_cmp cmp)
{
        vertex->vat->kcmp = cmp; 
        r2_robintable_put(vertex->vat, key, data, len);
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
 */
void r2_vertex_del_attributes(struct r2_vertex *vertex, r2_uc *key, r2_uint64 len, r2_cmp cmp)
{
        vertex->vat->kcmp = cmp;
        r2_robintable_del(vertex->vat, key, len);
}

/**
 * @brief                       Creates an empty edge.
 * 
 * @return struct r2_edge*      Returns an empty edge, else NULL.
 */
static struct r2_edge*  r2_create_edge()
{
        struct r2_edge *edge = malloc(sizeof(struct r2_edge)); 
        if(edge != NULL){
                edge->src         = NULL; 
                edge->dest        = NULL; 
                for(r2_uint64 i = 0; i < 4; ++i)
                        edge->pos[i] = NULL; 
                edge->eat         = r2_create_robintable(1, 1, 0, 0, .75,NULL, NULL, NULL, NULL, NULL, NULL); 
                edge->weight      = 0; 
                /**
                 * All metadata is important. Failure to allocate requisite memory is considered
                 * a failure to create edge.
                 */
                if(edge->eat == NULL){
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
 */
void r2_edge_add_attributes(struct r2_edge *edge, r2_uc *key, void *data, r2_uint64 len, r2_cmp cmp)
{
        edge->eat->kcmp = cmp;
        r2_robintable_put(edge->eat, key, data, len); 
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
 */
void r2_edge_del_attributes(struct r2_edge *edge, r2_uc *key, r2_uint64 len, r2_cmp cmp)
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
        
        r2_destroy_robintable(edge->eat); 
        if(src != NULL){
                r2_robintable_del(src->edges, dest->vkey, dest->len);
                --src->nedges;
        }


}



/********************************************Graph Algorithms*************************************************/

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
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
       /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || queue == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head   = NULL;
        struct r2_vertex   *vertex = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;   
        r2_queue_enqueue(queue, source);
        if(r2_queue_empty(queue) == TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }

        state[count] = GREY; 
        r2_robintable_put(processed, source->vkey, &state[count], source->len);   
        entry.key = entry.data = NULL;
        r2_robintable_get(processed, source->vkey, source->len, &entry);
        if(entry.key == NULL){
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
                        vertex = head->data; 
                        entry.key = entry.data = NULL;
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is white.
                         */
                        r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                r2_robintable_put(processed, vertex->vkey, vstate, vertex->len);
                                r2_queue_enqueue(queue, vertex);
                                if(r2_queue_rear(queue)->data != vertex){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }
                entry.key = entry.data = NULL;
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
}


/**
 * @brief               Performs depth first search on graph.
 * 
 * @param graph         Graph.
 * @param vertex        Source.
 * @param action        Action performed on each vertex.
 * @param arg           Argument to be passed to action.
 */
void r2_graph_dfs(struct r2_graph *graph, struct r2_vertex *source, r2_act action, void *arg)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);

       /**
        * Every vertex has a state. We keep track of this state by using an array and a hash table. 
        * The initial state of a vertex is always WHITE. As we perform the breadth first search 
        * the states change from WHITE => GREY => BLACK.
        */
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(state == NULL || processed == NULL || stack == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;

        state[count] = GREY;
        r2_robintable_put(processed, source->vkey, &state[count], source->len);
        entry.key = entry.data = NULL;
        r2_robintable_get(processed, source->vkey, source->len, &entry);
        if(entry.key == NULL){
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
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data; 
                        if(vstate == NULL){
                                /*Perform action*/
                                if(action != NULL)
                                        action(dest, arg);
                                
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                r2_robintable_put(processed, dest->vkey, vstate, dest->len);
                                entry.key = entry.data = NULL;
                                r2_robintable_get(processed, dest->vkey,dest->len, &entry);
                                if(entry.key == NULL){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                r2_arrstack_push(stack, edge->pos[0]);
                                if(r2_arrstack_top(stack) != edge->pos[0]){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                source    = dest;
                                head =  r2_listnode_first(source->elist);
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
                        }
                        r2_arrstack_pop(stack);
                }    
        }while(head != NULL);
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
 * @brief                       Performs a BFS on graph.
 *                              Performs a BFS search on graph. 
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @return struct r2_bfstree*   Returns BFS tree, else NULL.
 */
struct r2_bfstree* r2_graph_bfs_tree(struct r2_graph *graph, struct r2_vertex *source)
{
        r2_int16 FAILED = FALSE;
        struct r2_queue *queue            = r2_create_queue(NULL, NULL, NULL);
        struct r2_bfstree   *bfs          = malloc(sizeof(struct r2_bfstree));
        struct r2_bfsnode   *tree         = malloc(sizeof(struct r2_bfsnode) * graph->nvertices);
        struct r2_robintable *positions   = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_robintable *processed   = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        if(processed == NULL || queue == NULL || tree == NULL || bfs == NULL || positions == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        /**
         * Initializing BFS tree.
         */
        bfs->ncount = 0; 
        bfs->tree   = tree;
        bfs->positions = positions;

        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_entry  entry     = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        while(head != NULL){
                tree[count].vertex = NULL;
                tree[count].pos    = tree[count].dist = 0;  
                tree[count].state  = WHITE;
                tree[count].parent = -1; 
                tree[count].children.start = tree[count].children.end   = 0;  
                head = head->next; 
                count++; 
        }

        struct r2_bfsnode *root  = NULL;
        /*Initializing queue*/
        count = 0;
        tree[count].vertex = source;
        tree[count].state  = GREY;
        r2_queue_enqueue(queue, &tree[count]);
        if(r2_queue_empty(queue) == TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }
        r2_robintable_put(positions, source->vkey, &tree[count], source->len);
        entry.key = entry.data = NULL;
        r2_robintable_get(positions, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_vertex   *dest = NULL; 
        do{
                root = r2_queue_front(queue)->data;
                /*Current source*/
                source = root->vertex;
                /*Processing source edge list*/
                head   = r2_listnode_first(source->out);
                while(head != NULL){
                        dest = head->data; 
                        /**
                         * If dest hasn't begun processing then it's not a part of the tree.
                         * We can safely begin processing.
                         */
                        entry.key = entry.data = NULL;
                        r2_robintable_get(positions, dest->vkey, dest->len, &entry); 
                        if(entry.key == NULL){
                                ++count;
                                if(root->children.start == root->children.end){ 
                                        root->children.start = count; 
                                        root->children.end   = count;
                                }
                                tree[count].vertex = dest; 
                                tree[count].pos    = count; 
                                tree[count].state  = GREY;      
                                tree[count].parent = root->pos;
                                tree[count].dist   = root->dist + 1; 
                                ++root->children.end;
                                r2_queue_enqueue(queue, &tree[count]);
                                if(r2_queue_rear(queue)->data != &tree[count]){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                                r2_robintable_put(positions, dest->vkey, &tree[count], dest->len);   
                                entry.key = entry.data = NULL;
                                r2_robintable_get(positions, dest->vkey, dest->len, &entry);
                                if(entry.key == NULL){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }
                bfs->ncount++;
                root->state = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE && count != graph->nvertices);

        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
         
                if(processed != NULL) 
                        r2_destroy_robintable(processed);

                if(tree != NULL && FAILED == TRUE)
                        free(tree);

                if(FAILED == TRUE && bfs != NULL){
                        free(bfs);
                        bfs = NULL;
                }

                if(FAILED == TRUE && positions != NULL)
                        r2_destroy_robintable(positions);
                        
        assert(FAILED == FALSE);   
        return bfs;
}


/**
 * @brief                       Performs a DFS on graph.
 *                              Performs a DFS search on graph. The size of the array is the number 
 *                              of vertices in the graph. 
 * 
 * @param graph                 Graph.
 * @param source                Source.
 * @return struct r2_dfstree*   Returns DFS tree, else NULL.
 */
struct r2_dfstree* r2_graph_dfs_tree(struct r2_graph *graph, struct r2_vertex *source)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack   *stack       = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_dfsnode    *tree        = malloc(sizeof(struct r2_dfsnode) * graph->nvertices);
        struct r2_robintable *positions   = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dfstree    *dfs         = malloc(sizeof(struct r2_dfstree));

        if(stack == NULL || tree == NULL || positions == NULL || dfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        /**
         * Initializing DFS tree
         * 
         */
        dfs->tree = tree; 
        dfs->ncount = 0; 
        dfs->positions = positions;

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        while(head != NULL){
                dest = head->data; 
                tree[count].state  = WHITE;
                tree[count].vertex = NULL; 
                tree[count].start  = tree[count].end = 0; 
                tree[count].parent = -1; 
                tree[count].dist   = 0;
                tree[count].pos    = count;
                head = head->next; 
                count++; 
        }
        
        dest  = NULL; 
        count = 0;

        /*Initializng root*/
        struct r2_dfsnode *root = &tree[count];
        root->vertex = source; 
        root->start  = count;
        root->state  = GREY;
        r2_robintable_put(positions, source->vkey, &tree[count], source->len);
        entry.key = entry.data = NULL;
        r2_robintable_get(positions, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }


        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);
        do{
                r2_robintable_get(positions, source->vkey, source->len, &entry); 
                root = entry.data;
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        entry.key = entry.data = NULL;
                        r2_robintable_get(positions, dest->vkey, dest->len, &entry); 
                        if(entry.key == NULL){
                                ++count; 
                                tree[count].vertex = dest;
                                tree[count].state  = GREY;
                                tree[count].pos    = count;
                                tree[count].start  = count;
                                tree[count].parent = root->pos;
                                tree[count].dist   = root->dist + 1;
                                r2_robintable_put(positions, dest->vkey, &tree[count], dest->len);
                                entry.key = entry.data = NULL;
                                r2_robintable_get(positions, dest->vkey, dest->len, &entry);
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
                                head = r2_listnode_first(source->elist);
                                if(head == NULL)
                                        break;
                                     
                                r2_robintable_get(positions, source->vkey, source->len, &entry); 
                                root = entry.data;
                                continue;
                                
                        }
                        head = head->next; 
                }

                if(head == NULL){
                        dfs->ncount++;
                        r2_robintable_get(positions, source->vkey, source->len, &entry); 
                        root = entry.data;
                        root->state = BLACK;
                        root->end = count + 1;
                        head      = r2_arrstack_top(stack);
                        if(head != NULL){
                                edge      = head->data; 
                                source    = edge->src;
                        }
                        r2_arrstack_pop(stack);
                }    
        }while(head != NULL);
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(FAILED == TRUE && positions != NULL)
                        r2_destroy_robintable(positions);
                if(FAILED == TRUE && tree != NULL)
                        free(tree); 
                if(FAILED == TRUE && dfs != NULL){
                        free(dfs); 
                        dfs = NULL;
                }

        assert(FAILED == FALSE);    
        return dfs;
}

/**
 * @brief                       Destroys BFS tree.
 * 
 * @param bfs                   BFS Tree.
 * @return struct r2_bfstree*   Returns NULL when BFS tree is destroyed properly.
 */
struct r2_bfstree* r2_destroy_bfs_tree(struct r2_bfstree *bfs)
{
        r2_destroy_robintable(bfs->positions);
        free(bfs->tree);
        free(bfs);
        return NULL;
}

/**
 * @brief                       Destroys a DFS tree.
 * 
 * @param  dfs                  DFS Tree.
 * @return struct r2_dfstree*   Returns NULL when DFS tree is destroyed properly.
 */
struct r2_dfstree* r2_destroy_dfs_tree(struct r2_dfstree *dfs)
{
        r2_destroy_robintable(dfs->positions);
        free(dfs->tree);
        free(dfs); 
        return NULL; 
}

/**
 * @brief                      Returns the path in graph if it's exists.
 * 
 * @param graph                Graph.
 * @param src                  Source.
 * @param dest                 Dest.
 * @return struct r2_list*     Return list representing the shortest path regardless of weight, else NULL.
 */
struct r2_list* r2_graph_has_path(struct r2_graph *graph, struct r2_vertex *src, struct r2_vertex *dest)
{
        r2_int16 FAILED = FALSE;
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_robintable *parents   = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_list       *paths     = r2_create_list(NULL, NULL, NULL);

        if(state == NULL || processed == NULL || queue == NULL || parents  == NULL || paths == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }
                
        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *vertex = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        while(head != NULL){
                vertex = head->data; 
                r2_robintable_put(parents, vertex->vkey, NULL, vertex->len);
                entry.key = entry.data = NULL;
                r2_robintable_get(parents, vertex->vkey, vertex->len, &entry);
                if(entry.key == NULL){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }
                head = head->next; 
                count++; 
        }

        struct r2_vertex *source = src;
        r2_queue_enqueue(queue, source);
        if(r2_queue_empty(queue) == TRUE){
                FAILED = TRUE;
                goto CLEANUP;
        }
        count = 0;
        state[count] = GREY; 
       r2_robintable_put(processed, source->vkey, &state[count], source->len);   
        entry.key = entry.data = NULL;
        r2_robintable_get(processed, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }
        
        r2_uint16 *vstate = NULL;
        do{
                source = r2_queue_front(queue)->data;
                head   = r2_listnode_first(source->out);
                while(head != NULL){
                        vertex = head->data; 
                        /**
                         * Checking to see if the vertex is being processed. 
                         * If the vertex doesn't exist in the hash table then processing hasn't 
                         * started. We assume a vertex that hasn't start processing is white.
                         */
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                        vstate = entry.data;
                        if(vstate == NULL){
                                vstate    = &state[++count];
                                *vstate   = GREY; 
                                r2_robintable_put(processed, vertex->vkey, vstate, vertex->len);
                                r2_robintable_put(parents, vertex->vkey, source, vertex->len);
                               r2_queue_enqueue(queue, vertex);
                                if(r2_queue_rear(queue)->data != vertex){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }        
                        head = head->next; 
                }
                entry.key = entry.data = NULL;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                *vstate = BLACK;
                r2_queue_dequeue(queue);
        }while(r2_queue_empty(queue) != TRUE && count != graph->nvertices);

        struct r2_vertex *parent = NULL;
        do{
                r2_list_insert_at_front(paths, dest);
                if(r2_listnode_first(paths) == NULL ||r2_listnode_first(paths)->data != dest){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }

                r2_robintable_get(parents, dest->vkey, dest->len, &entry);
                parent = entry.data;
                if(parent == NULL){
                        r2_destroy_list(paths);
                        paths = NULL;
                        goto CLEANUP;
                }
                dest = parent;
        }while(dest != src);
        r2_list_insert_at_front(paths, src);
        CLEANUP:
                if(queue != NULL)
                        r2_destroy_queue(queue); 
                if(state != NULL)
                        free(state);
                if(processed != NULL) 
                        r2_destroy_robintable(processed);
                if(parents != NULL)
                        r2_destroy_robintable(parents);
                
                if(FAILED == TRUE && paths != NULL)
                        r2_destroy_list(paths);
        
        assert(FAILED == FALSE);
        return paths;
}

/**
 * @brief                      Returns the path in graph if it's exists.
 * 
 * @param graph                Graph.
 * @param src                  Source.
 * @param dest                 Dest.
 * @return struct r2_list*     Return list representing path, else NULL.
 */
struct r2_list* r2_graph_bfs_has_path_tree(struct r2_bfstree *tree, struct r2_vertex *src, struct r2_vertex *dest)
{
        struct r2_list *path = r2_create_list(NULL, NULL, NULL);
        if(path != NULL){
                struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
                r2_robintable_get(tree->positions, dest->vkey, dest->len, &entry);
                if(entry.key == NULL){
                        path = r2_destroy_list(path);
                        goto CLEANUP; 
                }
                
                struct r2_vertex *parent = NULL; 
                r2_int64 pos = ((struct r2_bfsnode *)entry.data)->pos; 
                do{
                        r2_list_insert_at_front(path, dest);
                        pos  = tree->tree[pos].parent;
                        if(pos != -1){
                                parent = tree->tree[pos].vertex;
                        }else{
                                path = r2_destroy_list(path);
                                goto CLEANUP;   
                        }
                        
                      dest = parent;
                }while(parent != src);
                r2_list_insert_at_front(path, src);
        }
        CLEANUP: 
                return path; 
}

/**
 * @brief                      Returns the path in graph if it's exists.
 * 
 * @param graph                Graph.
 * @param src                  Source.
 * @param dest                 Dest.
 * @return struct r2_list*     Return list representing path, else NULL.
 */
struct r2_list* r2_graph_dfs_has_path_tree(struct r2_dfstree *tree, struct r2_vertex *src, struct r2_vertex *dest)
{
        struct r2_list *path = r2_create_list(NULL, NULL, NULL);
        if(path != NULL){
                struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
                r2_robintable_get(tree->positions, dest->vkey, dest->len, &entry);
                if(entry.key == NULL){
                        path = r2_destroy_list(path);
                        goto CLEANUP; 
                }
                
                struct r2_vertex *parent = NULL; 
                r2_int64 pos = ((struct r2_dfsnode *)entry.data)->pos; 
                do{
                       r2_list_insert_at_front(path, dest);
                        pos  = tree->tree[pos].parent;
                        if(pos != -1){
                                parent = tree->tree[pos].vertex;
                        }else{
                                path = r2_destroy_list(path);
                                goto CLEANUP;      
                        }
                        
                      dest = parent;
                }while(parent != src);
                r2_list_insert_at_front(path, src);
        }
        CLEANUP: 
                return path; 
}

/**
 * @brief                       Returns all the children of parent. 
 *              
 * @param bfs                   BFS tree.
 * @param parent                Parent. 
 * @return struct r2_list*      Returns list containing all children. 
 */
struct r2_list* r2_graph_bfs_tree_children(struct r2_bfstree *bfs, struct r2_vertex *parent)
{
        struct r2_list *children  = r2_create_list(NULL, NULL, NULL); 
        if(children != NULL){
                struct r2_vertex *child = NULL; 
                struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
                r2_robintable_get(bfs->positions, parent->vkey, parent->len, &entry);
                if(entry.key != NULL){
                        struct r2_bfsnode *root = entry.data;
                        r2_uint64 pos = root->pos; 
                        for(r2_uint64 i = bfs->tree[pos].children.start; i < bfs->tree[pos].children.end; ++i){
                                child = bfs->tree[i].vertex; 
                                r2_list_insert_at_back(children, child); 
                                if(r2_listnode_last(children) == NULL || r2_listnode_last(children)->data != child){
                                        children =  r2_destroy_list(children);
                                        break;
                                }
                        }
                }else
                        children = r2_destroy_list(children);        
        }

        return children; 
}


/**
 * @brief                       Returns all children of parent. 
 *      
 * @param dfs                   DFS tree.
 * @param parent                Parent. 
 * @return struct r2_list*      Returns a list containing all children of parent, else NULL. 
 */
struct r2_list* r2_graph_dfs_tree_children(struct r2_dfstree *dfs, struct r2_vertex *parent)
{
        struct r2_list *children  = r2_create_list(NULL, NULL, NULL); 
        if(children != NULL){
                struct r2_vertex *child = NULL;
                struct r2_entry entry   = {.key = NULL, .data = NULL, .length = 0}; 
                r2_robintable_get(dfs->positions, parent->vkey, parent->len, &entry);
                struct r2_dfsnode *root = entry.data; 
                r2_uint64 pos   = root->pos;
                r2_uint64 start = dfs->tree[pos].start; 
                r2_uint64 end   = dfs->tree[pos].end;
                pos++;
                for(;start < end && pos < dfs->ncount; ++start, ++pos){
                        child = dfs->tree[pos].vertex;
                        if(dfs->tree[pos].parent == root->pos){
                                r2_list_insert_at_back(children, child); 
                                if(r2_listnode_last(children) == NULL || r2_listnode_last(children)->data != child){
                                        children = r2_destroy_list(children); 
                                        break;
                                }
                        }
                }
        }

        return children;    
}

/**
 * @brief                       Returns the root of the BFS tree.
 * 
 * @param bfs                   BFS tree.
 * @return struct r2_bfsnode*   Returns the root. 
 */     
struct r2_bfsnode* r2_graph_bfsnode_first(struct r2_bfstree *bfs)
{
        return &bfs->tree[0]; 
}


/**
 * @brief                       Returns the root of the DFS tree. 
 * 
 * @param dfs                   DFS tree. 
 * @return struct r2_dfsnode*   Returns the root.
 */
struct r2_dfsnode* r2_graph_dfsnode_first(struct r2_dfstree *dfs)
{
        return &dfs->tree[0];
}

/**
 * @brief                       Returns the previous node in a bfs traversal of the graph.
 * 
 * @param tree                  Tree.
 * @param root                  Root.
 * @return struct r2_bfsnode*   Returns previous bfsnode, else NULL.
 */
struct r2_bfsnode* r2_graph_bfsnode_prev(struct r2_bfstree *tree, struct r2_bfsnode *root)
{
        if(&tree->tree[0] == root)
                return NULL;     
        return --root; 
}

/**
 * @brief                       Returns the previous node in a dfs traversal of the graph.
 * 
 * @param tree                  Tree.
 * @param root                  Root.
 * @return struct r2_dfsnode*   Returns previous dfsnode, else NULL. 
 */
struct r2_dfsnode* r2_graph_dfsnode_prev(struct r2_dfstree *tree, struct r2_dfsnode *root)
{
        if(&tree->tree[0] == root)
                return NULL;    
        return --root; 
}


/**
 * @brief                       Returns the next node in a dfs traversal of the graph.
 * 
 * @param tree                  Tree.
 * @param root                  Root.
 * @return struct r2_dfsnode*   Returns next dfsnode, else NULL. 
 */
struct r2_dfsnode* r2_graph_dfsnode_next(struct r2_dfstree *tree, struct r2_dfsnode *root)
{
        if(&tree->tree[tree->ncount -1] == root)
                return NULL; 
        return ++root; 
}


/**
 * @brief                       Returns the next node in a bfs traversal of the graph.
 * 
 * @param tree                  Tree.
 * @param root                  Root.
 * @return struct r2_bfsnode*   Returns next bfsnode, else NULL.
 */
struct r2_bfsnode* r2_graph_bfsnode_next(struct r2_bfstree *tree, struct r2_bfsnode *root)
{
        if(&tree->tree[tree->ncount -1] == root)
                return NULL; 
        return ++root; 
}

/**
 * @brief                               Finds the connected components in the graph.
 * 
 * @param graph                         Graph.
 * @return struct r2_components*        Returns all the connected components in the graph, else NULL. 
 */
struct r2_components* r2_graph_connected_components(struct r2_graph *graph)
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

        head  = r2_listnode_first(graph->vlist); 
        count = 0;
        while(head != NULL){
                src = head->data; 
                r2_robintable_get(processed, src->vkey, src->len, &entry);
                vstate = entry.data;
                if(*vstate == WHITE){
                        *vstate = GREY;
                        forest->cc[count] = r2_graph_dfs_tree_components(graph, src,  processed);
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

                if(FAILED == TRUE && forest != NULL)
                        forest = r2_graph_destroy_components(forest);

        assert(FAILED == FAILED);
        return forest;
}

/**
 * @brief                               Helper function for connected components.
 * 
 * @param graph                         Graph.
 * @param source                        Source.
 * @param processed                     Hash table containing state of each vertices in the graph.
 * @return struct r2_dfstree*           Returns the dfs tree for component, else NULL.
 */
static struct r2_dfstree* r2_graph_dfs_tree_components(struct r2_graph *graph, struct r2_vertex *source,  struct r2_robintable *processed)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack   *stack       = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_dfsnode    *tree        = malloc(sizeof(struct r2_dfsnode) * graph->nvertices);
        struct r2_robintable *positions   = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_dfstree    *dfs         = malloc(sizeof(struct r2_dfstree));
        if(stack == NULL || tree == NULL || positions == NULL || dfs == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }


        /**
         * Initializing DFS tree
         * 
         */
        dfs->tree      = tree; 
        dfs->ncount    = 0; 
        dfs->positions = positions;

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        
        
        r2_uint64 count   = 0;
        while(head != NULL){
                dest = head->data; 
                tree[count].state  = WHITE;
                tree[count].vertex = NULL; 
                tree[count].start  = tree[count].end = 0; 
                tree[count].parent = -1; 
                tree[count].dist   = 0;
                tree[count].pos    = count;
                head = head->next; 
                count++; 
        }

        dest  = NULL; 
        count = 0;

        /*Initializng root*/
        struct r2_dfsnode *root = &tree[count];
        root->vertex = source; 
        root->start  = count;
        root->state  = GREY;
        r2_robintable_put(positions, source->vkey, &tree[count], source->len);
        entry.key = entry.data = NULL;
        r2_robintable_get(positions, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }


        if(source->elist != NULL)
                head  = r2_listnode_first(source->elist);
        r2_uint16 *vstate = NULL;
         do{
                r2_robintable_get(positions, source->vkey, source->len, &entry); 
                root = entry.data;
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        vstate = entry.data; 
                        if(*vstate == WHITE){
                                ++count; 
                                tree[count].vertex = dest;
                                tree[count].state  = GREY;
                                tree[count].pos    = count;
                                tree[count].start  = count;
                                tree[count].parent = root->pos;
                                tree[count].dist   = root->dist + 1;
                                r2_robintable_put(positions, dest->vkey, &tree[count], dest->len);
                                entry.key = entry.data = NULL;
                                *vstate = GREY;
                                r2_robintable_get(positions, dest->vkey, dest->len, &entry);
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
                                head = r2_listnode_first(source->elist);
                                if(head == NULL)
                                        break;

                                r2_robintable_get(positions, source->vkey, source->len, &entry); 
                                root = entry.data;
                                continue;
                                
                        }
                        head = head->next; 
                }

                if(head == NULL){
                        dfs->ncount++;
                        r2_robintable_get(positions, source->vkey, source->len, &entry); 
                        root = entry.data;
                        root->state = BLACK;
                        root->end = count + 1;
                        r2_robintable_get(processed, source->vkey, source->len, &entry);
                        vstate = entry.data; 
                        *vstate = BLACK;
                        head      = r2_arrstack_top(stack);
                        if(head != NULL){
                                edge      = head->data; 
                                source    = edge->src;
                        }
                        r2_arrstack_pop(stack);
                }    
        }while(head != NULL);
        CLEANUP:
                if(stack != NULL)
                        r2_arrstack_destroy_stack(stack);
                if(FAILED == TRUE && positions != NULL)
                        r2_destroy_robintable(positions);
                if(FAILED == TRUE && tree != NULL)
                        free(tree); 
                if(FAILED == TRUE && dfs != NULL){
                        free(dfs); 
                        dfs = NULL;
                }

        assert(FAILED == FALSE);    
        return dfs;
}

/**
 * @brief                               Frees memory used by connected components.
 * 
 * @param components                    Components.
 * @return struct r2_components*        Returns NULL whenever properly destroyed
 */
struct r2_components* r2_graph_destroy_components(struct r2_components *components)
{
        for(r2_uint64 i = 0; i < components->ncount; ++i)
                r2_destroy_dfs_tree(components->cc[i]);
        
        if(components->transpose != NULL)
                r2_destroy_graph(components->transpose);
        free(components->cc); 
        free(components);
        return NULL; 
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
 * @brief                               Determine if a graph is bipartite and returns the sets representing the bipartite graph.
 * 
 * @param graph                         Graph.
 * @return struct r2_bipartite*         Returns bipartite graph, else NULL. 
 */
struct r2_bipartite* r2_graph_is_bipartite(struct r2_graph *graph)
{
        r2_uint16 FAILED = FALSE;
        struct r2_bipartite *sets = malloc(sizeof(struct r2_bipartite));
        struct r2_queue* queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_robintable *groups =  r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        struct r2_robintable *processed =  r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        r2_uint16 *state  = malloc(sizeof(r2_int16) * graph->nvertices);
        r2_uint16 *colors = malloc(sizeof(r2_int16) * graph->nvertices);
        if(sets != NULL){
                sets->sets[0]   = r2_create_list(NULL, NULL, NULL); 
                sets->sets[1]   = r2_create_list(NULL, NULL, NULL);
                if(sets->sets[0] == NULL || sets->sets[1] == NULL){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }
        }

        if(colors == NULL || groups == NULL || state == NULL || queue == NULL || processed == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *vertex = NULL; 
        struct r2_entry  entry = {.key = NULL, .data = NULL, .length  = 0};
        struct r2_vertex *src  = NULL;
        r2_uint16 *vstate  = NULL;
        r2_uint16 *color   = NULL;
        r2_uint16 *srcolor = NULL;
        r2_uint64 count = 0;
        struct r2_list *list = NULL;
        head   = r2_listnode_first(graph->vlist); 
        while(head != NULL){
                /**
                 * Checking to see if the vertex is being processed. 
                 * If the vertex doesn't exist in the hash table then processing hasn't 
                 * started. We assume a vertex that hasn't start processing is white.
                 */
                vertex = head->data;
                entry.key = entry.data = NULL;
                r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        /*Update state of vertex*/
                        vstate    = &state[count];
                        *vstate   = GREY; 
                        r2_robintable_put(processed, vertex->vkey, vstate, vertex->len);
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                        if(entry.key == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        /*Get group*/
                        color = &colors[count];
                        *color = 0;
                        r2_robintable_put(groups, vertex->vkey, color, vertex->len);  
                        entry.key = entry.data = NULL;
                        r2_robintable_get(groups, vertex->vkey, vertex->len, &entry);
                        if(entry.key == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        r2_queue_enqueue(queue, vertex);
                        if(r2_queue_rear(queue)->data != vertex){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        
                        
                        r2_list_insert_at_back(sets->sets[*color], vertex); 
                        if(r2_listnode_last(list)->data != vertex){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                        ++count;
                         
                        /*Performing partitioning*/
                        do{
                                src    = r2_queue_front(queue)->data;
                                entry.key = entry.data = NULL;
                                r2_robintable_get(groups, src->vkey, src->len, &entry);
                                srcolor  = entry.data; 
                                struct r2_listnode *first = r2_listnode_first(src->out);
                                while(first != NULL){
                                        vertex = first->data;
                                        entry.key = entry.data = NULL;
                                        r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                                        vstate = entry.data;
                                        r2_robintable_get(groups, vertex->vkey, vertex->len, &entry);
                                        color  = entry.data; 
                                        if(vstate == NULL){
                                                vstate    = &state[count];
                                                *vstate   = GREY; 
                                                r2_robintable_put(processed, vertex->vkey, vstate, vertex->len);
                                                entry.key = entry.data = NULL;
                                                r2_robintable_get(processed, vertex->vkey, vertex->len, &entry);
                                                if(entry.key == NULL){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }  

                                                color     = &colors[count];
                                                *color    = !(*srcolor);
                                                entry.key = entry.data = NULL;
                                                r2_robintable_put(groups, vertex->vkey, color, vertex->len);  
                                                r2_robintable_get(groups, vertex->vkey, vertex->len, &entry);
                                                if(entry.key == NULL){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                } 

                                                r2_list_insert_at_back(sets->sets[*color], vertex);                                        
                                                if(r2_listnode_last(list)->data != vertex){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                                r2_queue_enqueue(queue, vertex);
                                                if(r2_queue_rear(queue)->data != vertex){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                } 

                                                ++count;      
                                        }else if(*srcolor == *color){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                                                                
                                        first = first->next;
                                }

                                r2_robintable_get(processed, src->vkey, src->len, &entry);
                                vstate = entry.data;
                                *vstate = BLACK;
                                r2_queue_dequeue(queue);
                        }while(r2_queue_empty(queue) != TRUE);
                        
                }
                head = head->next;
        }

        CLEANUP:
                if(state != NULL)
                        free(state);
                
                if(colors != NULL)
                        free(colors);
                
                if(groups != NULL)
                        r2_destroy_robintable(groups); 

                if(processed != NULL)
                        r2_destroy_robintable(processed); 

                if(queue  != NULL)
                        r2_destroy_queue(queue);
                if(FAILED == TRUE && sets != NULL)
                       sets =  r2_graph_destroy_bipartite(sets);


        return sets;
}

/**
 * @brief                               Releases memory used by bipartite graph.
 * 
 * @param bipartite                     Bipartite.
 * @return struct r2_bipartitie*        Returns NULL whenever memory is released properly.
 */
struct r2_bipartite* r2_graph_destroy_bipartite(struct r2_bipartite *bipartite)
{
        r2_destroy_list(bipartite->sets[0]);
        r2_destroy_list(bipartite->sets[1]);
        free(bipartite); 
        return NULL;
}

/**
 * @brief               Checks with a graph has a cycle.
 * 
 * @param graph         Graph.
 * @return r2_int16     Returns TRUE if graph has cycle, else FALSE.
 */
r2_int16 r2_graph_has_cycle(struct r2_graph *graph)
{
        r2_int16 CYCLE = FALSE;
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
        
        struct r2_listnode *head   = r2_listnode_first(graph->vlist); 
        struct r2_listnode *vertex = NULL;
        struct r2_vertex   *source = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_edge     *edge   = NULL;
        struct r2_entry    entry   = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint16 *vstate = NULL;
        r2_uint64 count = 0;
        
   
        vertex  = r2_listnode_first(graph->vlist);
        while(vertex != NULL){
                dest   = NULL; 
                source = vertex->data;
                entry.key = entry.data = NULL;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate    = entry.data; 
                if(vstate == NULL){
                        vstate    = &state[count++];
                        *vstate   = GREY; 
                        r2_robintable_put(processed, source->vkey, vstate, source->len);
                        if(source->elist != NULL)
                                head  = r2_listnode_first(source->elist);
                        do{
                                while(head != NULL){
                                        edge = head->data;
                                        dest = edge->dest;
                                        entry.key = entry.data = NULL;
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        if(vstate == NULL){
                                                vstate    = &state[count++];
                                                *vstate   = GREY; 
                                                r2_robintable_put(processed, dest->vkey, vstate, dest->len);
                                                entry.key = entry.data = NULL;
                                                r2_robintable_get(processed, dest->vkey,dest->len, &entry);
                                                if(entry.key == NULL)
                                                        goto CLEANUP;
                                                
                                               r2_arrstack_push(stack, edge->pos[0]);
                                                if(r2_arrstack_top(stack) != edge->pos[0])
                                                        goto CLEANUP;
                                                
                                                source    = dest;
                                                head = r2_listnode_first(source->elist);
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
                                        }
                                        r2_arrstack_pop(stack);
                                }    
                        }while(head != NULL);
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
 * @brief                       Performs a topological sort on graph.
 * 
 * @param graph                 Graph.
 * @return struct r2_list*      Returns the ordering of vertices, else NULL if the graph has a cycle.
 */
struct r2_list* r2_graph_topological_sort(struct r2_graph *graph)
{
        r2_uint16 FAILED = FALSE;
        struct r2_list  *top   = r2_create_list(NULL, NULL, NULL); 
        struct r2_queue *queue = r2_create_queue(NULL, NULL, NULL);
        struct r2_robintable *indegree = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        r2_uint64 *count = malloc(sizeof(r2_uint64)*graph->nvertices);
        if(top == NULL || queue == NULL || indegree == NULL || count == NULL){
                FAILED = TRUE;
                goto CLEANUP;
        }

        
        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_queuenode *node = NULL;
        struct r2_vertex *vertex  = NULL;
        struct r2_entry entry; 
        r2_uint64 i = 0; 
        r2_uint64 nvertices = 0;
        while(head != NULL){
                vertex = head->data; 
                count[i] = vertex->in->lsize; 
                if(count[i] == 0){
                        r2_queue_enqueue(queue, vertex);
                        node = r2_queue_rear(queue);
                        if(node == NULL || node->data != vertex){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }else{
                        r2_robintable_put(indegree, vertex->vkey, &count[i], vertex->len); 
                        entry.key = entry.data =  NULL; 
                        r2_robintable_get(indegree, vertex->vkey, vertex->len, &entry); 
                        if(entry.key == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }
                }
                i++;
                head  = head->next; 
        }

        
        r2_uint64 *in = NULL; 
        while(r2_queue_empty(queue) != TRUE){
                vertex = r2_queue_front(queue)->data;
                r2_list_insert_at_back(top, vertex); 
                if(r2_listnode_last(top) == NULL || r2_listnode_last(top)->data != vertex){
                        FAILED = TRUE; 
                        goto CLEANUP;
                }
                head  = r2_listnode_first(vertex->out);
                while(head != NULL){
                        vertex = head->data; 
                        entry.key = entry.data = NULL; 
                        r2_robintable_get(indegree, vertex->vkey, vertex->len, &entry);
                        if(entry.key != NULL){
                                in = entry.data; 
                                *in = *in -1; 
                                if(*in == 0){
                                        r2_queue_enqueue(queue, vertex); 
                                        node = r2_queue_rear(queue);
                                        if(node == NULL || node->data != vertex){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }
                                        r2_robintable_del(indegree, vertex->vkey, vertex->len);
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
                
                if(FAILED == TRUE && forest != NULL)
                       forest =  r2_graph_destroy_components(forest);
        
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
                struct r2_vertex   *src    = NULL;
                struct r2_vertex   *dest   = NULL;
                struct r2_listnode *head   = r2_listnode_first(graph->elist);
                struct r2_edge     *edge   = NULL;
                while(head != NULL){
                        edge  = head->data;
                        src   = edge->src; 
                        dest  = edge->dest;
                        transpose = r2_graph_add_edge(transpose, dest->vkey, dest->len, src->vkey, src->len, edge->weight);
                        edge = NULL; 
                        edge = r2_graph_get_edge(transpose, dest->vkey, dest->len, src->vkey, src->len);
                        if(edge == NULL){
                                transpose = r2_destroy_graph(transpose); 
                                break;
                        }
                        head  = head->next;
                }
        }
        return transpose;
}



/**
 * @brief                       Performs DFS on a graph and store the vertices in either preorder, 
 *                              postorder, or reverse postorder based on the value of order.
 *                              
 *                              
 * 
 * @param graph                 Graph.
 * @param order                 Order. order == 0 => preorder, order == 1 => postorder, order == 2 => reverse postorder
 * @return struct r2_list*      Returns the list containing order of vertices.
 */
struct r2_list* r2_graph_dfs_traversals(struct r2_graph *graph, r2_uint16 order)
{
        r2_int16 FAILED = FALSE;
        struct r2_arrstack *stack = r2_arrstack_create_stack(0, NULL, NULL, NULL);
        struct r2_list *list      = r2_create_list(NULL, NULL, NULL);
        r2_uint16 *state = malloc(sizeof(r2_int16) * graph->nvertices);
        struct r2_robintable *processed = r2_create_robintable(1, 1, 0, 0, .75,graph->vcmp, NULL, NULL, NULL, NULL, NULL); 
        
        if(state == NULL || processed == NULL || stack == NULL || list == NULL){
                FAILED = TRUE; 
                goto CLEANUP; 
        }

        struct r2_listnode *head  = r2_listnode_first(graph->vlist); 
        struct r2_vertex   *dest  = NULL;
        struct r2_edge     *edge  = NULL;
        struct r2_entry    entry  = {.key = NULL, .data = NULL, .length  = 0};
        r2_uint64 count = 0;
        struct r2_vertex *source = NULL; 
        struct r2_listnode *vertex = r2_listnode_first(graph->vlist);
        r2_uint16 *vstate = NULL;

        while(vertex != NULL){
                source = vertex->data;
                entry.key = entry.data = NULL;
                r2_robintable_get(processed, source->vkey, source->len, &entry);
                vstate = entry.data;
                if(vstate == NULL){
                        state[count] = GREY;
                        r2_robintable_put(processed, source->vkey, &state[count], source->len);  
                        entry.key = entry.data = NULL;
                        r2_robintable_get(processed, source->vkey, source->len, &entry);       
                        if(entry.key == NULL){
                                FAILED = TRUE; 
                                goto CLEANUP;
                        }

                        /*preorder*/
                        if(order == 0){
                                r2_list_insert_at_back(list, source); 
                                if(r2_listnode_last(list)->data != source){
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
                                        entry.key = entry.data = NULL;
                                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                        vstate = entry.data; 
                                        if(vstate == NULL){
                                                /*preorder*/
                                                if(order == 0){
                                                        r2_list_insert_at_back(list, dest);  
                                                        if(r2_listnode_last(list)->data != dest){
                                                                FAILED = TRUE; 
                                                                goto CLEANUP;
                                                        }     
                                                }

                                                vstate    = &state[++count];
                                                *vstate   = GREY; 
                                                r2_robintable_put(processed, dest->vkey, vstate, dest->len);
                                                entry.key = entry.data = NULL;
                                                r2_robintable_get(processed, dest->vkey,dest->len, &entry);
                                                if(entry.key == NULL){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                                r2_arrstack_push(stack, edge->pos[0]);
                                                if(r2_arrstack_top(stack) != edge->pos[0]){
                                                        FAILED = TRUE; 
                                                        goto CLEANUP;
                                                }
                                                source    = dest;
                                                head = r2_listnode_first(source->elist);
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
                                                r2_list_insert_at_back(list, source); 
                                                node = *r2_listnode_last(list);
                                        }      
                                        /*reverse postorder*/
                                        else if(order == 2){
                                                r2_list_insert_at_front(list, source);
                                                node = *r2_listnode_first(list); 
                                        }
                                        if(node.data != source){
                                                FAILED = TRUE; 
                                                goto CLEANUP;
                                        }               

                                        if(head != NULL){
                                                edge      = head->data; 
                                                source    = edge->src;
                                        }
                                        r2_arrstack_pop(stack);
                                }    
                        }while(head != NULL);
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
        
        assert(FAILED == FALSE); 

        return list;                
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

        struct r2_list  *topological_order = r2_graph_dfs_traversals(graph, 2); 
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

                if(FAILED == TRUE && forest != NULL)
                        forest = r2_graph_destroy_components(forest);

                if(topological_order != NULL)
                        r2_destroy_list(topological_order);
                


        assert(FAILED == FAILED);  
        return forest;
}


/**
 * @brief               Determines if a graph is strongly connected.
 *                      A graph is strongly connected if for any two vertices x and y
 *                      there is a path between them. 
 *      
 * @param graph         Graph.
 * @return r2_uint16    Returns TRUE if graph is connected, else FALSE.
 */
r2_uint16 r2_graph_is_strong_connected(struct r2_graph *graph)
{
        r2_uint16 CONNECTED  = FALSE; 
        r2_uint64 count[2] = {0, 0};
        struct r2_dfstree *dfs = r2_graph_dfs_tree(graph, r2_listnode_first(graph->vlist)->data);
        assert(dfs != NULL);
        count[0] = dfs->ncount;
        r2_destroy_dfs_tree(dfs); 
        
        struct r2_graph *transpose = r2_graph_transpose(graph); 
        assert(transpose != NULL);
        dfs = r2_graph_dfs_tree(transpose, r2_listnode_first(transpose->vlist)->data); 
        assert(dfs != NULL);
        count[1] = dfs->ncount; 
        r2_destroy_dfs_tree(dfs); 
        r2_destroy_graph(transpose);

        CONNECTED = graph->nvertices == count[0] && graph->nvertices == count[1];
        return CONNECTED;
}

/**
 * @brief                       Performs Dijkstra shortest path from the source vertex.
 * 
 * @param graph                 Graph.
 * @param src                   Source.
 * @param slen                  Source length.
 * @param rela                  Relaxation function.
 * @return struct r2_bfstree*   Returns shortest path tree.
 */
struct r2_dfstree* r2_graph_dijkstra(struct r2_graph *graph, r2_uc *src, r2_uint64 slen,  r2_ldbl(*relax)(r2_ldbl, r2_ldbl))
{
        r2_uint16 FAILED = FALSE;
        struct r2_vertex *source = r2_graph_get_vertex(graph, src, slen); 
        if(source == NULL){
                FAILED = TRUE;
                goto CLEANUP; 
        }

        struct r2_dfstree *dfs             = malloc(sizeof(struct r2_dfstree)); 
        struct r2_dfsnode *tree            = malloc(sizeof(struct r2_dfsnode) * graph->nvertices);
        struct r2_robintable *positions    = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_robintable    *processed = r2_create_robintable(1, 1, 0, 0, .75, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        struct r2_pq *pq                   = r2_create_priority_queue(graph->nvertices, 0, cmp, NULL, NULL);
        if(dfs == NULL || tree == NULL || positions == NULL || processed == NULL || pq == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        /*Initializing tree*/
        dfs->tree      = tree; 
        dfs->positions = positions; 
        dfs->ncount    = 0; 

        /*Initializing distances*/
        for(r2_uint64 i = 0; i < graph->nvertices; ++i){
                tree[i].vertex = NULL; 
                tree[i].pos    = 0;
                tree[i].state  = WHITE; 
                tree[i].dist   = INFINITY;
                tree[i].parent =  -1;   
                tree[i].start  = 0;
                tree[i].end    = 0;
        }

        struct r2_listnode *head  = NULL;
        struct r2_dfsnode  *root  = NULL;
        struct r2_dfsnode  *child = NULL;
        struct r2_edge   *edge    = NULL; 
        struct r2_vertex *dest    = NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        r2_uint64 count = 0;
        
        /*Initialize root*/
        tree[count].vertex   = source; 
        tree[count].state    = GREY;
        tree[count].dist     = 0;
        ++dfs->ncount;
        struct r2_locator *l = r2_pq_insert(pq, &tree[count]);
        struct r2_locator *m = NULL;
        if(l == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        r2_robintable_put(processed, source->vkey, l, source->len);
        r2_robintable_get(processed, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        r2_robintable_put(positions, source->vkey, &tree[count], source->len);
        entry.key = entry.data = NULL; 
        r2_robintable_get(positions, source->vkey, source->len, &entry);
        if(entry.key == NULL){
                FAILED = TRUE; 
                goto CLEANUP;
        }

        while(r2_pq_empty(pq) != TRUE){
                l      = r2_pq_first(pq); 
                root   = l->data;
                source = root->vertex; 
                head   = r2_listnode_first(source->elist);
                while(head != NULL){
                        edge = head->data;
                        dest = edge->dest;
                        entry.key = entry.data = NULL; 
                        r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                        if(entry.key == NULL){
                                ++dfs->ncount;
                                ++count;
                                tree[count].vertex = dest;
                                tree[count].pos    = count; 
                                tree[count].state  = GREY;
                                tree[count].dist   = relax(root->dist, edge->weight); /*possible custom function*/
                                tree[count].parent = root->pos; 
                                m =  r2_pq_insert(pq, &tree[count]);
                                r2_robintable_put(processed, dest->vkey, m, dest->len);
                                entry.key = entry.data = NULL; 
                                r2_robintable_get(processed, dest->vkey, dest->len, &entry);
                                if(entry.key == NULL){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                                
                                }

                                r2_robintable_put(positions, dest->vkey, &tree[count], dest->len);
                                entry.key = entry.data = NULL; 
                                r2_robintable_get(positions, dest->vkey, dest->len, &entry);
                                if(entry.key == NULL){
                                        FAILED = TRUE; 
                                        goto CLEANUP;
                                }
                        }else{
                                m = entry.data;
                                entry.key = entry.data = NULL;
                                r2_robintable_get(positions, dest->vkey, dest->len, &entry);
                                child = entry.data;
                                if(child->state != BLACK){
                                        r2_ldbl dist = relax(root->dist, edge->weight);
                                        if(dist < child->dist){
                                                child->parent = root->pos;
                                                child->dist = dist;
                                                pq =  r2_pq_adjust(pq, m, 0);
                                        }
                                }

                        }
                        head = head->next;
                }
                root->state = BLACK;
                r2_pq_remove(pq, l);

        }

        CLEANUP:
                if(processed != NULL)
                        r2_destroy_robintable(processed);

                if(pq != NULL)
                        r2_destroy_priority_queue(pq);
                
                if(FAILED == TRUE && positions != NULL)
                        r2_destroy_robintable(positions);
                
                if(FAILED == TRUE && tree != NULL)
                        free(tree); 

                if(FAILED == TRUE && dfs != NULL)
                        free(dfs);

        assert(FAILED == FALSE);    
        return dfs;
}

/**
 * @brief                       Performs Dijkstra shortest path from the source vertex.
 * 
 * @param graph                 Graph.
 * @param src                   Source.
 * @param slen                  Source length.
 * @param rela                  Relaxation function.
 * @return struct r2_bfstree*   Returns shortest path tree.
 */
struct r2_dfstree* r2_graph_bellman_ford(struct r2_graph *graph, r2_uc *src, r2_uint64 slen,  r2_ldbl(*relax)(r2_ldbl, r2_ldbl))
{

}


/*Callback function used to compare vertices*/
static r2_int16 cmp(const void *a, const void *b)
{
        struct r2_bfsnode *c = a; 
        struct r2_bfsnode *d = b; 
        if(c->dist <= d->dist)
                return  0; 
        return 1;
}