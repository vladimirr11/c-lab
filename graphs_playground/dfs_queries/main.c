#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef struct directed_graph {
    size_t num_vertices;
    slinked_list_t** adjacency_lists;
} directed_graph_t;

void create_slinked_list(slinked_list_t** list) {
    *list = (slinked_list_t*)malloc(sizeof(slinked_list_t));
    (*list)->head = (*list)->tail = NULL;
    (*list)->size = 0;
}

void insert_node_at_end(slinked_list_t** list, const char* vert_name, const int32_t dist) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    const int32_t vert_name_length = strlen(vert_name) + 1;
    char* copy_vert_name = (char*)malloc(vert_name_length);
    strcpy(copy_vert_name, vert_name);
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
        free(retire);
        return;
    }
    prev_node->next = NULL;
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

void dfs_graph(directed_graph_t* graph, node_t* src_vertex, slinked_list_t* visited_verts) {
    for (size_t i = 0; i < graph->num_vertices; i++) {
        node_t* head = graph->adjacency_lists[i]->head;
        if (strncmp(head->data, src_vertex->data, 32) == 0) {
            for (node_t* iter = head->next; iter != NULL; iter = iter->next) {
                if (!slinked_list_contains(visited_verts, iter->data)) {
                    insert_node_at_end(&visited_verts, iter->data, iter->dist);
                    dfs_graph(graph, iter, visited_verts);
                }
            }
            break;
        }
    }
}

void traverse_graph(directed_graph_t* graph) {
    slinked_list_t* visited_verts = NULL;
    create_slinked_list(&visited_verts);

    for (size_t i = 0; i < graph->num_vertices; i++) {
        node_t* curr_head = graph->adjacency_lists[i]->head;
        if (!slinked_list_contains(visited_verts, curr_head->data)) {
            insert_node_at_end(&visited_verts, curr_head->data, curr_head->dist);
            dfs_graph(graph, curr_head, visited_verts);
        }
    }

    // Print traversed vertices
    for (node_t* iter = visited_verts->head; iter != NULL; iter = iter->next) {
        printf("%s ", iter->data);
    }
    printf("\n");

    // Free the heap
    free_slinked_list(visited_verts);
    free(visited_verts);
}

int32_t get_number_of_vertices(FILE* graph_file) {
    const int32_t num_vertices = fgetc(graph_file) - 48;
    int32_t pos = ftell(graph_file);
    fseek(graph_file, ++pos, SEEK_CUR);
    return num_vertices;
}

int32_t main(int32_t argc, char** argv) {
    char* graph_file_name;
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments provided\n");
        exit(EXIT_FAILURE);
    }

    graph_file_name = argv[1];
    FILE* graph_file = fopen(graph_file_name, "r");
    if (!graph_file) {
        perror("fopen() failed for graph file");
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

    // Traverse the graph
    traverse_graph(graph);

    // Free graph memory
    free_directed_graph(graph);
    free(graph);

    // Close files
    fclose(graph_file);

    return 0;
}
