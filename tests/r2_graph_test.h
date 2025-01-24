#ifndef R2_GRAPH_TEST_H_
#define R2_GRAPH_TEST_H_
#include "../src/r2_graph.h"
static void test_r2_create_graph(); 
static void test_r2_destroy_graph();
static void test_r2_graph_add_vertex(); 
static void test_r2_graph_get_vertex();
static void test_r2_graph_del_vertex();
static void test_r2_graph_add_edge(); 
static void test_r2_graph_get_edge();
static void test_r2_graph_del_edge();
static void test_r2_graph_add_attributes(); 
static void test_r2_graph_del_attributes(); 
static void test_r2_graph_get_attributes();
static void test_r2_vertex_add_attributes();
static void test_r2_vertex_get_attributes();
static void test_r2_vertex_del_attributes(); 
static void test_r2_edge_add_attributes(); 
static void test_r2_edge_get_attributes(); 
static void test_r2_edge_del_attributes();
static void test_r2_graph_bfs();
static void test_r2_graph_dfs();
static void test_r2_graph_bfs_tree();
static void test_r2_graph_dfs_tree();
static void test_r2_graph_has_path();
static void test_r2_graph_bfs_has_path_tree();
static void test_r2_graph_bfs_children();
static void test_r2_graph_dfs_children();
static void test_r2_graph_dfsnode_first(); 
static void test_r2_graph_bfsnode_first(); 
static void test_r2_graph_dfsnode_prev(); 
static void test_r2_graph_bfsnode_prev(); 
static void test_r2_graph_dfsnode_next(); 
static void test_r2_graph_bfsnode_next(); 
static void test_r2_graph_connected_components();
static void test_r2_graph_is_connected();
static void test_r2_graph_bipartite_graph();
static void test_r2_graph_has_cycle();
static void test_graph_topological_sort();
static void test_r2_graph_tarjan_strongly_connected_components();
static void test_r2_graph_transpose();
static void test_r2_graph_traversals();
static void test_r2_graph_strongly_connected_components();
static void test_r2_graph_strongly_connected();
static void test_r2_graph_dijkstra();
static void test_r2_graph_large_network();
void r2_graph_run();
#endif