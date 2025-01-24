#include "r2_graph_test.h"
#include <stdlib.h>
#include <assert.h> 
#include <stdio.h> 
#include <string.h>
#include <math.h>

static r2_int16 kcmp(const void *, const void *);
static r2_int16  vcmp(const void *, const void *);
static void print_edges(struct r2_vertex *);
static void print_graph(struct r2_graph *);
static void print_vertex(void *, void *);
static void print_bfstree(struct r2_bfsnode *, r2_uint64);
static void print_dfstree(struct r2_dfstree *);
static r2_ldbl relax(r2_ldbl , r2_ldbl );
static void print_dfstree_distances(struct r2_dfstree *);

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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
                
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, &vertices[i], sizeof(r2_uint64)); 
                assert(vertex->vkey == &vertices[i]);
        }

        r2_destroy_graph(graph);      
}

static void test_r2_graph_del_vertex()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

        struct r2_vertex *vertex = NULL; 
        for(r2_uint64 i = 0; i < 10; ++i){
                graph  = r2_graph_del_vertex(graph, &vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, &vertices[i], sizeof(r2_uint64)); 
                assert(vertex == NULL);
        }
        
        assert(graph->nvertices == 0);
        assert(graph->vlist->lsize == 0); 
        assert(graph->vertices->nsize == 0);

        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
       
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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        }
        for(r2_uint64 i = 0; i < 10; ++i){
                graph  = r2_graph_del_vertex(graph, &vertices[i], sizeof(r2_uint64));
                vertex = r2_graph_get_vertex(graph, &vertices[i], sizeof(r2_uint64)); 
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        assert(vertex->nedges == 4); 
        assert(vertex->elist->lsize  == 4);
        node = r2_listnode_first(vertex->elist);
        edge = node->data; 
        assert(edge->src->vkey  == &vertices[3]); 
        assert(edge->dest->vkey == &vertices[4]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == &vertices[3]); 
        assert(edge->dest->vkey == &vertices[5]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == &vertices[3]); 
        assert(edge->dest->vkey == &vertices[9]); 
        node = node->next;
        edge = node->data; 
        assert(edge->src->vkey  == &vertices[3]); 
        assert(edge->dest->vkey == &vertices[1]);
        node = node->next; 
        assert(node == NULL); 
        
        for(r2_uint64 i = 0; i < 20; ++i){
                edge  = r2_graph_get_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64));
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        

        struct r2_edge *edge = NULL;
        for(r2_uint64 i = 0; i < 20; ++i){
                graph = r2_graph_del_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64));
                edge  = r2_graph_get_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64));
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[0], sizeof(r2_uint64)); 
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[0], sizeof(r2_uint64)); 
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
        
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[0], sizeof(r2_uint64)); 
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, &edges[0][0], sizeof(r2_uint64), &edges[0][1], sizeof(r2_uint64));
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, &edges[0][0], sizeof(r2_uint64), &edges[0][1], sizeof(r2_uint64));
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        
        char *attribute[] = {
                "Romario",
                "Oniesh",
                "Newell"
        };

        struct r2_edge *edge = NULL; 
        edge  = r2_graph_get_edge(graph, &edges[0][0], sizeof(r2_uint64), &edges[0][1], sizeof(r2_uint64));
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
 * @brief Tests bfs functionality.
 * 
 */
static void test_r2_graph_bfs()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);

        printf("\nBFS: ");
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);

        printf("\nDFS: ");
        struct r2_vertex *vertex = r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        r2_graph_dfs(graph, vertex, print_vertex, NULL);
        r2_destroy_graph(graph);

}

/**
 * @brief Tests bfs tree functionality.
 * 
 */
static void test_r2_graph_bfs_tree()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);

        print_graph(graph);
        
        /*Source 4*/
        struct r2_vertex   *vertex =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_bfstree  *bfstree =  r2_graph_bfs_tree(graph, vertex);

        assert(*(bfstree->tree[0].vertex->vkey) == 4); 
        assert(*(bfstree->tree[1].vertex->vkey) == 5);
        assert(*(bfstree->tree[2].vertex->vkey) == 6);
        assert(*(bfstree->tree[3].vertex->vkey) == 10);
        assert(*(bfstree->tree[4].vertex->vkey) == 2);
        assert(*(bfstree->tree[5].vertex->vkey) == 1);
        assert(*(bfstree->tree[6].vertex->vkey) == 8); 
        assert(*(bfstree->tree[7].vertex->vkey) == 3);
        assert(*(bfstree->tree[8].vertex->vkey) == 9);
        assert(*(bfstree->tree[9].vertex->vkey) == 0);
        assert(*(bfstree->tree[10].vertex->vkey) == 7);
        assert(bfstree->tree[11].vertex == NULL); 
        print_bfstree(bfstree->tree, graph->nvertices);
        r2_destroy_bfs_tree(bfstree);
        /*Source 3*/
        vertex =   r2_graph_get_vertex(graph, &vertices[2], sizeof(r2_uint64));
        bfstree =  r2_graph_bfs_tree(graph, vertex);
        assert(*(bfstree->tree[0].vertex->vkey) == 3); 
        assert(*(bfstree->tree[1].vertex->vkey) == 9);
        assert(*(bfstree->tree[2].vertex->vkey) == 6);
        assert(*(bfstree->tree[3].vertex->vkey) == 2);
        assert(*(bfstree->tree[4].vertex->vkey) == 7);
        assert(*(bfstree->tree[5].vertex->vkey) == 5);
        assert(*(bfstree->tree[6].vertex->vkey) == 8); 
        assert(*(bfstree->tree[7].vertex->vkey) == 1);
        assert(*(bfstree->tree[8].vertex->vkey) == 0);
        assert(bfstree->tree[9].vertex == NULL); 
        print_bfstree(bfstree->tree, graph->nvertices);
        r2_destroy_bfs_tree(bfstree);
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
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);

        print_graph(graph);
        struct r2_vertex   *vertex =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_dfstree  *dfstree =  r2_graph_dfs_tree(graph, vertex);
        assert(*(dfstree->tree[0].vertex->vkey) == 4); 
        assert(*(dfstree->tree[1].vertex->vkey) == 5); 
        assert(*(dfstree->tree[2].vertex->vkey) == 1);
        assert(*(dfstree->tree[3].vertex->vkey) == 2); 
        assert(*(dfstree->tree[4].vertex->vkey) == 8); 
        assert(*(dfstree->tree[5].vertex->vkey) == 0); 
        assert(*(dfstree->tree[6].vertex->vkey) == 6); 
        assert(*(dfstree->tree[7].vertex->vkey) == 3); 
        assert(*(dfstree->tree[8].vertex->vkey) == 9);
        assert(*(dfstree->tree[9].vertex->vkey) == 7);
        assert(*(dfstree->tree[10].vertex->vkey) == 10); 
        assert(dfstree->tree[11].vertex == NULL); 
        print_dfstree(dfstree); 
        r2_destroy_dfs_tree(dfstree);
        vertex = r2_graph_get_vertex(graph, &vertices[2], sizeof(r2_uint64));
        dfstree =  r2_graph_dfs_tree(graph, vertex);
        assert(*(dfstree->tree[0].vertex->vkey) == 3); 
        assert(*(dfstree->tree[1].vertex->vkey) == 9); 
        assert(*(dfstree->tree[2].vertex->vkey) == 7);
        assert(*(dfstree->tree[3].vertex->vkey) == 8); 
        assert(*(dfstree->tree[4].vertex->vkey) == 0); 
        assert(*(dfstree->tree[5].vertex->vkey) == 1); 
        assert(*(dfstree->tree[6].vertex->vkey) == 2); 
        assert(*(dfstree->tree[7].vertex->vkey) == 5);
        assert(*(dfstree->tree[8].vertex->vkey) == 6);
        assert(dfstree->tree[9].vertex == NULL); 
        print_dfstree(dfstree); 
        r2_destroy_dfs_tree(dfstree);
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
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_vertex   *dest =   r2_graph_get_vertex(graph, &vertices[8], sizeof(r2_uint64));
        struct r2_list     *path =   r2_graph_has_path(graph, src, dest);

        assert(path != NULL);
        struct r2_listnode *head =   r2_listnode_first(path);
        printf("\nPath: ");
        
        /*4, 10, 9*/
        src = head->data;
        assert(src->vkey == &vertices[3]); 

        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[9]); 

        head = head->next;
        src = head->data; 
        assert(src->vkey == &vertices[8]); 

        head = head->next; 
        assert(head ==NULL); 
        head =   r2_listnode_first(path);
        while(head != NULL){
                src = head->data;
                printf("%d ", *src->vkey);
                head  = head->next; 
        }

        dest =   r2_graph_get_vertex(graph, &edges[6][1], sizeof(r2_uint64));
        r2_destroy_list(path);
        path =   r2_graph_has_path(graph, src, dest);
        assert(path == NULL);
        r2_destroy_graph(graph);
}

static void test_r2_graph_bfs_has_path_tree()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_vertex   *dest =   r2_graph_get_vertex(graph, &vertices[8], sizeof(r2_uint64));

        struct r2_bfstree *bfs   = r2_graph_bfs_tree(graph, src); 
        struct r2_list *path     = r2_graph_bfs_has_path_tree(bfs, src, dest);
        assert(path != NULL);
        struct r2_listnode *head =   r2_listnode_first(path);
        printf("\nPath Tree: ");
        
        /*4, 10, 9*/
        src = head->data;
        assert(src->vkey == &vertices[3]); 

        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[9]); 

        head = head->next;
        src = head->data; 
        assert(src->vkey == &vertices[8]); 

        head = head->next; 
        assert(head ==NULL); 
        head =   r2_listnode_first(path);
        while(head != NULL){
                src = head->data;
                printf("%d ", *src->vkey);
                head  = head->next; 
        }

        dest =   r2_graph_get_vertex(graph, &edges[6][1], sizeof(r2_uint64));
        r2_destroy_list(path);
        path     = r2_graph_bfs_has_path_tree(bfs, src, dest);
        assert(path == NULL);
        src  =   r2_graph_get_vertex(graph, &vertices[4], sizeof(r2_uint64));
        dest =   r2_graph_get_vertex(graph, &vertices[5], sizeof(r2_uint64));
        path = r2_graph_bfs_has_path_tree(bfs, src, dest);
        assert(path == NULL);
        r2_destroy_bfs_tree(bfs);
        r2_destroy_graph(graph); 

}

static void test_r2_graph_bfs_children()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_bfstree *bfs   =   r2_graph_bfs_tree(graph, src); 
        struct r2_list *children =   r2_graph_bfs_tree_children(bfs, src);
        struct r2_listnode *head =   r2_listnode_first(children);

        /*5, 6, 10, 2*/
        src = head->data; 
        assert(src->vkey == &vertices[4]); 

        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[5]); 

        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[9]); 
        
        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[1]);

        head = head->next;
        assert(head == NULL);

        head = r2_listnode_first(children);
        printf("\nBFS Children: ");
        while(head != NULL){
                src = head->data;
                printf("%d ", *src->vkey);
                head  = head->next; 
        }

        r2_destroy_list(children);
        r2_destroy_graph(graph); 
        r2_destroy_bfs_tree(bfs);
}

static void test_r2_graph_dfs_children()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_dfstree *dfs   =   r2_graph_dfs_tree(graph, src); 
        struct r2_list *children =   r2_graph_dfs_tree_children(dfs, src);
        struct r2_listnode *head =   r2_listnode_first(children);

        src = head->data;
        assert(src->vkey == &vertices[4]); 

        head = head->next; 
        src  = head->data; 
        assert(src->vkey == &vertices[5]); 

        head = head->next; 
        src = head->data;
        assert(src->vkey == &vertices[9]); 

        head = head->next; 
        assert(head == NULL);

        printf("\nDFS Children: ");
        head =   r2_listnode_first(children);
        while(head != NULL){
                src = head->data;
                printf("%d ", *src->vkey);
                head  = head->next; 
        }

        r2_destroy_list(children);
        r2_destroy_graph(graph); 
        r2_destroy_dfs_tree(dfs);


}


static void test_r2_graph_bfsnode_first()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_bfstree  *bfs  =   r2_graph_bfs_tree(graph, src); 
        struct r2_bfsnode  *root =   r2_graph_bfsnode_first(bfs); 
        assert(root->vertex == src);

        r2_destroy_bfs_tree(bfs);
        r2_destroy_graph(graph); 
}

static void test_r2_graph_dfsnode_first()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_dfstree  *dfs  =   r2_graph_dfs_tree(graph, src); 
        struct r2_dfsnode  *root =   r2_graph_dfsnode_first(dfs); 
        assert(root->vertex == src); 


        r2_destroy_dfs_tree(dfs);
        r2_destroy_graph(graph);        
}

static void test_r2_graph_dfsnode_prev()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_dfstree  *dfs  =   r2_graph_dfs_tree(graph, src); 
        struct r2_dfsnode  *root =   r2_graph_dfsnode_first(dfs); 
        root = r2_graph_dfsnode_next(dfs, root); 
        assert(r2_graph_dfsnode_prev(dfs,  r2_graph_dfsnode_first(dfs)) == NULL);
        assert(r2_graph_dfsnode_prev(dfs, root)->vertex == src);

        r2_destroy_dfs_tree(dfs);
        r2_destroy_graph(graph);         
}

static void test_r2_graph_bfsnode_prev()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);


        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_bfstree  *bfs  =   r2_graph_bfs_tree(graph, src); 
        struct r2_bfsnode  *root =   r2_graph_bfsnode_first(bfs); 
        root = r2_graph_bfsnode_next(bfs, root); 
        assert(r2_graph_bfsnode_prev(bfs,  r2_graph_bfsnode_first(bfs)) == NULL);
        assert(r2_graph_bfsnode_prev(bfs, root)->vertex == src);

        r2_destroy_bfs_tree(bfs);
        r2_destroy_graph(graph);                     
}

static void test_r2_graph_dfsnode_next()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));
        struct r2_dfstree  *dfs  =   r2_graph_dfs_tree(graph, src); 
        struct r2_dfsnode  *root =   r2_graph_dfsnode_first(dfs); 
        root = r2_graph_dfsnode_next(dfs, root); 
        assert(root->vertex->vkey == &vertices[4]);

        r2_destroy_dfs_tree(dfs);
        r2_destroy_graph(graph);               
}

static void test_r2_graph_bfsnode_next()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        struct r2_vertex   *src  =   r2_graph_get_vertex(graph, &vertices[3], sizeof(r2_uint64));  
        struct r2_bfstree  *bfs  =   r2_graph_bfs_tree(graph, src); 
        struct r2_bfsnode  *root =   r2_graph_bfsnode_first(bfs); 
        root = r2_graph_bfsnode_next(bfs, root);   
        assert(root->vertex->vkey == &vertices[4]);
        r2_destroy_bfs_tree(bfs);
        r2_destroy_graph(graph);            
}

static void test_r2_graph_connected_components()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  
        
        struct r2_components *forest = r2_graph_connected_components(graph);

        /*Verifying forest*/
        assert(forest->ncount == 4);
        struct r2_dfsnode *tree = forest->cc[0]->tree;
        assert(tree[0].vertex->vkey == &vertices[0]);
        assert(tree[1].vertex->vkey== &vertices[1]);
        assert(tree[2].vertex->vkey== &vertices[4]);
        assert(tree[3].vertex->vkey == &vertices[7]);
        assert(tree[4].vertex->vkey == &edges[4][0]);
        tree = forest->cc[1]->tree;
        assert(tree[0].vertex->vkey == &vertices[2]);
        assert(tree[1].vertex->vkey == &vertices[8]);
        assert(tree[2].vertex->vkey == &vertices[6]);
        assert(tree[3].vertex->vkey == &vertices[5]);
        tree = forest->cc[2]->tree;
        assert(tree[0].vertex->vkey == &vertices[3]);
        assert(tree[1].vertex->vkey == &vertices[9]);
        tree = forest->cc[3]->tree;
        assert(tree[0].vertex->vkey == &edges[6][0]);
        assert(tree[1].vertex->vkey == &edges[6][1]);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nConnected Component %d: ", i);
                print_dfstree(forest->cc[i]);
        }
                

        r2_graph_destroy_components(forest);
        r2_destroy_graph(graph);
}

static void test_r2_graph_is_connected()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  
        
        struct r2_components *forest = r2_graph_connected_components(graph);  
        struct r2_vertex *src  = r2_graph_get_vertex(graph, &vertices[0], sizeof(r2_uint64));
        struct r2_vertex *dest = r2_graph_get_vertex(graph, &vertices[1], sizeof(r2_uint64));
        struct r2_dfstree *component = r2_graph_is_connected(forest, src, dest);
        assert(component != NULL); 
        dest = r2_graph_get_vertex(graph, &vertices[8], sizeof(r2_uint64));
        component = r2_graph_is_connected(forest, src, dest);
        assert(component == NULL); 
        r2_graph_destroy_components(forest);
        r2_destroy_graph(graph);
}

static void test_r2_graph_bipartite_graph()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        /*Our graph is*/
        assert(r2_graph_is_bipartite(graph) == NULL); 
        r2_destroy_graph(graph);   
        
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 2; ++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        struct r2_bipartite *set = r2_graph_is_bipartite(graph);
        assert(set != NULL); 
        assert(set->sets[0]->lsize == 2); 
        assert(set->sets[1]->lsize == 2);
        struct r2_listnode *head = set->sets[0]->front; 
        assert(((struct r2_vertex *)head->data)->vkey == &edges[0][0]);
        head = head->next;      
        assert(((struct r2_vertex *)head->data)->vkey == &edges[1][0]);
        head = head->next;
        assert(head == NULL);
        head = set->sets[1]->front; 
        assert(((struct r2_vertex *)head->data)->vkey== &edges[0][1]);
        head = head->next;
        assert(((struct r2_vertex *)head->data)->vkey == &edges[1][1]);
        head = head->next;
        assert(head == NULL);
        r2_destroy_graph(graph);   
}

static void test_r2_graph_has_cycle()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        assert(r2_graph_has_cycle(graph) == TRUE);
        r2_destroy_graph(graph);    

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);
        
        
        assert(r2_graph_has_cycle(graph) != TRUE);
        r2_destroy_graph(graph);        
}

static void test_graph_topological_sort()
{
        r2_uint64 vertices[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

        assert(r2_graph_topological_sort(graph) == NULL); 
        r2_destroy_graph(graph);                
        
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL); 
        for(r2_uint64 i = 0; i < 4; ++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0); 
        
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
                graph = r2_graph_add_edge(graph, &tedges[i][0], sizeof(r2_uint64), &tedges[i][1], sizeof(r2_uint64), 0); 

        struct r2_list *topological = r2_graph_topological_sort(graph);
        struct r2_listnode *head = r2_listnode_first(topological); 
        struct r2_vertex *vertex = head->data;
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

        topological = r2_graph_dfs_traversals(graph, 2);

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
                graph = r2_graph_add_edge(graph, &tedges[i][0], sizeof(r2_uint64), &tedges[i][1], sizeof(r2_uint64), 0);

        topological = r2_graph_topological_sort(graph);
        assert(topological == NULL);
        r2_destroy_graph(graph);   
}

static void test_r2_graph_tarjan_strongly_connected_components()
{
        r2_uint64 vertices[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 8; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
        

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
                {'b', 'e'}
        };  

        for(r2_uint64 i = 0; i < 12; ++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);   

        struct r2_components* forest =   r2_graph_tarjan_strongly_connected_components(graph);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nTarjan Strongly Connected Component %d: ", i);
                print_dfstree(forest->cc[i]);
        }

        assert(forest->ncount == 4);
        struct r2_dfstree *tree = forest->cc[0];
        struct r2_dfsnode *root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'h');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);

        tree = forest->cc[1]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'g'); 
        root = r2_graph_dfsnode_next(tree,root); 
        assert(*(root->vertex->vkey) == 'f');
        root = r2_graph_dfsnode_next(tree,root); 
        assert(root == NULL);

        tree = forest->cc[2]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'c'); 
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'd');
        root = r2_graph_dfsnode_next(tree,root); 
        assert(root == NULL);

        tree = forest->cc[3]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'a'); 
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'b');
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'e');
        root = r2_graph_dfsnode_next(tree,root); 
        assert(root == NULL);
        r2_destroy_graph(graph);  
        r2_graph_destroy_components(forest);

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
                graph = r2_graph_add_edge(graph, &edge[i][0], sizeof(r2_uint64), &edge[i][1], sizeof(r2_uint64), 0);   

        forest = r2_graph_tarjan_strongly_connected_components(graph);
        assert(forest->ncount == 3);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nTarjan Strongly Connected Component %d: ", i);
                print_dfstree(forest->cc[i]);
        }


        tree = forest->cc[0];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '0');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '1');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '2');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '3');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);

        tree = forest->cc[1];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '7');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);

        tree = forest->cc[2];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '4');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '5');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '6');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);
        r2_destroy_graph(graph);  
        r2_graph_destroy_components(forest);
}


static void test_r2_graph_transpose()
{
        r2_uint64 vertices[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);  

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
        r2_destroy_graph(transpose); 
        r2_destroy_graph(graph);

}

static void test_r2_graph_traversals()
{
        r2_uint64 vertices[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));

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
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);

        struct r2_list* preorder = r2_graph_dfs_traversals(graph, 0);
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

        struct r2_list *postorder = r2_graph_dfs_traversals(graph, 1);

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

        struct r2_list *topological = r2_graph_dfs_traversals(graph, 2);
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

        r2_destroy_list(postorder);
        r2_destroy_graph(graph);
}

static void test_r2_graph_strongly_connected_components()
{
        r2_uint64 vertices[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 8; ++i)
                graph = r2_graph_add_vertex(graph, &vertices[i], sizeof(r2_uint64));
        

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
                {'b', 'e'}
        };  

        for(r2_uint64 i = 0; i < 12; ++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), 0);   

        struct r2_components* forest =   r2_graph_strongly_connected_components(graph);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nStrongly Connected Component %d: ", i);
                print_dfstree(forest->cc[i]);
        }

        assert(forest->ncount == 4);
        struct r2_dfstree *tree = forest->cc[0];
        struct r2_dfsnode *root = r2_graph_dfsnode_first(tree);

        
        tree = forest->cc[0]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'a'); 
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'e');
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'b');
        root = r2_graph_dfsnode_next(tree,root); 
        assert(root == NULL);

        tree = forest->cc[1]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'c'); 
        root = r2_graph_dfsnode_next(tree, root); 
        assert(*(root->vertex->vkey) == 'd');
        root = r2_graph_dfsnode_next(tree,root); 

        tree = forest->cc[2]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'g'); 
        root = r2_graph_dfsnode_next(tree,root); 
        assert(*(root->vertex->vkey) == 'f');
        root = r2_graph_dfsnode_next(tree,root); 
        assert(root == NULL);

        tree = forest->cc[3]; 
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == 'h');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);


        r2_destroy_graph(graph);  
        r2_graph_destroy_components(forest);

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
                graph = r2_graph_add_edge(graph, &edge[i][0], sizeof(r2_uint64), &edge[i][1], sizeof(r2_uint64), 0);   

        forest = r2_graph_strongly_connected_components(graph);
        assert(forest->ncount == 3);
        for(r2_uint16 i = 0; i < forest->ncount; ++i){
                printf("\nStrongly Connected Component 2 %d: ", i);
                print_dfstree(forest->cc[i]);
        }

        
        tree = forest->cc[0];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '4');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '6');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '5');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);

        tree = forest->cc[1];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '7');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);

        tree = forest->cc[2];
        root = r2_graph_dfsnode_first(tree);
        assert(*(root->vertex->vkey) == '0');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '3');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '2');
        root = r2_graph_dfsnode_next(tree, root);
        assert(*(root->vertex->vkey) == '1');
        root = r2_graph_dfsnode_next(tree, root);
        assert(root == NULL);



        r2_destroy_graph(graph);  
        r2_graph_destroy_components(forest);        
}

static void test_r2_graph_strongly_connected()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);   
        r2_uint64 edge[][2] = {
                {1, 2},
                {2, 1},
                {3, 1}
        };

        for(r2_uint64 i = 0; i < 2;++i)
                graph = r2_graph_add_edge(graph, &edge[i][0], sizeof(r2_uint64), &edge[i][1], sizeof(r2_uint64), 0);  
        
        assert(r2_graph_is_strong_connected(graph) == TRUE);
        r2_destroy_graph(graph); 

        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);  
        for(r2_uint64 i = 0; i < 3;++i)
                graph = r2_graph_add_edge(graph, &edge[i][0], sizeof(r2_uint64), &edge[i][1], sizeof(r2_uint64), 0);  


        assert(r2_graph_is_strong_connected(graph) == FALSE);
        r2_destroy_graph(graph); 
        
}

static void test_r2_graph_dijkstra()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        r2_uint64 edges [][2] = {
                {'s', 't'}, //10
                {'s', 'y'}, //5
                {'t', 'x'}, //1
                {'t', 'y'}, //2
                {'y', 't'}, //3
                {'y', 'z'}, //2
                {'y', 'x'}, //9
                {'x', 'z'}, //4
                {'z', 'x'}, //6
                {'z', 's'} //7
        };

        r2_ldbl weight [] = {10, 5, 1, 2, 3, 2, 9, 4, 6, 7};
        
        for(r2_uint64 i = 0; i < 10;++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), weight[i]);  

        struct r2_dfstree *dfs = r2_graph_dijkstra(graph, &edges[0][0], sizeof(r2_uint64), relax);
        print_dfstree_distances(dfs);

        struct r2_dfsnode *root = NULL;
        struct r2_entry entry   = {.key = NULL, .data = NULL, .length = 0};
        r2_uint64 vertex[5] = {'s', 't', 'y', 'x', 'z'};
        r2_ldbl   distances[5] = {0, 8, 5, 9, 7};
        for(r2_uint64 i = 0; i < 5; ++i){
                r2_robintable_get(dfs->positions, &vertex[i], sizeof(r2_uint64), &entry);
                root = entry.data;
                assert(root->dist == distances[i]);
        }

        weight[0] = 3; 
        weight[1] = 5;
        weight[2] = 6;
        weight[3] = 2; 
        weight[4] = 1;
        weight[5] = 6;
        weight[6] = 4; 
        weight[7] = 2; 
        weight[8] = 7; 
        weight[9] = 2; 
        r2_destroy_dfs_tree(dfs);
        r2_destroy_graph(graph);
        graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        for(r2_uint64 i = 0; i < 10;++i)
                graph = r2_graph_add_edge(graph, &edges[i][0], sizeof(r2_uint64), &edges[i][1], sizeof(r2_uint64), weight[i]);  


        dfs = r2_graph_dijkstra(graph, &edges[0][0], sizeof(r2_uint64), relax);
        print_dfstree_distances(dfs);
        distances[0] = 0; 
        distances[1] = 3; 
        distances[2] = 5; 
        distances[3] = 9;
        distances[4] = 11;
        for(r2_uint64 i = 0; i < 5; ++i){
                r2_robintable_get(dfs->positions, &vertex[i], sizeof(r2_uint64), &entry);
                root = entry.data;
                assert(root->dist == distances[i]);
        }
        r2_destroy_dfs_tree(dfs);
        r2_destroy_graph(graph);    
}

static void test_r2_graph_large_network()
{
        struct r2_graph *graph = r2_create_graph(vcmp, NULL, NULL, NULL, NULL);
        struct r2_vertex *vertex[2];  
        FILE *fp = fopen("Email-EuAll.txt", "r");
        r2_uint64 vertices[2];
        r2_uint64 *src  = NULL;
        r2_uint64 *dest = NULL;

        while(fscanf(fp, "%lld\t%lld", &vertices[0], &vertices[1]) == 2){
                vertex[0] = r2_graph_get_vertex(graph, &vertices[0], sizeof(r2_uint64));
                vertex[1] = r2_graph_get_vertex(graph, &vertices[1], sizeof(r2_uint64));
                if(vertex[0] == NULL){
                        src  = malloc(sizeof(r2_uint64));
                        assert(src != NULL);
                        *src = vertices[0];
                }else
                        src = vertex[0]->vkey;

                if(vertex[1] == NULL){
                        dest = malloc(sizeof(r2_uint64));
                        assert(dest != NULL);
                        *dest = vertices[1];
                }else 
                        dest = vertex[1]->vkey;

              //  printf("\n%lld => %lld", *src, *dest);
                graph = r2_graph_add_edge(graph, src, sizeof(r2_uint64), dest, sizeof(r2_uint64), 0);
        } 

        r2_uint64 s = 0;
        struct r2_dfstree *dfs = r2_graph_dfs_tree(graph, r2_graph_get_vertex(graph, &s, sizeof(r2_uint64)));
        struct r2_bfstree *bfs = r2_graph_bfs_tree(graph, r2_graph_get_vertex(graph, &s, sizeof(r2_uint64)));

        struct r2_components *scc = r2_graph_strongly_connected_components(graph);
        r2_uint64 largest = 0;
        r2_uint64 pos = 0 ; 
        for(r2_uint64 i = 0; i < scc->ncount; ++i){
                if(scc->cc[i]->ncount > largest){
                        largest = scc->cc[i]->ncount;
                        pos = i;
                }
        } 

        printf("\nNumber of nodes in largest component: %ld", largest);
        fclose(fp);
        r2_destroy_graph(graph);      
}


void r2_graph_run()
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
        test_r2_graph_bfs_tree();
        test_r2_graph_dfs_tree();
        test_r2_graph_has_path();
        test_r2_graph_bfs_has_path_tree();
        test_r2_graph_bfs_children();
        test_r2_graph_dfs_children();
        test_r2_graph_bfsnode_first();
        test_r2_graph_dfsnode_first();
        test_r2_graph_dfsnode_prev();
        test_r2_graph_bfsnode_prev();
        test_r2_graph_dfsnode_next();
        test_r2_graph_bfsnode_next();
        test_r2_graph_connected_components();
        test_r2_graph_is_connected();
        test_r2_graph_bipartite_graph();
        test_r2_graph_has_cycle();
        test_graph_topological_sort();
  //      test_r2_graph_tarjan_strongly_connected_components();
        test_r2_graph_transpose();
        test_r2_graph_traversals();
   //     test_r2_graph_strongly_connected_components();
        test_r2_graph_strongly_connected();
        test_r2_graph_dijkstra();
        test_r2_graph_large_network();
}




static r2_int16 vcmp(const void *a, const void *b)
{
        r2_uint64 *c = ((struct r2_key *)a)->key; 
        r2_uint64 *d = ((struct r2_key *)b)->key; 

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

static void print_vertex(void *v, void *arg)
{
        struct r2_vertex *vertex = v; 
        printf("%d ", *(r2_uint64 *)vertex->vkey); 
}

static void print_bfstree(struct r2_bfsnode *root, r2_uint64 size)
{
        printf("\n BFS Tree: ");
        for(r2_uint64 i = 0; i < size; ++i){
                if(root[i].vertex != NULL){
                        printf("\n%d ==>", *root[i].vertex->vkey); 
                        for(r2_uint64 j = root[i].children.start; j < root[i].children.end;++j){
                                if(root[j].vertex != NULL)
                                        printf("%d ", *root[j].vertex->vkey);
                        }
                }
        }
}

static void print_dfstree(struct r2_dfstree *dfs)
{
        struct r2_dfsnode  *root    = dfs->tree;
        struct r2_dfsnode  *child   = NULL;
        struct r2_vertex   *vertex  = NULL;
        struct r2_listnode *head    = NULL;
        struct r2_entry entry = {.key = NULL, .data = NULL, .length = 0};
        printf("\n DFS Tree: ");
        for(r2_uint64 i = 0; i < dfs->ncount; ++i){
                vertex = root[i].vertex;
                if(vertex != NULL){
                        printf("\n%ld ==>", *vertex->vkey); 
                        head = r2_listnode_first(vertex->out); 
                        while(head != NULL){
                                vertex = head->data; 
                                entry.key = NULL; 
                                entry.data = NULL; 
                                entry.length = 0;
                                r2_robintable_get(dfs->positions, vertex->vkey, vertex->len, &entry);
                                child = entry.data;
                                if(child != NULL){
                                        if(child->parent == root[i].pos)
                                              printf("%ld ", *vertex->vkey);   
                                }

                                head = head->next;
                        }
                }
        }
}

static r2_ldbl relax(r2_ldbl a, r2_ldbl b)
{
        return a + b;
}

static void print_dfstree_distances(struct r2_dfstree *dfs)
{
        struct r2_dfsnode *root  = dfs->tree; 
        struct r2_vertex *source = root->vertex; 
        struct r2_vertex *dest   = NULL;
        struct r2_list *path = NULL;
        struct r2_listnode *head = NULL;
        r2_ldbl dist = 0;
        printf("\nShortest Path Tree: ");
        for(r2_uint64 i = 0; i < dfs->ncount; ++i){
                dest = root[i].vertex;
                dist = root[i].dist;
                if(dist == 0 || dist == INFINITY)
                        continue;
                printf("\n%c", *source->vkey);
                path = r2_graph_dfs_has_path_tree(dfs, source, dest);
                head = r2_listnode_first(path); 
                while(head != NULL){
                        dest = head->data;
                        if(dest != source) 
                                printf("=>%c", *dest->vkey);
                        head = head->next; 
                }
                printf(" = %.2lf", (double)dist);
                r2_destroy_list(path);
        }
}