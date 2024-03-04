#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct node {
    char* data;
    struct node* next;
} node_t;

typedef struct slinked_list {
    node_t* head;
    node_t* tail;
    size_t size;
} slinked_list_t;

typedef struct undirected_graph {
    size_t vertices_count;
    slinked_list_t** adjacency_lists;
} undirected_graph_t;

void create_slinked_list(slinked_list_t** list) {
    (*list) = (slinked_list_t*)malloc(sizeof(slinked_list_t));
    (*list)->head = (*list)->tail = NULL;
    (*list)->size = 0;
}

void insert_node_at_end(slinked_list_t** list, char* data) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    char* copy_data = (char*)malloc(strlen(data) + 1);
    strcpy(copy_data, data);
    new_node->data = copy_data;
    new_node->next = NULL;
    if ((*list)->head == NULL) {
        (*list)->head = new_node;
    } else {
        (*list)->tail->next = new_node;
    }

    (*list)->tail = new_node;
    (*list)->size++;
}

void delete_node(node_t* prev_node, node_t* node) {
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
    free(node);
}

void free_list(slinked_list_t* list) {
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

void sort_slinked_list(const slinked_list_t* list, slinked_list_t* list_out) {
    slinked_list_t* copy_list;
    create_slinked_list(&copy_list);

    node_t* temp = list->head;
    while (temp) {
        insert_node_at_end(&copy_list, temp->data);
        temp = temp->next;
    }

    node_t* curr_head = copy_list->head;
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
        insert_node_at_end(&list_out, min->data);
        if (prev_min != min) {
            delete_node(prev_min, min);
        } else {
            curr_head = curr_head->next;
        }
    }

    // Free heap memory
    free_list(copy_list);
    free(copy_list);
}

void print_slinked_list(const slinked_list_t* list) {
    for (node_t* nptr = list->head; nptr != NULL; nptr = nptr->next) {
        printf("%s - ", nptr->data);
    }
    printf("NULL\n");
}

void create_undirected_graph(undirected_graph_t** graph, int num_vertices) {
    *graph = (undirected_graph_t*)malloc(sizeof(undirected_graph_t));
    (*graph)->vertices_count = num_vertices;
    (*graph)->adjacency_lists = (slinked_list_t**)malloc(num_vertices * sizeof(slinked_list_t*));
}

void print_undirected_graph(const undirected_graph_t* graph) {
    printf("Undirected graph size: %llu\n", graph->vertices_count);
    for (size_t i = 0; i < graph->vertices_count; i++) {
        print_slinked_list(graph->adjacency_lists[i]);
    }
}

void read_graph_from_file(undirected_graph_t* graph, FILE* graph_file) {
    char vertex_buffer[50];
    for (size_t i = 0; i < graph->vertices_count; i++) {
        fgets(vertex_buffer, 50, graph_file);
        vertex_buffer[strlen(vertex_buffer) - 1] = '\0';
        create_slinked_list(&graph->adjacency_lists[i]);
        insert_node_at_end(&graph->adjacency_lists[i], vertex_buffer);
    }

    while (fgets(vertex_buffer, 50, graph_file) != NULL) {
        int32_t len_vertex_buffer = strlen(vertex_buffer) - 1;
        int32_t len_first_edge = 0;
        vertex_buffer[len_vertex_buffer] = '\0';
        while (vertex_buffer[len_first_edge] != ' ') {
            ++len_first_edge;
        }
        char edge_u[len_first_edge + 1];
        char edge_v[len_vertex_buffer - len_first_edge];
        for (int32_t i = 0; i < len_first_edge; i++) {
            edge_u[i] = vertex_buffer[i];
        }
        edge_u[len_first_edge] = '\0';
        for (int32_t i = 0, j = len_first_edge + 1; j < len_vertex_buffer; i++, j++) {
            edge_v[i] = vertex_buffer[j];
        }
        edge_v[len_vertex_buffer - len_first_edge - 1] = '\0';

        for (size_t i = 0; i < graph->vertices_count; i++) {
            char* curr_head_data = graph->adjacency_lists[i]->head->data;
            if (strcmp(curr_head_data, edge_u) == 0) {
                insert_node_at_end(&graph->adjacency_lists[i], edge_v);
            } else if (strcmp(curr_head_data, edge_v) == 0) {
                insert_node_at_end(&graph->adjacency_lists[i], edge_u);
            }
        }
    }
}

void free_graph(undirected_graph_t* graph) {
    for (size_t i = 0; i < graph->vertices_count; i++) {
        free_list(graph->adjacency_lists[i]);
        free(graph->adjacency_lists[i]);
    }
    free(graph->adjacency_lists);
    graph->vertices_count = 0;
}

void process_bfs_queries(const undirected_graph_t* graph, FILE* query_file) {
    char query_buffer[50];
    while (fgets(query_buffer, 50, query_file) != NULL) {
        int32_t query_lenght = strlen(query_buffer) - 1;
        query_buffer[query_lenght] = '\0';
        char query = query_buffer[0];
        char vertex[query_lenght - 1];
        for (int32_t i = 0, j = 2; j < query_lenght; i++, j++) {
            vertex[i] = query_buffer[j];
        }
        vertex[query_lenght - 2] = '\0';

        if (query == 'd') {
            for (size_t i = 0; i < graph->vertices_count; i++) {
                char* curr_head_data = graph->adjacency_lists[i]->head->data;
                if (strcmp(curr_head_data, vertex) == 0) {
                    printf("%llu\n", graph->adjacency_lists[i]->size - 1);
                    break;
                }
            }
        } else if (query == 'a') {
            for (size_t i = 0; i < graph->vertices_count; i++) {
                char* curr_head_data = graph->adjacency_lists[i]->head->data;
                if (strcmp(curr_head_data, vertex) == 0) {
                    slinked_list_t* sorted = NULL;
                    create_slinked_list(&sorted);
                    sort_slinked_list(graph->adjacency_lists[i], sorted);
                    print_slinked_list(graph->adjacency_lists[i]);
                    print_slinked_list(sorted);
                    free_list(sorted);
                    free(sorted);
                    break;
                }
            }
        }
    }
}

int32_t get_number_of_vertices(FILE* graph_file) {
    int32_t num_vertices = fgetc(graph_file) - 48;
    int32_t pos = ftell(graph_file);
    fseek(graph_file, ++pos, SEEK_CUR);
    return num_vertices;
}

int main(int argc, char* argv[]) {
    FILE* graph_file;
    FILE* query_file;

    if (argc != 3) {
        fprintf(stderr, "Incorrect number of argument: %i provided instead of 3\n", argc);
        return 1;
    }

    const char* graph_file_name = argv[1];
    const char* query_file_name = argv[2];

    graph_file = fopen(graph_file_name, "r");
    query_file = fopen(query_file_name, "r");
    if (graph_file == NULL) {
        fprintf(stderr, "Cannot open %s graph file\n", graph_file_name);
        return 2;
    }

    if (query_file == NULL) {
        fprintf(stderr, "Cannot open %s quary file\n", query_file_name);
        return 3;
    }

    int32_t num_vertices = get_number_of_vertices(graph_file);

    // Read graph from file
    undirected_graph_t* graph = NULL;
    create_undirected_graph(&graph, num_vertices);
    read_graph_from_file(graph, graph_file);

    // Process each query from file
    process_bfs_queries(graph, query_file);

    // Free heap memory
    free_graph(graph);
    free(graph);

    // Close the opened streams
    fclose(graph_file);
    fclose(query_file);

    return 0;
}
