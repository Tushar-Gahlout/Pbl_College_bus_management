#include "c_graph.h"

CGraph* c_graph_create(int max_vertices, bool is_directed) {
    if (max_vertices <= 0) max_vertices = 64;

    CGraph* g = (CGraph*)malloc(sizeof(CGraph));
    if (!g) return NULL;

    g->vertices = (CGraphVertex*)calloc((size_t)max_vertices, sizeof(CGraphVertex));
    if (!g->vertices) {
        free(g);
        return NULL;
    }

    g->num_vertices = 0;
    g->max_vertices = max_vertices;
    g->is_directed = is_directed;
    return g;
}

int c_graph_find_vertex(const CGraph* g, const char* stop_name) {
    if (!g || !stop_name) return -1;
    for (int i = 0; i < g->num_vertices; ++i) {
        if (g->vertices[i].stop_name && strcmp(g->vertices[i].stop_name, stop_name) == 0) {
            return i;
        }
    }
    return -1;
}

int c_graph_add_vertex(CGraph* g, const char* stop_name) {
    if (!g || !stop_name) return -1;

    int existing = c_graph_find_vertex(g, stop_name);
    if (existing >= 0) return existing;

    if (g->num_vertices >= g->max_vertices) return -1;

    int idx = g->num_vertices++;
    g->vertices[idx].stop_name = ds_strdup(stop_name);
    g->vertices[idx].head = NULL;
    return idx;
}

static CGraphEdge* c_graph_create_edge(int dest_idx, int dist_km, const char* route_id) {
    CGraphEdge* edge = (CGraphEdge*)malloc(sizeof(CGraphEdge));
    if (!edge) return NULL;
    edge->dest_vertex_idx = dest_idx;
    edge->weight_distance_km = dist_km;
    edge->route_id = route_id ? ds_strdup(route_id) : ds_strdup("Direct");
    edge->next = NULL;
    return edge;
}

static void c_graph_add_edge_directed(CGraph* g, int src, int dest, int dist_km, const char* route_id) {
    /* Check if edge already exists */
    CGraphEdge* curr = g->vertices[src].head;
    while (curr) {
        if (curr->dest_vertex_idx == dest) {
            curr->weight_distance_km = dist_km;
            return;
        }
        curr = curr->next;
    }

    CGraphEdge* new_edge = c_graph_create_edge(dest, dist_km, route_id);
    if (new_edge) {
        new_edge->next = g->vertices[src].head;
        g->vertices[src].head = new_edge;
    }
}

DS_Status c_graph_add_edge(CGraph* g, const char* src_stop, const char* dest_stop, int distance_km, const char* route_id) {
    if (!g || !src_stop || !dest_stop) return DS_ERR_NULL_PTR;

    int u = c_graph_add_vertex(g, src_stop);
    int v = c_graph_add_vertex(g, dest_stop);

    if (u < 0 || v < 0) return DS_ERR_FULL;

    c_graph_add_edge_directed(g, u, v, distance_km, route_id);
    if (!g->is_directed) {
        c_graph_add_edge_directed(g, v, u, distance_km, route_id);
    }
    return DS_SUCCESS;
}

static void c_graph_remove_edge_directed(CGraph* g, int src, int dest) {
    CGraphEdge* curr = g->vertices[src].head;
    CGraphEdge* prev = NULL;
    while (curr) {
        if (curr->dest_vertex_idx == dest) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g->vertices[src].head = curr->next;
            }
            free(curr->route_id);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

DS_Status c_graph_remove_edge(CGraph* g, const char* src_stop, const char* dest_stop) {
    if (!g || !src_stop || !dest_stop) return DS_ERR_NULL_PTR;

    int u = c_graph_find_vertex(g, src_stop);
    int v = c_graph_find_vertex(g, dest_stop);
    if (u < 0 || v < 0) return DS_ERR_NOT_FOUND;

    c_graph_remove_edge_directed(g, u, v);
    if (!g->is_directed) {
        c_graph_remove_edge_directed(g, v, u);
    }
    return DS_SUCCESS;
}

void c_graph_bfs(const CGraph* g, const char* start_stop, CGraphVisitor visitor, void* user_data) {
    if (!g || !start_stop || g->num_vertices == 0) return;

    int start_idx = c_graph_find_vertex(g, start_stop);
    if (start_idx < 0) return;

    bool* visited = (bool*)calloc((size_t)g->num_vertices, sizeof(bool));
    int* queue = (int*)malloc(sizeof(int) * (size_t)g->num_vertices);
    int* levels = (int*)malloc(sizeof(int) * (size_t)g->num_vertices);
    if (!visited || !queue || !levels) {
        free(visited); free(queue); free(levels);
        return;
    }

    int q_front = 0;
    int q_rear = 0;

    visited[start_idx] = true;
    queue[q_rear] = start_idx;
    levels[q_rear] = 0;
    q_rear++;

    while (q_front < q_rear) {
        int u = queue[q_front];
        int lvl = levels[q_front];
        q_front++;

        if (visitor) {
            visitor(g->vertices[u].stop_name, lvl, user_data);
        }

        CGraphEdge* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->dest_vertex_idx;
            if (!visited[v]) {
                visited[v] = true;
                queue[q_rear] = v;
                levels[q_rear] = lvl + 1;
                q_rear++;
            }
            edge = edge->next;
        }
    }

    free(visited);
    free(queue);
    free(levels);
}

static void c_graph_dfs_rec(const CGraph* g, int u, int depth, bool* visited, CGraphVisitor visitor, void* user_data) {
    visited[u] = true;
    if (visitor) {
        visitor(g->vertices[u].stop_name, depth, user_data);
    }

    CGraphEdge* edge = g->vertices[u].head;
    while (edge) {
        int v = edge->dest_vertex_idx;
        if (!visited[v]) {
            c_graph_dfs_rec(g, v, depth + 1, visited, visitor, user_data);
        }
        edge = edge->next;
    }
}

void c_graph_dfs(const CGraph* g, const char* start_stop, CGraphVisitor visitor, void* user_data) {
    if (!g || !start_stop || g->num_vertices == 0) return;

    int start_idx = c_graph_find_vertex(g, start_stop);
    if (start_idx < 0) return;

    bool* visited = (bool*)calloc((size_t)g->num_vertices, sizeof(bool));
    if (!visited) return;

    c_graph_dfs_rec(g, start_idx, 0, visited, visitor, user_data);
    free(visited);
}

bool c_graph_is_reachable(const CGraph* g, const char* start_stop, const char* target_stop) {
    if (!g || !start_stop || !target_stop) return false;

    int s = c_graph_find_vertex(g, start_stop);
    int t = c_graph_find_vertex(g, target_stop);
    if (s < 0 || t < 0) return false;
    if (s == t) return true;

    bool* visited = (bool*)calloc((size_t)g->num_vertices, sizeof(bool));
    int* queue = (int*)malloc(sizeof(int) * (size_t)g->num_vertices);
    if (!visited || !queue) {
        free(visited); free(queue);
        return false;
    }

    int q_front = 0;
    int q_rear = 0;

    visited[s] = true;
    queue[q_rear++] = s;
    bool found = false;

    while (q_front < q_rear) {
        int u = queue[q_front++];
        if (u == t) {
            found = true;
            break;
        }

        CGraphEdge* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->dest_vertex_idx;
            if (!visited[v]) {
                visited[v] = true;
                queue[q_rear++] = v;
            }
            edge = edge->next;
        }
    }

    free(visited);
    free(queue);
    return found;
}

int c_graph_vertex_count(const CGraph* g) {
    return g ? g->num_vertices : 0;
}

void c_graph_print(const CGraph* g) {
    if (!g) {
        printf("Graph: (NULL)\n");
        return;
    }
    printf("Bus Network Graph (Vertices: %d, %s):\n", g->num_vertices, g->is_directed ? "Directed" : "Undirected");
    for (int i = 0; i < g->num_vertices; ++i) {
        printf("  [%d] %-20s -> ", i, g->vertices[i].stop_name);
        CGraphEdge* edge = g->vertices[i].head;
        while (edge) {
            printf("%s (%d km, %s)%s",
                   g->vertices[edge->dest_vertex_idx].stop_name,
                   edge->weight_distance_km,
                   edge->route_id,
                   edge->next ? ", " : "");
            edge = edge->next;
        }
        printf("\n");
    }
}

void c_graph_destroy(CGraph* g) {
    if (!g) return;
    if (g->vertices) {
        for (int i = 0; i < g->num_vertices; ++i) {
            free(g->vertices[i].stop_name);
            CGraphEdge* curr = g->vertices[i].head;
            while (curr) {
                CGraphEdge* next = curr->next;
                free(curr->route_id);
                free(curr);
                curr = next;
            }
        }
        free(g->vertices);
    }
    free(g);
}
