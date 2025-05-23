#include "r2_graph_test.h"
#include <stdlib.h>
#include <assert.h> 
#include <stdio.h> 
#include <string.h>
#include <math.h>
#include <time.h>

static r2_int16 kcmp(const void *, const void *);
static r2_int16 vcmp(const void *, const void *);
static void print_edges(struct r2_vertex *);
static void print_graph(struct r2_graph *);
static void print_vertex(void *, void *);
static void dumpcc(void *a, void *arg);
static void dump_edges(struct r2_graph *, char *);
static r2_dbl weight(struct r2_edge *);
static void r2_graph_spt_dump(struct r2_graph *, char *);
static void r2_graph_mst_dump(struct r2_graph *, struct r2_graph  * ,char *, r2_weight);
/**
 * Test create graph functionality.
 * 
 */
static void test_r2_create_graph()
{
        struct r2_graph *graph  = r2_create_graph(NULL, NULL,  NULL, NULL, NULL); 
        assert(graph != NULL); 
        assert(graph->nvertices == 0); 
        assert(graph->nedges    == 0); 
        assert(graph->fk == NULL); 
        assert(graph->fv == NULL);
        assert(graph->fd == NULL); 
        assert(graph->gcmp == NULL); 
        assert(graph->vcmp == NULL);
        assert(graph->gat != NULL); 
        assert(graph->vlist != NULL); 
        assert(graph->elist != NULL); 
        r2_destroy_graph(graph);     
}

/**
 * Tests add vertex functionality.
 * 
 */
static void test_r2_graph_add_vertex()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
                
        assert(graph->nvertices == 10);
        assert(graph->vlist->lsize == 10); 
        assert(graph->vertices->nsize == 10);
        r2_destroy_graph(graph);     
}


/**
 * Testing get vertex functionality.
 * 
 */
static void test_r2_graph_get_vertex()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex = NULL; 
        for(r2_uint64 i = 0; i < 10; ++i){
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64)); 
                assert(vertex->vkey == (r2_uc*)&vertices[i]);
        }

        r2_destroy_graph(graph);      
}

static void test_r2_graph_del_vertex()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        struct r2_vertex *vertex = NULL; 
        for(r2_uint64 i = 0; i < 10; ++i){
                r2_graph_del_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64)); 
                assert(vertex == NULL);
        }
        
        assert(graph->nvertices == 0);
        assert(graph->vlist->lsize == 0); 
        assert(graph->vertices->nsize == 0);

        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
       
        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        
        for(r2_uint64 i = 0; i < 10; ++i){
                r2_graph_del_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64)); 
                assert(vertex == NULL);
        }
        r2_destroy_graph(graph);
}

/**
 * Testing add edge functionality.
 * 
 */
static void test_r2_graph_add_edge()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i){
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        }

        assert(graph->nvertices == 13);
        assert(graph->nedges == 20); 
        assert(graph->elist->lsize == 20);
        
        r2_destroy_graph(graph);
}

/**
 * @brief     Test get edge functionality.
 * 
 */
static void test_r2_graph_get_edge()
{
        struct r2_listnode *node = NULL;
        struct r2_edge     *edge = NULL;
        r2_uint64 vertices[10]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph   = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

       
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[3], sizeof(r2_uint64));
        assert(vertex->nedges == 4); 
        assert(vertex->elist->lsize  == 4);
        node = r2_listnode_first(vertex->elist);
        edge = node->data; 
        assert(edge->src->vkey  == (r2_uc*)&vertices[3]); 
        assert(edge->dest->vkey == (r2_uc*)&vertices[4]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == (r2_uc*)&vertices[3]); 
        assert(edge->dest->vkey == (r2_uc*)&vertices[5]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == (r2_uc*)&vertices[3]); 
        assert(edge->dest->vkey == (r2_uc*)&vertices[9]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == (r2_uc*) &vertices[3]); 
        assert(edge->dest->vkey == (r2_uc*) &vertices[1]);
        node = node->next; 
        assert(node == NULL); 
        
        for(r2_uint64 i = 0; i < 20; ++i){
                edge  = r2_graph_get_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
                assert(edge != NULL);
                assert(*(r2_uint64 *)edge->src->vkey  ==  edges[i][0]); 
                assert(*(r2_uint64 *)edge->dest->vkey == edges[i][1]);
        }

        
        
        r2_destroy_graph(graph);
} 

/**
 * Test delete edge functionality.
 * 
 */
static void test_r2_graph_del_edge()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        

        struct r2_edge *edge = NULL;
        for(r2_uint64 i = 0; i < 20; ++i){
                r2_graph_del_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
                r2_graph_get_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
                assert(edge == NULL);
        }
        
        assert(graph->nedges == 0);
        assert(r2_list_empty(graph->elist) == TRUE);
        r2_destroy_graph(graph);
}

/**
 * @brief  Test graph add attributes functionality.
 * 
 */
static void test_r2_graph_add_attributes()
{
        struct r2_graph *graph = r2_create_graph(vcmp, kcmp, NULL, NULL, NULL);
        char * attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_graph_add_attributes(graph, attribute[0], attribute[0], strlen(attribute[0]));
        r2_graph_add_attributes(graph, attribute[1], attribute[1], strlen(attribute[1]));
        r2_graph_add_attributes(graph, attribute[2], attribute[2], strlen(attribute[2]));

        assert(graph->gat->nsize == 3);
        r2_destroy_graph(graph);
}

/**
 * @brief       Test graph delete attributes functionality.
 * 
 */
static void test_r2_graph_del_attributes()
{
        struct r2_graph *graph = r2_create_graph(vcmp, kcmp, NULL, NULL, NULL);
        char * attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_graph_add_attributes(graph, attribute[0], attribute[0], strlen(attribute[0]));
        r2_graph_add_attributes(graph, attribute[1], attribute[1], strlen(attribute[1]));
        r2_graph_add_attributes(graph, attribute[2], attribute[2], strlen(attribute[2]));
        r2_graph_del_attributes(graph, attribute[0], strlen(attribute[0]));
        char *a = r2_graph_get_attributes(graph, attribute[0], strlen(attribute[0]));
        assert(graph->gat->nsize == 2); 
        assert(a == NULL);
        r2_destroy_graph(graph);

}

/**
 * @brief Test graph get attributes functionality.
 * 
 */
static void test_r2_graph_get_attributes()
{
        struct r2_graph *graph = r2_create_graph(vcmp, kcmp, NULL, NULL, NULL);
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_graph_add_attributes(graph, attribute[0], attribute[0], strlen(attribute[0]));
        r2_graph_add_attributes(graph, attribute[1], attribute[1], strlen(attribute[1]));
        r2_graph_add_attributes(graph, attribute[2], attribute[2], strlen(attribute[2]));
        char *a = r2_graph_get_attributes(graph, attribute[0], strlen(attribute[0])); 
        assert(a != NULL);
        r2_destroy_graph(graph);
}

/**
 * @brief Test vertex add attribute.
 * 
 */
static void test_r2_vertex_add_attributes()
{
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[0], sizeof(r2_uint64)); 
        r2_vertex_add_attributes(vertex, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[2], attribute[2], strlen(attribute[2]), kcmp);
        assert(vertex->vat->nsize == 3);
        r2_destroy_graph(graph);
}

/**
 * @brief Test add vertex attributes.
 * 
 */
static void test_r2_vertex_get_attributes()
{
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[0], sizeof(r2_uint64)); 
        r2_vertex_add_attributes(vertex, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[2], attribute[2], strlen(attribute[2]), kcmp);

        char *a = r2_vertex_get_attributes(vertex, attribute[0], strlen(attribute[0]), kcmp);
        assert(a != NULL);
        r2_destroy_graph(graph);
}

static void test_r2_vertex_del_attributes()
{
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc*)&vertices[0], sizeof(r2_uint64)); 
        r2_vertex_add_attributes(vertex, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_vertex_add_attributes(vertex, attribute[2], attribute[2], strlen(attribute[2]), kcmp);

        r2_vertex_del_attributes(vertex, attribute[0], strlen(attribute[0]), kcmp);
        char *a = r2_vertex_get_attributes(vertex, attribute[0], strlen(attribute[0]), kcmp);
        assert(a == NULL);
        assert(vertex->vat->nsize == 2);
        r2_destroy_graph(graph);
}

/**
 * @brief       Test edge add attributes 
 * 
 */
static void test_r2_edge_add_attributes()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, (r2_uc*)&edges[0][0], sizeof(r2_uint64), (r2_uc*)&edges[0][1], sizeof(r2_uint64));
        r2_edge_add_attributes(edge, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_edge_add_attributes(edge, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_edge_add_attributes(edge, attribute[2], attribute[2], strlen(attribute[2]), kcmp);
        assert(edge->eat->nsize == 3);
        r2_destroy_graph(graph);
}

/**
 * @brief       Test edge get attributes
 * 
 */
static void test_r2_edge_get_attributes()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, (r2_uc*)&edges[0][0], sizeof(r2_uint64), (r2_uc*)&edges[0][1], sizeof(r2_uint64));
        r2_edge_add_attributes(edge, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_edge_add_attributes(edge, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_edge_add_attributes(edge, attribute[2], attribute[2], strlen(attribute[2]), kcmp);
        char *a = r2_edge_get_attributes(edge, attribute[0], strlen(attribute[0]), kcmp);
        assert(a != NULL); 
        assert(strcmp(a, attribute[0]) == 0);
        r2_destroy_graph(graph);
} 

/**
 * @brief       Test edge del attributes
 * 
 */
static void test_r2_edge_del_attributes()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, (r2_uc*)&edges[0][0], sizeof(r2_uint64), (r2_uc*)&edges[0][1], sizeof(r2_uint64));
        r2_edge_add_attributes(edge, attribute[0], attribute[0], strlen(attribute[0]), kcmp);
        r2_edge_add_attributes(edge, attribute[1], attribute[1], strlen(attribute[1]), kcmp);
        r2_edge_add_attributes(edge, attribute[2], attribute[2], strlen(attribute[2]), kcmp);
        r2_edge_del_attributes(edge, attribute[2], strlen(attribute[2]), kcmp);
        char *a = r2_edge_get_attributes(edge, attribute[2], strlen(attribute[2]), kcmp);
        assert(a == NULL); 
        assert(edge->eat->nsize == 2);
        r2_destroy_graph(graph);
}

/**
 * @brief Test graph transpose functionality.
 * 
 */
static void test_r2_graph_transpose()
{
        r2_uint64 vertices[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

        struct r2_graph *transpose = r2_graph_transpose(graph);
        struct r2_vertex   *src    = NULL;
        struct r2_vertex   *dest   = NULL;
        struct r2_listnode *head   = r2_listnode_first(graph->elist);
        struct r2_edge     *edge   = NULL;
        while(head != NULL){
                edge  = head->data;
                src   = edge->src; 
                dest  = edge->dest;
                edge = NULL; 
                edge = r2_graph_get_edge(transpose, dest->vkey, dest->len, src->vkey, src->len);
                assert(edge != NULL);
                head  = head->next;
        }

        head   = r2_listnode_first(transpose->elist);
        while(head != NULL){
                edge  = head->data;
                src   = edge->src; 
                dest  = edge->dest;
                edge = NULL; 
                edge = r2_graph_get_edge(graph, dest->vkey, dest->len, src->vkey, src->len);
                assert(edge != NULL);
                head  = head->next;
        }

        assert(graph->nvertices == transpose->nvertices); 
        assert(graph->nedges == transpose->nedges);        
        r2_destroy_graph(transpose); 
        r2_destroy_graph(graph);
}

/**
 * @brief Tests bfs functionality.
 * 
 */
static void test_r2_graph_bfs()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc*)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc*)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));

        printf("\nBFS: ");
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        r2_graph_bfs(graph, vertex, print_vertex, NULL);
        r2_destroy_graph(graph);
}

/**
 * @brief Test dfs functionality.
 * 
 */
static void test_r2_graph_dfs()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        printf("\nDFS: ");
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        r2_graph_dfs(graph, vertex, print_vertex, NULL);
        r2_destroy_graph(graph);
}

/**
 * @brief Test cycle functionality.
 * 
 */
static void test_r2_graph_has_cycle()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

        assert(r2_graph_has_cycle(graph) == TRUE);
        r2_destroy_graph(graph);    

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));
        
        assert(r2_graph_has_cycle(graph) != TRUE);
        r2_destroy_graph(graph);        
}

/**
 * @brief Tests graph traversals
 * 
 */
static void test_r2_graph_traversals()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_list* preorder = r2_graph_dfs_traversals(graph, NULL, 0);
        struct r2_listnode *head = r2_listnode_first(preorder);
        struct r2_vertex *vertex = NULL; 
        
        vertex = head->data; 
        assert(*(vertex->vkey) == 1);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 2);
        head = head->next;         

        vertex = head->data; 
        assert(*(vertex->vkey) == 5);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 8);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 0);
        head = head->next;         


        vertex = head->data; 
        assert(*(vertex->vkey) == 3);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 9);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 7);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 6);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 4);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 10);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 12);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 11);
        head = head->next; 

        assert(head == NULL);

        r2_destroy_list(preorder);

        struct r2_list *postorder = r2_graph_dfs_traversals(graph, NULL, 1);

        head = r2_listnode_first(postorder);
        vertex = head->data; 
        assert(*(vertex->vkey) == 0);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 8);
        head = head->next;         

        vertex = head->data; 
        assert(*(vertex->vkey) == 5);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 2);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 1);
        head = head->next;         


        vertex = head->data; 
        assert(*(vertex->vkey) == 7);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 9);
        head = head->next; 


        vertex = head->data; 
        assert(*(vertex->vkey) == 6);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 3);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 10);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 4);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 11);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 12);
        head = head->next; 

        assert(head == NULL);
        r2_destroy_list(postorder);

        struct r2_list *topological = r2_graph_dfs_traversals(graph, NULL, 2);
        head = r2_listnode_first(topological);
        vertex = head->data; 
        assert(*(vertex->vkey) == 12);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 11);
        head = head->next;         

        vertex = head->data; 
        assert(*(vertex->vkey) == 4);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 10);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 3);
        head = head->next;         


        vertex = head->data; 
        assert(*(vertex->vkey) == 6);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 9);
        head = head->next; 


        vertex = head->data; 
        assert(*(vertex->vkey) == 7);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 1);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 2);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 5);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 8);
        head = head->next; 

        vertex = head->data; 
        assert(*(vertex->vkey) == 0);
        head = head->next; 

        assert(head == NULL);

        r2_destroy_list(topological);
        r2_destroy_graph(graph);
}

/**
 * @brief Tests topological sort.
 * 
 */
static void test_graph_topological_sort()
{
        r2_uint64 vertices[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

        assert(r2_graph_topological_sort(graph) == NULL); 
        r2_destroy_graph(graph);                
        
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        for(r2_uint64 i = 0; i < 4; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64)); 
        
        assert(r2_graph_topological_sort(graph) != NULL);  
        r2_destroy_graph(graph);  


        r2_uint64 tedges [][2] = {
                {1, 2}, 
                {2, 4}, 
                {2, 5},
                {1, 4},
                {1, 3},
                {4, 3},
                {4, 7}, 
                {4, 6}, 
                {5, 7},
                {5, 4},
                {7, 6},
                {3, 6},
                {6, 1}, 
        };

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 12; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&tedges[i][0], sizeof(r2_uint64), (r2_uc *)&tedges[i][1], sizeof(r2_uint64)); 

        struct r2_list *topological = r2_graph_topological_sort(graph);
        struct r2_listnode *head    = r2_listnode_first(topological); 
        struct r2_vertex *vertex    = head->data;
        assert(*(vertex->vkey) == 1);
        
        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 2);

        
        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 5);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 4);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 3);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 7);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 6);
        head = head->next;

        assert(head == NULL);

        r2_destroy_list(topological);

        topological = r2_graph_dfs_traversals(graph, NULL, 2);

        head = r2_listnode_first(topological); 
        vertex = head->data;
        assert(*(vertex->vkey) == 1);
        
        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 2);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 5);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 4);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 7);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 3);

        head = head->next;
        vertex = head->data;
        assert(*(vertex->vkey) == 6);
        head = head->next;

        assert(head == NULL);
        r2_destroy_list(topological);
        r2_destroy_graph(graph);

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 13; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&tedges[i][0], sizeof(r2_uint64), (r2_uc *)&tedges[i][1], sizeof(r2_uint64));

        topological = r2_graph_topological_sort(graph);
        assert(topological == NULL);
        r2_destroy_graph(graph);   
}

/**
 * @brief Tests topological sort edges
 * 
 */
static void test_graph_topological_sort_edges()
{
        r2_uint64 vertices[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

   
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

        assert(r2_graph_topological_sort_edges(graph) == NULL); 
        r2_destroy_graph(graph); 

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        for(r2_uint64 i = 0; i < 4; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64)); 
        
        assert(r2_graph_topological_sort_edges(graph) != NULL);  
        r2_destroy_graph(graph);    

        r2_uint64 tedges [][2] = {
                {1, 2}, 
                {2, 4}, 
                {2, 5},
                {1, 4},
                {1, 3},
                {4, 3},
                {4, 7}, 
                {4, 6}, 
                {5, 7},
                {5, 4},
                {7, 6},
                {3, 6},
                {6, 1}, 
        };

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 12; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&tedges[i][0], sizeof(r2_uint64), (r2_uc *)&tedges[i][1], sizeof(r2_uint64)); 

        struct r2_list *topological = r2_graph_topological_sort_edges(graph);
        struct r2_listnode *head    = r2_listnode_first(topological); 
        struct r2_edge *edge   = head->data;
        printf("\n-----------------------Topological Sort--------------------");
        while(head != NULL){
                edge = head->data;
                printf("\n[%ld => %d]", *edge->src->vkey, *edge->dest->vkey);
                head = head->next;
        }
        
        head    = r2_listnode_first(topological); 
        edge   = head->data;
        assert(*edge->src->vkey  == vertices[0]); //[1, 2]
        assert(*edge->dest->vkey == vertices[1]);
        head = head->next; 
        edge = head->data;

        assert(*edge->src->vkey  == vertices[1]); //[2, 5]
        assert(*edge->dest->vkey == vertices[4]);
        head = head->next; 
        edge = head->data;
       

        assert(*edge->src->vkey == vertices[4]); //[5, 4]
        assert(*edge->dest->vkey == vertices[3]); 
        head = head->next; 
        edge = head->data;

        assert(*edge->src->vkey == vertices[3]);
        assert(*edge->dest->vkey == vertices[2]); 
        head = head->next; 
        edge = head->data;
        
        assert(*edge->src->vkey == vertices[3]); 
        assert(*edge->dest->vkey == vertices[6]);
        head = head->next; 
        edge = head->data;
        

        assert(*edge->src->vkey == vertices[6]); 
        assert(*edge->dest->vkey == vertices[5]);
        head = head->next; 

        r2_destroy_list(topological);
        r2_destroy_graph(graph);
}


/**
 * @brief Test path functionality.
 * 
 */
static void test_r2_graph_has_path()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                r2_graph_add_vertex(graph, (r2_uc *)&vertices[i], sizeof(r2_uint64));

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };
 
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        struct r2_vertex   *dest =   r2_graph_get_vertex(graph, (r2_uc *)&vertices[8], sizeof(r2_uint64));
        assert(r2_graph_has_path(graph, src, dest) == TRUE);

        dest = r2_graph_get_vertex(graph, (r2_uc *)&edges[6][1], sizeof(r2_uint64));
        assert(r2_graph_has_path(graph, src, dest) != TRUE);
        r2_destroy_graph(graph);
}

/**
 * @brief Test get path functionality.
 * 
 */
static void test_r2_graph_get_paths()
{
        r2_uint64 vertices[] = {'s', 'y'};
        r2_uint64 edges [][2] = {
                {'s', 't'},
                {'s', 'y'},
                {'t', 'y'},
                {'y', 't'},
                {'t', 'x'}, 
                {'t', 'k'},
                {'k', 'x'},
                {'x', 'z'},
                {'x', 'y'},
                {'z', 'x'}, 
                {'y', 'z'},
                {'z', 'y'},
                {'z', 's'}
        };

        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 12; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_vertex *src  = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64)); 
        struct r2_vertex *dest = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64)); 
        struct r2_list *paths  = r2_graph_get_paths(graph, src, dest);
        struct r2_list *path   = NULL;  
        assert(paths->lsize == 6);
        struct r2_listnode *head = r2_listnode_first(paths);
        printf("\n--------------All Paths---------------------");
        while(head != NULL){
                path = head->data; 
                struct r2_listnode *cur = r2_listnode_first(path); 
                printf("\n");
                while(cur != NULL){
                        src = cur->data; 
                        if(cur->next != NULL)
                                printf("%c=>",* src->vkey);
                        else
                                 printf("%c",* src->vkey);
                        cur = cur->next; 
                }
              
                head = head->next;
        } 
        r2_destroy_list(paths);
        r2_destroy_graph(graph); 
}

static void test_r2_graph_get_paths_edges()
{
        r2_uint64 vertices[] = {'s', 'y'};
        r2_uint64 edges [][2] = {
                {'s', 't'},
                {'s', 'y'},
                {'t', 'y'},
                {'y', 't'},
                {'t', 'x'}, 
                {'t', 'k'},
                {'k', 'x'},
                {'x', 'z'},
                {'x', 'y'},
                {'z', 'x'}, 
                {'y', 'z'},
                {'z', 'y'},
                {'z', 's'}
        };

        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 12; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_vertex *src  = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64)); 
        struct r2_vertex *dest = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64)); 
        struct r2_list *elist  = r2_graph_get_paths_edges(graph, src, dest);
        struct r2_edge *edge   = NULL;    
        assert(elist->lsize == 9);
        struct r2_listnode *head = r2_listnode_first(elist); 
        printf("\n-----------------------All Path Edges------------");
        while(head != NULL){
                edge = head->data; 
                printf("\n%c=>%c", *edge->src->vkey, *edge->dest->vkey);
                head = head->next;
        }
        r2_destroy_list(elist); 
        r2_destroy_graph(graph);              
}

/**
 * @brief Tests bfs tree functionality.
 * 
 */
static void test_r2_graph_bfs_tree()
{
        r2_uint64 vertices[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

  
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        
        /*Source 4*/
        struct r2_vertex *source = r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        printf("\n-------------------BFS Tree---------------");
        struct r2_graph *bfs = r2_graph_bfs_tree(graph, source);
        print_graph(bfs);
        assert(bfs->nvertices == 11);
        assert(bfs->nedges == 10);
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        assert(r2_graph_parent(bfs, source) == NULL);

        
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[0], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[4]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[1], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[3]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[2], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[5]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[4], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[3]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[5], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[3]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[6], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[8]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[7], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[4]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[8], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[9]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[9], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[3]);

                
        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[10], sizeof(r2_uint64));
        assert(*r2_graph_parent(bfs, source)->vkey == vertices[7]);
        r2_destroy_graph(bfs);
        r2_destroy_graph(graph);
}

/**
 * @brief Test dfs tree functionality.
 * 
 */
static void test_r2_graph_dfs_tree()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };
        
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        
        
        /*Source 4*/
        struct r2_vertex *source = r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        printf("\n--------------------------DFS Tree----------------");
        struct r2_graph *dfs = r2_graph_dfs_tree(graph, source);
        print_graph(dfs);
        assert(dfs->nvertices == 11);
        assert(dfs->nedges == 10);
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        assert(r2_graph_parent(dfs, source) == NULL);

        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[0], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[4]);
        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[1], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[0]);

        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[2], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[5]);

        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[4], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[3]);

        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[5], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[3]);

        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[6], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[8]);

        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[7], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[4]);
        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[8], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[2]);
        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[9], sizeof(r2_uint64));
        assert(*r2_graph_parent(dfs, source)->vkey == vertices[3]);

        r2_destroy_graph(dfs);
        r2_destroy_graph(graph);        
}

/**
 * @brief Tests graph children functionality.
 * 
 */
static void test_r2_graph_children()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);

        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };
        
        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        
        
        /*Source 4*/
        struct r2_vertex *source = r2_graph_get_vertex(graph, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        struct r2_graph *dfs     = r2_graph_dfs_tree(graph, source);
        struct r2_graph *bfs     = r2_graph_bfs_tree(graph, source);
        
        source = r2_graph_get_vertex(dfs, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        struct r2_list *children = r2_graph_children(dfs, source);
        struct r2_listnode *head = r2_listnode_first(children); 
        source = head->data; 
        assert(*source->vkey == vertices[4]);
        head = head->next;

        source = head->data; 
        assert(*source->vkey == vertices[5]);
        head = head->next;

        source = head->data; 
        assert(*source->vkey == vertices[9]);
        head = head->next;
        assert(head == NULL);

        source = r2_graph_get_vertex(bfs, (r2_uc *)&vertices[3], sizeof(r2_uint64));
        children = r2_graph_children(bfs, source);
        head = r2_listnode_first(children); 
        source = head->data; 
        assert(*source->vkey == vertices[4]);
        head = head->next;

        source = head->data; 
        assert(*source->vkey == vertices[5]);
        head = head->next;

        source = head->data; 
        assert(*source->vkey == vertices[9]);
        head = head->next;
   

        source = head->data; 
        assert(*source->vkey == vertices[1]);
        head = head->next;
        assert(head == NULL);

        r2_destroy_graph(dfs); 
        r2_destroy_graph(bfs);
        r2_destroy_graph(graph);
}

/**
 * @brief Test is strongly connected procedure.
 * 
 */
static void test_r2_graph_strongly_connected()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);   
        r2_uint64 edge[][2] = {
                {1, 2},
                {2, 1},
                {3, 1}
        };

        for(r2_uint64 i = 0; i < 2;++i)
                r2_graph_add_edge(graph, (r2_uc *)&edge[i][0], sizeof(r2_uint64), (r2_uc *)&edge[i][1], sizeof(r2_uint64));  
        
        assert(r2_graph_strongly_connected(graph) == TRUE);
        r2_destroy_graph(graph); 

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);  
        for(r2_uint64 i = 0; i < 3;++i)
                r2_graph_add_edge(graph, (r2_uc *)&edge[i][0], sizeof(r2_uint64), (r2_uc *)&edge[i][1], sizeof(r2_uint64));  


        assert(r2_graph_strongly_connected(graph) == FALSE);
        r2_destroy_graph(graph); 
}

/**
 * @brief Tests if graph is bipartite.
 * 
 */
static void test_r2_graph_is_bipartite()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64),  (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

  
        assert(r2_graph_is_bipartite(graph) == FALSE); 
        r2_destroy_graph(graph);   
        
        r2_uint64 bipartite[][2] = {
                {'1', 'A'},
                {'A', '1'},
                {'A', '3'},
                {'3', 'A'},
                {'2', 'C'},
                {'C', '2'},
                {'3', 'B'},
                {'B', '3'},
                {'1', 'E'},
                {'E', '1'},                
                {'4', 'C'},
                {'C', '4'},
                {'4', 'E'},
                {'E', '4'},
                {'5', 'E'},
                {'E', '5'}
        };
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 16; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&bipartite[i][0], sizeof(r2_uint64), (r2_uc *)&bipartite[i][1], sizeof(r2_uint64));  

        assert(r2_graph_is_bipartite(graph) == TRUE); 
        r2_destroy_graph(graph); 
}

/**
 * @brief Test bipartite set functionality.
 * 
 */
static void test_r2_graph_bipartite_set()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        r2_uint64 edges [][2] = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64),  (r2_uc *)&edges[i][1], sizeof(r2_uint64));  

  
        assert(r2_graph_bipartite_set(graph, 0) == NULL);
        r2_destroy_graph(graph);   
        
        r2_uint64 bipartite[][2] = {
                {'1', 'A'},
                {'A', '1'},
                {'A', '3'},
                {'3', 'A'},
                {'2', 'C'},
                {'C', '2'},
                {'3', 'B'},
                {'B', '3'},
                {'1', 'E'},
                {'E', '1'},                
                {'4', 'C'},
                {'C', '4'},
                {'4', 'E'},
                {'E', '4'},
                {'5', 'E'},
                {'E', '5'}
        };
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 16; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&bipartite[i][0], sizeof(r2_uint64), (r2_uc *)&bipartite[i][1], sizeof(r2_uint64));  

        struct r2_list *A = r2_graph_bipartite_set(graph, 0);
        struct r2_list *B = r2_graph_bipartite_set(graph, 1);
        struct r2_listnode *head = r2_listnode_first(A);
        struct r2_listnode *cur = NULL;  
        struct r2_vertex *vertex = NULL;
        while(head != NULL){
                cur = r2_listnode_first(B); 
                while(cur != NULL){
                        assert(head->data != cur->data); 
                        cur  = cur->next;
                }
                head = head->next;
        }
        printf("\nSet A: "); 
        head = r2_listnode_first(A);
        while(head != NULL){
                vertex = head->data;
                printf("%c ", *vertex->vkey);
                head = head->next;
        }

        printf("\nSet B: "); 
        head = r2_listnode_first(B);
        while(head != NULL){
                vertex = head->data;
                printf("%c ", *vertex->vkey);
                head = head->next;
        }
        r2_destroy_list(A); 
        r2_destroy_list(B);
        r2_destroy_graph(graph);    
}


/**
 * @brief Tests connected components functionality.
 * 
 */
static void test_r2_graph_connected_components()
{
        r2_uint64 vertices[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11, 12};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        r2_uint64 edges [][2]  = {
                {1, 2}, 
                {4, 5}, 
                {2, 5},
                {7, 3},
                {0, 1}, 
                {8, 0},
                {12, 11}, 
                {4, 6}, 
                {10, 9}, 
                {4, 10}, 
                {3, 9}, 
                {6, 3}, 
                {3, 6},
                {4, 2},
                {5, 1},
                {5, 8},
                {3, 2},
                {11, 5},
                {9, 7},
                {7 , 8}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc*)&edges[i][1], sizeof(r2_uint64));  
        
        struct r2_forest *forest = r2_graph_cc(graph);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\n---------Connected Component %d--------------", i);
                print_graph(forest->tree[i]);
        }
        assert(forest->ncount == 3);
        
        /*Verifying forest*/
        struct r2_graph *tree = forest->tree[0];
        assert(tree->nvertices == 5);
        struct r2_listnode *head = r2_listnode_first(tree->vlist);
        
        struct r2_vertex *source = head->data;
        assert(*source->vkey ==  vertices[0]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[1]);
        
        head = head->next;
        source = head->data;
        assert(*source->vkey == vertices[4]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[7]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[10]);
        head = head->next;
        assert(head == NULL);

        tree = forest->tree[1];
        head = r2_listnode_first(tree->vlist);
        assert(tree->nvertices == 6);
        
        source = head->data;
        assert(*source->vkey == vertices[3]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[5]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[9]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[2]);
        
        head = head->next;
        source = head->data;
        assert(*source->vkey == vertices[8]);
        
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[6]);
        head = head->next;
        assert(head == NULL);

        tree = forest->tree[2];
        head = r2_listnode_first(tree->vlist);
        assert(tree->nvertices == 2);
        source = head->data;

        assert(*source->vkey == vertices[12]);
        head   = head->next;
        source = head->data;
        assert(*source->vkey == vertices[11]);
        head = head->next;
        assert(head == NULL);


        r2_graph_destroy_cc(forest);
        r2_destroy_graph(graph);
}

/**
 * @brief Test strongly connected components.
 * 
 */
static void test_r2_graph_scc()
{
        r2_uint64 vertices[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);

        r2_uint64 edges [][2] = {
                {'a', 'b'}, 
                {'b', 'c'}, 
                {'c', 'd'},
                {'d', 'c'},
                {'d', 'h'}, 
                {'h', 'h'},
                {'c', 'g'}, 
                {'g', 'f'}, 
                {'f', 'g'}, 
                {'e', 'a'}, 
                {'e', 'f'},
                {'b', 'e'},
                {'g', 'h'}
        };  

        for(r2_uint64 i = 0; i < 13; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));   

        struct r2_forest *forest =   r2_graph_tscc(graph);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nTarjan Strongly Connected Component %d: ", i);
                print_graph(forest->tree[i]);
        }

        assert(forest->ncount == 4);
        struct r2_graph* tree  = forest->tree[0];
        struct r2_listnode *head = r2_listnode_first(tree->vlist);
        struct r2_vertex *source = head->data;
        assert(*source->vkey == 'h');
        head = head->next;
        assert(head == NULL);

        tree  = forest->tree[1];
        head = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey  == 'g'); 
        head = head->next;
        source = head->data;
        assert(*source->vkey  == 'f');
        head = head->next;
        assert(head == NULL);

        tree  = forest->tree[2];
        head  = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == 'c'); 
        head = head->next;
        source = head->data;
        assert(*source->vkey == 'd');
        head = head->next;
        assert(head == NULL);

        tree  = forest->tree[3];
        head  = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == 'a'); 
        
        head = head->next;
        source = head->data;
        assert(*source->vkey == 'b');
        
        head = head->next;
        source = head->data;
        assert(*source->vkey == 'e');
        head = head->next;
        assert(head == NULL);
       
        r2_destroy_graph(graph);  
        r2_graph_destroy_cc(forest);

        graph  = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        r2_uint64 edge [][2] = {
                {'0', '1'}, 
                {'1', '2'}, 
                {'2', '3'},
                {'3', '0'},
                {'4', '5'},
                {'5', '6'},
                {'6', '4'}, 
                {'6', '7'}
        }; 

        for(r2_uint64 i = 0; i < 8; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edge[i][0], sizeof(r2_uint64), (r2_uc *)&edge[i][1], sizeof(r2_uint64));   

        forest = r2_graph_tscc(graph);
        assert(forest->ncount == 3);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nTarjan Strongly Connected Component %d: ", i);
                print_graph(forest->tree[i]);
        }


        tree  = forest->tree[0];
        head = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == '0');
        head = head->next; 
        source = head->data;
        assert(*source->vkey == '1');
        
        head = head->next; 
        source = head->data;
        assert(*source->vkey == '2');
        
        head = head->next; 
        source = head->data;
        assert(*source->vkey == '3');
        head = head->next; 
        assert(head == NULL);

        tree  = forest->tree[1];
        head = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == '7');
        head = head->next; 
        assert(head == NULL);

        tree   = forest->tree[2];
        head   = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == '4');

        head   = head->next; 
        source = head->data;
        assert(*source->vkey == '5');
        
        head = head->next; 
        source = head->data;
        assert(*source->vkey == '6');
        
        head = head->next; 

        assert(head == NULL);

        r2_graph_destroy_cc(forest);
        r2_destroy_graph(graph);  

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 13; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));   

        forest = r2_graph_kcc(graph);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nKorsaju  Strongly Connected Component %d: ", i);
                print_graph(forest->tree[i]);
        }

        assert(forest->ncount == 4);
        tree  = forest->tree[0];
        head  = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == 'a'); 
        
        head = head->next;
        source = head->data;
        assert(*source->vkey == 'e');
     

        head = head->next;
        source = head->data;
        assert(*source->vkey == 'b');
        head = head->next;
        assert(head == NULL);

      

        tree  = forest->tree[2];
        head = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey  == 'g'); 
   
        

        head = head->next;
        source = head->data;
        assert(*source->vkey  == 'f');
        head = head->next;
        assert(head == NULL);


        tree  = forest->tree[1];
        head  = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == 'c'); 
        head = head->next;
        source = head->data;

        assert(*source->vkey == 'd');
        head = head->next;
        assert(head == NULL);

        tree  = forest->tree[3];
        head = r2_listnode_first(tree->vlist);
        source = head->data;
        assert(*source->vkey == 'h');
        head = head->next;
        assert(head == NULL);

        r2_graph_destroy_cc(forest);
        r2_destroy_graph(graph);  
      
}

/**
 * @brief Tests is connected functionality.
 * 
 */
static void test_r2_graph_is_connected()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);   
        r2_uint64 edge[][2] = {
                {1, 2},
                {2, 1},
                {3, 1}
        };

        for(r2_uint64 i = 0; i < 2;++i)
                r2_graph_add_edge(graph, (r2_uc *)&edge[i][0], sizeof(r2_uint64), (r2_uc *)&edge[i][1], sizeof(r2_uint64));  
        
        assert(r2_graph_is_connected(graph) == TRUE);
        r2_destroy_graph(graph); 

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);  
        for(r2_uint64 i = 0; i < 3;++i)
                r2_graph_add_edge(graph, (r2_uc *)&edge[i][0], sizeof(r2_uint64), (r2_uc *)&edge[i][1], sizeof(r2_uint64));  


        assert(r2_graph_is_connected(graph) == FALSE);
        r2_destroy_graph(graph); 
}

/**
 * @brief Test path tree functionality.
 * 
 */
static void test_r2_graph_path_tree()
{
        r2_uint64 vertices[] = {'s', 'y'};
        r2_uint64 edges [][2] = {
                {'s', 't'},
                {'s', 'y'},
                {'t', 'y'},
                {'y', 't'},
                {'t', 'x'}, 
                {'t', 'k'},
                {'k', 'x'},
                {'x', 'z'},
                {'x', 'y'},
                {'z', 'x'}, 
                {'y', 'z'},
                {'z', 'y'},
                {'z', 's'}
        };

        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 12; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_vertex *src  = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64)); 
        struct r2_vertex *dest = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64)); 

        struct r2_graph *ptree = r2_graph_path_tree(graph, src, dest);
        printf("\nPath Tree:");
        print_graph(ptree);
        r2_destroy_graph(ptree); 
        r2_destroy_graph(graph);
}

/**
 * @brief Testing biconnectivity functionality.
 * 
 */
static void test_r2_graph_bcc()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        r2_uint64 edges[][2] = {
                {'G', 'H'},
                {'H', 'G'},
                {'H', 'J'},
                {'J', 'H'},
                {'G', 'J'},
                {'J', 'G'},
                {'J', 'F'},
                {'F', 'J'},
                {'G', 'F'},
                {'F', 'G'}, 
                {'H', 'I'},
                {'I', 'H'},
                {'F', 'I'},
                {'I', 'F'},
                {'F', 'C'},
                {'C', 'F'},
                {'C', 'E'},
                {'E', 'C'},
                {'F', 'E'}, 
                {'E', 'F'}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_forest *forest = r2_graph_bcc(graph);
        struct r2_graph *bcc     =  NULL; 
        for(r2_uint64 i = 0; i < forest->ncount; ++i){
                printf("\nBi-connected Component %d: ", i);
                print_graph(forest->tree[i]);
        }
        
        FILE *fp = NULL;
        char *fname[] = {"bcc_1.txt", "bcc_2.txt"};
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        struct r2_vertex *vertex[2]; 
        for(r2_uint64 i = 0; i < forest->ncount; ++i){
                fp  = fopen(fname[i], "r");
                bcc = forest->tree[i];
                fscanf(fp, "%lld", &vertices[0]);
                assert(vertices[0] == forest->tree[i]->nvertices);
                fscanf(fp, "%lld", &vertices[0]);
                assert(vertices[0] == forest->tree[i]->nedges);
                while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                        assert(r2_graph_get_edge(bcc, (r2_uc *)&vertices[0], sizeof(r2_uint64), (r2_uc *)&vertices[1], sizeof(r2_uint64)) != NULL);
                } 
                fclose(fp); 
        }



        r2_destroy_graph(graph);
        r2_graph_destroy_cc(forest);
        fp  = fopen("Email-Enron.txt", "r");
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
        } 
        fclose(fp);
        forest = r2_graph_bcc(graph);
        r2_uint64 largest = 0;
        r2_uint64 pos = 0 ; 
       
        for(r2_uint64 i = 0; i < forest->ncount; ++i){
                if(forest->tree[i]->nvertices > largest){
                        largest = forest->tree[i]->nvertices;
                        pos = i;
                }
        } 
       
        bcc = forest->tree[pos];
        fp = fopen("bcc_edges.txt", "r");
        pos = 0;
        while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                assert(r2_graph_get_edge(bcc, (r2_uc *)&vertices[0], sizeof(r2_uint64), (r2_uc *)&vertices[1], sizeof(r2_uint64)) != NULL);
                pos++;
        } 

        assert(pos == 163257);
        fclose(fp);  
        r2_destroy_graph(graph);
        r2_graph_destroy_cc(forest);
}

/**
 * @brief Testing if a graph is biconnected.
 * 
 */
static void test_r2_graph_is_bcc()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        r2_uint64 edges[][2] = {
                {'G', 'H'},
                {'H', 'G'},
                {'H', 'J'},
                {'J', 'H'},
                {'G', 'J'},
                {'J', 'G'},
                {'J', 'F'},
                {'F', 'J'},
                {'G', 'F'},
                {'F', 'G'}, 
                {'H', 'I'},
                {'I', 'H'},
                {'F', 'I'},
                {'I', 'F'},
                {'F', 'C'},
                {'C', 'F'},
                {'C', 'E'},
                {'E', 'C'},
                {'F', 'E'}, 
                {'E', 'F'}
        };

        for(r2_uint64 i = 0; i < 20; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));
        
        assert(r2_graph_is_biconnected(graph) == FALSE);
        r2_destroy_graph(graph);

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        for(r2_uint64 i = 0; i < 6; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));
        
        assert(r2_graph_is_biconnected(graph) == TRUE);
        r2_destroy_graph(graph);

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        assert(r2_graph_is_biconnected(graph) == TRUE);

        r2_uint64 v = 'J'; 
        r2_graph_add_vertex(graph, (r2_uc *)&v, sizeof(r2_uint64));
        assert(r2_graph_is_biconnected(graph) == TRUE);
        r2_destroy_graph(graph);
}

static void test_r2_graph_stats()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        FILE *fp = fopen("web-NotreDame.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;

        clock_t before = clock(); 
        while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
        } 
        double elapsed = ( clock() - before )/CLOCKS_PER_SEC;
        
        before = clock(); 
        struct r2_graph *transpose = r2_graph_transpose(graph);
        elapsed = ( clock() - before )/CLOCKS_PER_SEC;
        
        before = clock(); 
        r2_destroy_graph(transpose);
        elapsed = ( clock() - before )/CLOCKS_PER_SEC;
        
        before = clock(); 
        struct r2_forest *forest = r2_graph_tscc(graph);
        elapsed = ( clock() - before )/CLOCKS_PER_SEC;
        r2_uint64 largest = 0;
        r2_uint64 pos = 0 ; 
       
        for(r2_uint64 i = 0; i < forest->ncount; ++i){
                if(forest->tree[i]->nvertices > largest){
                        largest = forest->tree[i]->nvertices;
                        pos = i;
                }
        } 

        printf("\nNumber of nodes in largest component and edges: %ld, %ld", largest, forest->tree[pos]->nedges);
        fclose(fp);
       
        
        before = clock(); 
        struct r2_forest *kcc = r2_graph_kcc(graph);
        elapsed = ( clock() - before )/CLOCKS_PER_SEC;
        largest = 0;
        pos = 0 ; 
       
        for(r2_uint64 i = 0; i < kcc->ncount; ++i){
                if(kcc->tree[i]->nvertices > largest){
                        largest = kcc->tree[i]->nvertices;
                        pos = i;
                }
        } 

        printf("\nNumber of nodes in largest component and edges: %ld, %ld", largest, kcc->tree[pos]->nedges);
        r2_graph_destroy_cc(kcc);
        r2_graph_destroy_cc(forest);
        r2_destroy_graph(graph);            
}

/**
 * @brief Testing articulation points.
 * 
 */
static void test_r2_graph_articulation_points()
{
        struct r2_listnode *head = NULL; 
        struct r2_vertex *v = NULL;
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        r2_uint64 edges[][2] = {
                {'G', 'H'},
                {'H', 'G'},
                {'H', 'J'},
                {'J', 'H'},
                {'G', 'J'},
                {'J', 'G'},
                {'J', 'F'},
                {'F', 'J'},
                {'G', 'F'},
                {'F', 'G'}, 
                {'H', 'I'},
                {'I', 'H'},
                {'F', 'I'},
                {'I', 'F'},
                {'F', 'C'},
                {'C', 'F'},
                {'C', 'E'},
                {'E', 'C'},
                {'E', 'D'}, 
                {'D', 'E'},
                {'D', 'C'},
                {'C', 'D'},
                {'C', 'M'}, 
                {'M', 'C'},
                {'C', 'K'}, 
                {'K', 'C'},
                {'C', 'B'},
                {'B', 'C'},
                {'B', 'M'},
                {'M', 'B'},
                {'B', 'A'},
                {'A', 'B'},
                {'A', 'K'},
                {'K', 'A'},
                {'K', 'L'},
                {'L', 'K'}
        };

        for(r2_uint64 i = 0; i < 36; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        struct r2_list *artpoints = r2_graph_articulation_points(graph);
        head = r2_listnode_first(artpoints);
        assert(artpoints->lsize == 3); 
        
        char cuts[3] = {'K', 'C', 'F'};
        r2_uint16 i = 0;
        printf("\nArticulation points: ");
        while(head != NULL){
                v = head->data; 
                printf(" %c", *v->vkey);
                assert(*v->vkey == cuts[i++]);
                head = head->next; 
        }
        r2_destroy_graph(graph);
        r2_destroy_list(artpoints);
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        FILE *fp = fopen("Email-Enron.txt", "r");
        struct r2_vertex *vertex[2];  
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;

        while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
        } 
        fclose(fp);

        artpoints = r2_graph_articulation_points(graph);
        struct r2_robintable *cut = r2_create_robintable(1, 1, 0, 0, .90, graph->vcmp, NULL, NULL, NULL, NULL, NULL);
        head = r2_listnode_first(artpoints);
        while(head != NULL){
                v = head->data; 
                r2_robintable_put(cut, v->vkey, v->vkey, v->len);
                head = head->next;
        }

        struct r2_entry entry;
        fp = fopen("cuts.txt", "r");
        while(fscanf(fp, "%lld", &vertices[0]) == 1){
                r2_robintable_get(cut, (r2_uc *)&vertices[0], sizeof(r2_uint64), &entry);
                assert(entry.key != NULL);
        } 
        fclose(fp);
        r2_destroy_robintable(cut);
        r2_destroy_list(artpoints);
        r2_destroy_graph(graph);
}

/**
 * @brief Tests the graph bridges.
 * 
 */
static void test_r2_graph_bridges()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        r2_uint64 edges[][2] = {
                {'G', 'H'},
                {'H', 'G'},
                {'H', 'J'},
                {'J', 'H'},
                {'G', 'J'},
                {'J', 'G'},
                {'J', 'F'},
                {'F', 'J'},
                {'G', 'F'},
                {'F', 'G'}, 
                {'H', 'I'},
                {'I', 'H'},
                {'F', 'I'},
                {'I', 'F'},
                {'F', 'C'},
                {'C', 'F'},
                {'C', 'E'},
                {'E', 'C'},
                {'E', 'D'}, 
                {'D', 'E'},
                {'D', 'C'},
                {'C', 'D'},
                {'C', 'M'}, 
                {'M', 'C'},
                {'C', 'K'}, 
                {'K', 'C'},
                {'C', 'B'},
                {'B', 'C'},
                {'B', 'M'},
                {'M', 'B'},
                {'B', 'A'},
                {'A', 'B'},
                {'A', 'K'},
                {'K', 'A'},
                {'K', 'L'},
                {'L', 'K'}
        };
        for(r2_uint64 i = 0; i < 36; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));
        
        struct r2_list *bridges = r2_graph_bridges(graph);

        struct r2_listnode *head = r2_listnode_first(bridges); 
        struct r2_edge *edge = NULL;
        printf("\nBridges: ");
        while(head != NULL){
                edge = head->data; 
                printf("[%c, %c] ", *edge->src->vkey,*edge->dest->vkey);
                head = head->next;
        }
        r2_destroy_list(bridges);
        r2_destroy_graph(graph); 
}

/**
 * @brief Test Dijkstra functionality
 * 
 */
static void test_r2_graph_dijkstra()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        struct r2_edge *edge = NULL;
        FILE *fp = fopen("dijkstra_1.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        r2_dbl val = 0;
        r2_dbl *w = NULL;
        r2_c nw;
        while(fscanf(fp, "%c\t%c\t%lf%c", &vertices[0], &vertices[1], &val, &nw) == 4){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }    
        fclose(fp);
        vertices[0] = 's';
        struct r2_graph *spt = r2_graph_dijkstra(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64), weight);
        printf("\nShortest Path Tree: "); 
        print_graph(spt);

        fp = fopen("dijkstra_results.txt", "r"); 
        while(fscanf(fp, "%c\t%lf%c", &vertices[1], &val, &nw) == 3){
                assert(val == r2_graph_dist_from_source(spt, (r2_uc *)&vertices[1], sizeof(r2_uint64)));
        }  

        fclose(fp);
        r2_destroy_graph(graph);
        r2_destroy_graph(spt);
          
}

/**
 * @brief Test Bellman ford functionality
 * 
 */
static void test_r2_graph_bellmanford()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        struct r2_edge *edge = NULL;
        FILE *fp = fopen("bellmanford.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        r2_dbl val = 0;
        r2_dbl *w = NULL;
        r2_c nw;
        while(fscanf(fp, "%c\t%c\t%lf%c", &vertices[0], &vertices[1], &val, &nw) == 4){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }    
        fclose(fp);
        vertices[0] = 's';
        struct r2_graph *spt = r2_graph_bellman_ford(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64), weight);
        printf("\nShortest Path Tree: "); 
        print_graph(spt);

        fp = fopen("bellmanford_results.txt", "r"); 
        while(fscanf(fp, "%c\t%lf%c", &vertices[1], &val, &nw) == 3){
                assert(val == r2_graph_dist_from_source(spt, (r2_uc *)&vertices[1], sizeof(r2_uint64)));
        }  
        fclose(fp);
        r2_destroy_graph(graph);
        r2_destroy_graph(spt);

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        fp = fopen("soc-sign-bitcoinotc.csv", "r");
        while(fscanf(fp, "%lld\t%lld\t%lf", &vertices[0], &vertices[1], &val) == 3){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }


        vertices[0] = 13; 
        spt = r2_graph_bellman_ford(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64), weight);
        r2_graph_spt_dump(spt, "bellman_dump.txt");
        fclose(fp);
        r2_destroy_graph(graph);
        r2_destroy_graph(spt);
}

/**
 * @brief Test the shortest path for a DAG.
 * 
 */
static void test_r2_graph_dag_shortest()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        struct r2_edge *edge = NULL;
        FILE *fp = fopen("dag2.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        r2_dbl val = 0;
        r2_dbl *w = NULL;
        r2_c nw;
        
        while(fscanf(fp, "%c\t%c\t%lf%c", &vertices[0], &vertices[1], &val, &nw) == 4){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }   

        fclose(fp);
        vertices[0] = '1';
        struct r2_graph *spt = r2_graph_shortest_dag(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64), weight);
        printf("\nShortest Path Tree: "); 
        print_graph(spt);

        fp = fopen("dijkstra_results2.txt", "r"); 
        while(fscanf(fp, "%c\t%lf%c", &vertices[1], &val, &nw) == 3){
                assert(val == r2_graph_dist_from_source(spt, (r2_uc *)&vertices[1], sizeof(r2_uint64)));
        }  
        fclose(fp);
        r2_destroy_graph(graph);
        r2_destroy_graph(spt);
}

/**
 * @brief Testing Prim MST functionality
 * 
 */
static void test_r2_graph_prim_mst()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        struct r2_edge *edge = NULL;
        FILE *fp = fopen("mst.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        r2_dbl val = 0;
        r2_dbl *w = NULL;
        r2_c nw;
        
        while(fscanf(fp, "%c\t%c\t%lf%c", &vertices[0], &vertices[1], &val, &nw) == 4){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }   
        fclose(fp);
        struct r2_graph *mst = r2_graph_mst_prim(graph, weight); 
        printf("\nPrim MST: ");
        print_graph(mst);
        r2_destroy_graph(mst); 
        r2_destroy_graph(graph);

        fp = fopen("CA.txt", "r");
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        while(fscanf(fp, "%c\t%lld\t%lld\t%lf\t%c", &nw, &vertices[0], &vertices[1], &val, &nw) == 5){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        } 

        mst = r2_graph_mst_prim(graph, weight); 
        r2_graph_mst_dump(mst, graph,"mstresults.txt", weight);
        r2_destroy_graph(mst); 
        r2_destroy_graph(graph);

}

/**
 * @brief Test Kruskal MST.
 * 
 */
static void test_r2_graph_kruskal_mst()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        struct r2_edge *edge = NULL;
        FILE *fp = fopen("mst.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;
        r2_dbl val = 0;
        r2_dbl *w = NULL;
        r2_c nw;
        
        while(fscanf(fp, "%c\t%c\t%lf\t%c", &vertices[0], &vertices[1], &val, &nw) == 4){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        }   

        fclose(fp);
        struct r2_graph *mst = r2_graph_mst_kruskal(graph, weight); 
        printf("\nKruskal MST: ");
        print_graph(mst);
        r2_destroy_graph(mst); 
        r2_destroy_graph(graph);


        fp = fopen("CA.txt", "r");
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        while(fscanf(fp, "%c\t%lld\t%lld\t%lf\t%c", &nw, &vertices[0], &vertices[1], &val, &nw) == 5){
                vertex[0] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, (r2_uc *)&vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = (r2_uint64 *)vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = (r2_uint64 *)vertex[1]->vkey;

                printf("\n%lld => %lld", vertices[0], vertices[1]);
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                edge = r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64));
                w = malloc(sizeof(r2_dbl));
                assert(w != NULL);
                *w = val;
                assert(r2_edge_add_attributes(edge, "weight", w, strlen("weight"), kcmp) == TRUE);
        } 
        mst = r2_graph_mst_kruskal(graph, weight); 
        r2_graph_mst_dump(mst, graph,"mstresults.txt", weight);
        r2_destroy_graph(mst); 
        r2_destroy_graph(graph);
}

/**
 * @brief Test graph copy.
 * 
 */
static void test_r2_graph_copy()
{
       struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
       FILE *fp = fopen("facebook_combined.txt" , "r");
       r2_uint64 a;
       r2_uint64 b;
       r2_uint64 *src; 
       r2_uint64 *dest; 
       while(fscanf(fp, "%lld\t%lld", &a, &b) == 2){
                printf("\n%lld=>%lld", a, b); 
                src  = malloc(sizeof(r2_uint64));
                dest = malloc(sizeof(r2_uint64));
                *src = a; 
                *dest= b;
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                assert(r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) != NULL);    
       }

       struct r2_graph *copy = r2_graph_copy(graph);
       struct r2_vertex *u = NULL; 
       struct r2_vertex *v = NULL;
       struct r2_listnode *head = r2_listnode_first(graph->vlist);
       while(head != NULL){
                u = head->data;
                v = r2_graph_get_vertex(copy, u->vkey, u->len);
                assert(v != NULL); 
                assert(u->vat == v->vat);
                head = head->next;
       }

       struct r2_edge *edge = NULL;
       head  = r2_listnode_first(graph->elist); 
       while(head != NULL){
                edge = head->data;
                u = edge->src; 
                v = edge->dest; 
                assert(r2_graph_get_edge(copy, u->vkey, u->len, v->vkey, v->len) != NULL);
                assert(r2_graph_get_edge(copy, u->vkey, u->len, v->vkey, v->len)->eat == edge->eat);
                head = head->next;
       }

       assert(graph->nvertices == copy->nvertices); 
       assert(graph->nedges == copy->nedges); 
       assert(graph->gat == copy->gat);
       fclose(fp);
       r2_destroy_graph(copy); 
       r2_destroy_graph(graph);
}

/**
 * @brief Test transitive closure.
 * 
 */
static void test_r2_transitive_closure()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        r2_uint64 edges[][2] = {
                {1, 2}, 
                {1, 3}, 
                {2, 4}, 
                {2, 5}, 
                {3, 1}, 
                {3, 6}, 
                {4, 6}, 
                {4, 3}, 
                {6, 5}
        };

        for(r2_uint64 i = 0; i < 9; ++i)
                r2_graph_add_edge(graph, (r2_uc *)&edges[i][0], sizeof(r2_uint64), (r2_uc *)&edges[i][1], sizeof(r2_uint64));

        printf("\n---------------------------Closure----------------------");
        print_graph(graph);
        struct r2_graph *closure = r2_graph_transitive_closure(graph);
        printf("\n---------------------------Graph Closure----------------------");
        print_graph(closure);
        r2_destroy_graph(closure);
        r2_destroy_graph(graph); 


        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        FILE *fp = fopen("CA-GrQc.txt" , "r");
        r2_uint64 a;
        r2_uint64 b;
        r2_uint64 *src; 
        r2_uint64 *dest; 
        while(fscanf(fp, "%lld\t%lld", &a, &b) == 2){
                printf("\n%lld=>%lld", a, b); 
                src  = malloc(sizeof(r2_uint64));
                dest = malloc(sizeof(r2_uint64));
                *src = a; 
                *dest= b;
                assert(r2_graph_add_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) == TRUE);
                assert(r2_graph_get_edge(graph, (r2_uc *)src, sizeof(r2_uint64), (r2_uc *)dest, sizeof(r2_uint64)) != NULL);
        }

        closure = r2_graph_transitive_closure(graph);
        fclose(fp);
        r2_destroy_graph(closure);
        r2_destroy_graph(graph); 
}

void test_r2_graph_run()
{
        test_r2_create_graph(); 
        test_r2_graph_add_vertex();
        test_r2_graph_get_vertex();
        test_r2_graph_del_vertex();
        test_r2_graph_add_edge();
        test_r2_graph_get_edge();
        test_r2_graph_del_edge();
        test_r2_graph_add_attributes();
        test_r2_graph_get_attributes();
        test_r2_graph_del_attributes();
        test_r2_vertex_add_attributes();
        test_r2_vertex_get_attributes();
        test_r2_vertex_del_attributes();
        test_r2_edge_add_attributes();
        test_r2_edge_get_attributes();
        test_r2_edge_del_attributes();
        test_r2_graph_bfs();
        test_r2_graph_dfs();
        test_r2_graph_transpose();
        test_r2_graph_has_cycle();
        test_r2_graph_traversals();
        test_graph_topological_sort();
        test_graph_topological_sort_edges();
        test_r2_graph_has_path();
        test_r2_graph_get_paths();
        test_r2_graph_get_paths_edges();
        test_r2_graph_bfs_tree();
        test_r2_graph_dfs_tree();
        test_r2_graph_children();
        test_r2_graph_strongly_connected();
        test_r2_graph_is_bipartite();
        test_r2_graph_bipartite_set();
        test_r2_graph_connected_components();
        test_r2_graph_scc();
        test_r2_graph_is_connected();
        test_r2_graph_path_tree();
        test_r2_graph_copy();
        test_r2_transitive_closure();
        //test_r2_graph_bcc();
        //test_r2_graph_is_bcc();
        //test_r2_graph_articulation_points();
        //test_r2_graph_bridges();
        //test_r2_graph_dijkstra();
        //test_r2_graph_bellmanford();
        //test_r2_graph_dag_shortest();
        //test_r2_graph_prim_mst();
        //test_r2_graph_kruskal_mst();
        //test_r2_graph_stats();
}


static r2_int16 vcmp(const void *a, const void *b)
{
        r2_uint64 *c = (r2_uint64 *)((struct r2_key *)a)->key; 
        r2_uint64 *d = (r2_uint64 *)((struct r2_key *)b)->key; 

        if(*c > *d)
                return 1; 
        else if(*c < *d)
                return -1; 

        return 0;
}


static r2_int16 kcmp(const void *a, const void *b)
{
        char *c = ((struct r2_key *)a)->key; 
        char *d = ((struct r2_key *)b)->key;
        return strcmp(c, d);
}

static void print_vertex(void *v, void *arg)
{
        struct r2_vertex *vertex = v; 
        printf("%d ", *(r2_uint64 *)vertex->vkey); 
}

static void print_graph(struct r2_graph *graph)
{
        struct r2_listnode *node = r2_listnode_first(graph->vlist);
        struct r2_vertex *vertex = NULL; 
        while(node != NULL){
                vertex = node->data; 
                printf("\n[%d]=>", *(r2_uint64 *)vertex->vkey);
                print_edges(vertex);
                node = node->next; 
        }
}

static void print_edges(struct r2_vertex *vertex)
{
        struct r2_edge     *edge = NULL;
        struct r2_listnode *node = r2_listnode_first(vertex->elist); 
        while(node != NULL){
                edge = node->data; 
                printf(" [%d, %d]", *(r2_uint64 *)edge->src->vkey, *(r2_uint64 *)edge->dest->vkey);
                node = node->next; 
        }
}

static void dumpcc(void *a, void *arg)
{
        FILE *fp = arg; 
        struct r2_vertex *source = a;
        
        fprintf(fp,"%lld\n", *(r2_uint64 *)source->vkey);
}

static void dump_edges(struct r2_graph *graph, char *fname)
{
        struct r2_listnode *head = r2_listnode_first(graph->elist); 
        struct r2_edge *edge = NULL; 
        FILE *fp = fopen(fname, "w");
        while(head != NULL){
                edge = head->data;
                fprintf(fp, "%lld\t%lld\n", *(r2_uint64 *)edge->src->vkey, *(r2_uint64 *)edge->dest->vkey);
                head = head->next; 
        }

        fclose(fp);
}


static r2_dbl weight(struct r2_edge *edge)
{
        r2_dbl *w = r2_edge_get_attributes(edge, "weight", strlen("weight"), kcmp);
        if(w == NULL)
                return 1; 
        
        return *w;
}


static void r2_graph_spt_dump(struct r2_graph *spt, char *fname)
{
        struct r2_listnode *head = r2_listnode_first(spt->vlist); 
        struct r2_vertex *v = NULL; 
        FILE *fp = fopen(fname, "w");
        while(head != NULL){
                v = head->data;
                fprintf(fp, "%lld\t%lf\n", *(r2_uint64 *)v->vkey, r2_graph_dist_from_source(spt, v->vkey, v->len));
                head = head->next; 
        }

        fclose(fp);
}

static void r2_graph_mst_dump(struct r2_graph *mst, struct r2_graph  *graph,char *fname, r2_weight weight)
{
        struct r2_listnode *head = r2_listnode_first(mst->elist); 
        struct r2_vertex *src = NULL;
        struct r2_vertex *dest = NULL;
        struct r2_edge *edge = NULL;  
        FILE *fp = fopen(fname, "w");
        while(head != NULL){
                edge = head->data;
                src  = edge->src; 
                dest = edge->dest;
                edge = r2_graph_get_edge(graph, src->vkey, src->len, dest->vkey, dest->len);
                fprintf(fp, "%lld\t%lld\t%lf\n", *(r2_uint64 *)src->vkey, *(r2_uint64 *)dest->vkey, weight(edge));
                head = head->next; 
        }

        fclose(fp);
}