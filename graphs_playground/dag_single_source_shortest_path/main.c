#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

typedef struct node {
    char* data;
    int32_t dist;
    struct node* next;
} node_t;

typedef struct slinked_list {
    size_t size;
    node_t* head;
    node_t* tail;
} slinked_list_t;

typedef struct set {
    slinked_list_t* list;
} set_t;

typedef struct directed_graph {
    size_t num_vertices;
    slinked_list_t** adjacency_lists;
} directed_graph_t;

void create_slinked_list(slinked_list_t** list) {
    *list = (slinked_list_t*)malloc(sizeof(slinked_list_t));
    (*list)->head = (*list)->tail = NULL;
    (*list)->size = 0;
}

void insert_node_at_end(slinked_list_t** list, const char* data, const int32_t dist) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    const int32_t vert_name_length = strlen(data) + 1;
    char* copy_vert_name = (char*)malloc(vert_name_length);
    strcpy(copy_vert_name, data);
    new_node->data = copy_vert_name;
    new_node->dist = dist;
    new_node->next = NULL;

    if ((*list)->head == NULL) {
        (*list)->head = new_node;
    } else {
        (*list)->tail->next = new_node;
    }
    (*list)->tail = new_node;
    (*list)->size++;
}

void free_slinked_list(slinked_list_t* list) {
    node_t* temp = list->head;
    while (temp) {
        node_t* retire = temp;
        temp = temp->next;
        free(retire->data);
        free(retire);
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

void delete_slinked_list_node(node_t* prev_node, node_t* node) {
    if (node == NULL || prev_node == NULL) {
        fprintf(stderr, "NULL node provided for deletion");
        return;
    }

    if (node->next) {
        node_t* retire = node;
        node = node->next;
        prev_node->next = node;
        free(retire->data);
        free(retire);
        return;
    }
    prev_node->next = NULL;
    free(node->data);
    free(node);
}

void sort_slinked_list(slinked_list_t* list) {
    if (!list->head) {
        return;
    }

    slinked_list_t* copy_list;
    create_slinked_list(&copy_list);

    node_t* temp = list->head;
    while (temp) {
        insert_node_at_end(&copy_list, temp->data, temp->dist);
        temp = temp->next;
    }

    free_slinked_list(list);
    insert_node_at_end(&list, copy_list->head->data, copy_list->head->dist);

    if (!copy_list->head->next) {
        return;
    }

    node_t* curr_head = copy_list->head->next;
    while (curr_head) {
        node_t* iter = curr_head;
        node_t* prev_iter = curr_head;
        node_t* min = curr_head;
        node_t* prev_min = curr_head;
        while (iter) {
            int32_t rc = strcmp(iter->data, min->data);
            if (rc < 0) {
                prev_min = prev_iter;
                min = iter;
            }
            prev_iter = iter;
            iter = iter->next;
        }
        insert_node_at_end(&list, min->data, min->dist);
        if (prev_min != min) {
            delete_slinked_list_node(prev_min, min);
        } else {
            curr_head = curr_head->next;
        }
    }

    // Free heap memory
    free_slinked_list(copy_list);
    free(copy_list);
}

void reverse_slinked_list(slinked_list_t** list) {
    slinked_list_t* reversed_list = NULL;
    create_slinked_list(&reversed_list);

    for (int32_t i = (int32_t)(*list)->size - 1; i >= 0; i--) {
        node_t* iter = (*list)->head;
        int32_t c = 0;
        while (c != i) {
            iter = iter->next;
            c++;
        }
        insert_node_at_end(&reversed_list, iter->data, iter->dist);
    }

    free_slinked_list(*list);
    (*list) = reversed_list;
}

bool slinked_list_contains(slinked_list_t* list, const char* data) {
    node_t* iter = list->head;
    while (iter) {
        if (strncmp(iter->data, data, 32) == 0) {
            return true;
        }
        iter = iter->next;
    }
    return false;
}

void print_slinked_list(slinked_list_t* list) {
    for (node_t* iter = list->head; iter != NULL; iter = iter->next) {
        printf("%s[%d] - ", iter->data, iter->dist);
    }
    printf("NULL\n");
}

void create_set(set_t** set) {
    *set = (set_t*)malloc(sizeof(set_t));
    create_slinked_list(&(*set)->list);
}

void free_set(set_t* set) {
    free_slinked_list(set->list);
    set->list = NULL;
}

bool set_contains(set_t* set, const char* data) {
    if (slinked_list_contains(set->list, data)) {
        return true;
    }
    return false;
}

bool set_insert(set_t* set, node_t* node) {
    if (!set_contains(set, node->data)) {
        insert_node_at_end(&set->list, node->data, node->dist);
        return true;
    }
    return false;
}

bool set_remove(set_t* set, node_t* node) {
    if (!set_contains(set, node->data)) {
        return false;
    }
    node_t* iter = set->list->head;
    node_t* prev_iter = iter;
    while (iter) {
        if (strncmp(iter->data, node->data, 32) == 0) {
            if (iter == prev_iter) {  // We are at head
                node_t* retire = set->list->head;
                if (set->list->size == 1) {
                    set->list->head = set->list->tail = NULL;
                } else {
                    set->list->head = set->list->head->next;
                }
                free(retire->data);
                free(retire);
            } else {
                if (iter == set->list->tail) {  // We can be at tail
                    set->list->tail = prev_iter;
                }
                prev_iter->next = iter->next;
                free(iter->data);
                free(iter);
            }
            break;
        }
        prev_iter = iter;
        iter = iter->next;
    }

    set->list->size--;
    return true;
}

void create_directed_graph(directed_graph_t** graph, const size_t num_vertices) {
    *graph = (directed_graph_t*)malloc(sizeof(directed_graph_t));
    (*graph)->num_vertices = num_vertices;
    (*graph)->adjacency_lists = (slinked_list_t**)malloc(num_vertices * sizeof(slinked_list_t*));
    for (size_t i = 0; i < num_vertices; i++) {
        create_slinked_list(&(*graph)->adjacency_lists[i]);
    }
}

void free_directed_graph(directed_graph_t* graph) {
    for (size_t i = 0; i < graph->num_vertices; i++) {
        free_slinked_list(graph->adjacency_lists[i]);
        free(graph->adjacency_lists[i]);
    }
    free(graph->adjacency_lists);
    graph->num_vertices = 0;
}

void print_directed_graph(directed_graph_t* graph) {
    const size_t graph_size = graph->num_vertices;
    printf("Ordered graph size: %llu\n", graph_size);
    for (size_t i = 0; i < graph_size; i++) {
        print_slinked_list(graph->adjacency_lists[i]);
    }
}

void read_graph_from_file(directed_graph_t** graph, FILE* graph_file) {
    for (size_t i = 0; i < (*graph)->num_vertices; i++) {
        char vertex_buffer[64];
        fgets(vertex_buffer, 64, graph_file);
        vertex_buffer[strlen(vertex_buffer) - 1] = '\0';
        const int32_t head_dist = -1;
        insert_node_at_end(&(*graph)->adjacency_lists[i], vertex_buffer, head_dist);
    }

    char edge_buffer[64];
    while (fgets(edge_buffer, 64, graph_file) != NULL) {
        edge_buffer[strlen(edge_buffer) - 1] = '\0';
        char edge_u[32];
        char edge_v[32];

        // Get the lenght of the name of the first vertex
        int32_t len_name_first_edge = 0;
        while (edge_buffer[len_name_first_edge] != ' ') {
            ++len_name_first_edge;
        }
        // Get the lenght of the name of the second vertex
        int32_t len_name_second_edge = 0;
        while (edge_buffer[len_name_first_edge + len_name_second_edge + 1] != ' ') {
            ++len_name_second_edge;
        }

        // Get the distance between both vertices
        int32_t edge_dist = 0;
        sscanf(&edge_buffer[len_name_first_edge + len_name_second_edge + 2], "%d", &edge_dist);

        // Read the first vertex
        for (int32_t i = 0; i < len_name_first_edge; i++) {
            edge_u[i] = edge_buffer[i];
        }
        edge_u[len_name_first_edge] = '\0';

        // Read the second vertex
        for (int32_t i = 0; i < len_name_second_edge; i++) {
            edge_v[i] = edge_buffer[i + len_name_first_edge + 1];
        }
        edge_v[len_name_second_edge] = '\0';

        // Insert the second vertex into the adjacency list of the first
        for (size_t i = 0; i < (*graph)->num_vertices; i++) {
            const char* curr_list_head = (*graph)->adjacency_lists[i]->head->data;
            if (strncmp(curr_list_head, edge_u, 32) == 0) {
                insert_node_at_end(&(*graph)->adjacency_lists[i], edge_v, edge_dist);
                break;
            }
        }
    }
}

bool dfs_topological_sort(directed_graph_t* graph, node_t* src_vertex,
                          slinked_list_t* visited_verts, set_t* cycle_verts,
                          slinked_list_t* sorted_verts) {
    if (set_contains(cycle_verts, src_vertex->data)) {
        return false;  // There is a cycle in the graph
    }
    if (!slinked_list_contains(visited_verts, src_vertex->data)) {
        insert_node_at_end(&visited_verts, src_vertex->data, src_vertex->dist);
        set_insert(cycle_verts, src_vertex);
        for (size_t i = 0; i < graph->num_vertices; i++) {
            node_t* curr_head = graph->adjacency_lists[i]->head;
            if (strncmp(curr_head->data, src_vertex->data, 32) == 0) {
                for (node_t* iter = curr_head->next; iter != NULL; iter = iter->next) {
                    dfs_topological_sort(graph, iter, visited_verts, cycle_verts, sorted_verts);
                }
                break;
            }
        }
        set_remove(cycle_verts, src_vertex);
        insert_node_at_end(&sorted_verts, src_vertex->data, src_vertex->dist);
    }
    return true;
}

bool graph_topological_sort(directed_graph_t* graph, slinked_list_t* sorted_verts_out) {
    slinked_list_t* visited_verts;
    set_t* cycle_verts;
    create_slinked_list(&visited_verts);
    create_set(&cycle_verts);

    bool cycle_free = true;
    for (size_t i = 0; i < graph->num_vertices; i++) {
        node_t* curr_vertex = graph->adjacency_lists[i]->head;
        if (!slinked_list_contains(visited_verts, curr_vertex->data)) {
            if (!dfs_topological_sort(graph, curr_vertex, visited_verts, cycle_verts,
                                      sorted_verts_out)) {
                cycle_free = false;
                break;
            }
        }
    }

    // Free the heap
    free_slinked_list(visited_verts);
    free_set(cycle_verts);
    free(visited_verts);
    free(cycle_verts);

    return cycle_free;
}

int32_t get_distance(slinked_list_t* top_sorted_verts, const int32_t* distances,
                     const char* src_vertex) {
    int32_t c = 0;
    for (node_t* iter = top_sorted_verts->head; iter != NULL; iter = iter->next) {
        if (strncmp(iter->data, src_vertex, 32) == 0) {
            break;
        }
        c++;
    }
    return distances[c];
}

void update_distance(slinked_list_t* top_sorted_verts, int32_t* distances, const char* src_vertex,
                     const int32_t dist) {
    int32_t c = 0;
    for (node_t* iter = top_sorted_verts->head; iter != NULL; iter = iter->next) {
        if (strncmp(iter->data, src_vertex, 32) == 0) {
            distances[c] = dist;
            return;
        }
        c++;
    }
}

int32_t get_weight(directed_graph_t* graph, const node_t* u_vert, const node_t* v_vert) {
    for (size_t i = 0; i < graph->num_vertices; i++) {
        node_t* head = graph->adjacency_lists[i]->head;
        if (strncmp(head->data, u_vert->data, 32) == 0) {
            for (node_t* iter = head->next; iter != NULL; iter = iter->next) {
                if (strncmp(iter->data, v_vert->data, 32) == 0) {
                    return iter->dist;
                }
            }
            break;
        }
    }
    return INT32_MAX - 100000;
}

void run_bellman_ford_shortest_path(directed_graph_t* graph, const char* src_vertex) {
    slinked_list_t* top_sorted_verts;
    create_slinked_list(&top_sorted_verts);

    // Sort the graph topologically
    bool is_cycle_free = graph_topological_sort(graph, top_sorted_verts);
    if (!is_cycle_free) {
        printf("Cycle detected\n");
        return;
    }

    // Reverse the topologically sorted list
    reverse_slinked_list(&top_sorted_verts);

    // Initialize the distances array to infinity
    int32_t distances[top_sorted_verts->size];
    for (size_t i = 0; i < graph->num_vertices; i++) {
        distances[i] = INT32_MAX - 100000;
    }

    // Update the source vertex to distance 0
    update_distance(top_sorted_verts, distances, src_vertex, 0);

    // Update the rest of the distances
    for (node_t* u_vert = top_sorted_verts->head; u_vert != NULL; u_vert = u_vert->next) {
        const int32_t u_vert_dist = get_distance(top_sorted_verts, distances, u_vert->data);
        for (size_t i = 0; i < graph->num_vertices; i++) {
            if (strncmp(u_vert->data, graph->adjacency_lists[i]->head->data, 32) == 0) {
                node_t* curr_head = graph->adjacency_lists[i]->head;
                for (node_t* v_vert = curr_head->next; v_vert != NULL; v_vert = v_vert->next) {
                    const int32_t v_vert_dist =
                        get_distance(top_sorted_verts, distances, v_vert->data);
                    const int32_t weight_u_v = get_weight(graph, u_vert, v_vert);
                    if (v_vert_dist > u_vert_dist + weight_u_v) {
                        update_distance(top_sorted_verts, distances, v_vert->data,
                                        u_vert_dist + weight_u_v);
                    }
                }
                break;
            }
        }
    }

    // Print the findings
    int32_t c = 0;
    for (node_t* iter = top_sorted_verts->head; iter != NULL; iter = iter->next) {
        if (distances[c] == INT32_MAX - 100000) {
            printf("%s INF\n", iter->data);

        } else {
            printf("%s %d\n", iter->data, distances[c]);
        }
        c++;
    }
    printf("\n");

    // Free the heap
    free_slinked_list(top_sorted_verts);
    free(top_sorted_verts);
}

void process_single_source_shortest_path_queries(directed_graph_t* graph, FILE* query_file) {
    char query_buffer[32];
    while (fgets(query_buffer, 64, query_file) != NULL) {
        query_buffer[strlen(query_buffer) - 1] = '\0';
        run_bellman_ford_shortest_path(graph, query_buffer);
    }
}

int32_t get_number_of_vertices(FILE* graph_file) {
    const int32_t num_vertices = fgetc(graph_file) - 48;
    int32_t pos = ftell(graph_file);
    fseek(graph_file, ++pos, SEEK_CUR);
    return num_vertices;
}

int32_t main(int32_t argc, char** argv) {
    char *graph_file_name, *query_file_name;
    if (argc != 3) {
        fprintf(stderr, "Incorrect number of arguments provided\n");
        exit(EXIT_FAILURE);
    }

    graph_file_name = argv[1];
    FILE* graph_file = fopen(graph_file_name, "r");
    if (!graph_file) {
        perror("fopen() failed for graph file");
        exit(EXIT_FAILURE);
    }

    query_file_name = argv[2];
    FILE* query_file = fopen(query_file_name, "r");
    if (!query_file) {
        perror("fopen() failed for query file");
        exit(EXIT_FAILURE);
    }

    // Read number of vertices in graph
    int32_t num_vertices = get_number_of_vertices(graph_file);

    // Create empty graph with given number of vertices
    directed_graph_t* graph = NULL;
    create_directed_graph(&graph, (size_t)num_vertices);

    // Read the graph from file
    read_graph_from_file(&graph, graph_file);

    // Sort the graph adjacency lists
    for (int32_t i = 0; i < num_vertices; i++) {
        sort_slinked_list(graph->adjacency_lists[i]);
    }

    // Print the read graph
    print_directed_graph(graph);

    // Process queries
    process_single_source_shortest_path_queries(graph, query_file);

    // Free graph memory
    free_directed_graph(graph);
    free(graph);

    // Close files
    fclose(graph_file);
    fclose(query_file);

    return 0;
}
