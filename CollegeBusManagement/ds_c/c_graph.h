#ifndef C_GRAPH_H
#define C_GRAPH_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGraphEdge {
    int dest_vertex_idx;
    int weight_distance_km;
    char* route_id;
    struct CGraphEdge* next;
} CGraphEdge;

typedef struct {
    char* stop_name;
    CGraphEdge* head;
} CGraphVertex;

typedef struct {
    CGraphVertex* vertices;
    int num_vertices;
    int max_vertices;
    bool is_directed;
} CGraph;

typedef void (*CGraphVisitor)(const char* stop_name, int level, void* user_data);

CGraph* c_graph_create(int max_vertices, bool is_directed);
int c_graph_add_vertex(CGraph* g, const char* stop_name);
int c_graph_find_vertex(const CGraph* g, const char* stop_name);
DS_Status c_graph_add_edge(CGraph* g, const char* src_stop, const char* dest_stop, int distance_km, const char* route_id);
DS_Status c_graph_remove_edge(CGraph* g, const char* src_stop, const char* dest_stop);
bool c_graph_is_reachable(const CGraph* g, const char* start_stop, const char* target_stop);

void c_graph_bfs(const CGraph* g, const char* start_stop, CGraphVisitor visitor, void* user_data);
void c_graph_dfs(const CGraph* g, const char* start_stop, CGraphVisitor visitor, void* user_data);

int c_graph_vertex_count(const CGraph* g);
void c_graph_print(const CGraph* g);
void c_graph_destroy(CGraph* g);

#ifdef __cplusplus
}
#endif

#endif /* C_GRAPH_H */
