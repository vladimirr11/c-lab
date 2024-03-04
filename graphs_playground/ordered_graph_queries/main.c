#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct node {
    char* vert_name;
    int32_t dist;
    struct node* next;
} node_t;

typedef struct slinked_list {
    size_t size;
    node_t* head;
    node_t* tail;
} slinked_list_t;

typedef struct ordered_graph {
    size_t num_vertices;
    slinked_list_t** adjacency_lists;
} ordered_graph_t;

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
    new_node->vert_name = copy_vert_name;
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
        free(retire->vert_name);
        free(retire);
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

void print_slinked_list(slinked_list_t* list) {
    for (node_t* iter = list->head; iter != NULL; iter = iter->next) {
        printf("%s[%d] - ", iter->vert_name, iter->dist);
    }
    printf("NULL\n");
}

void create_ordered_graph(ordered_graph_t** graph, const size_t num_vertices) {
    *graph = (ordered_graph_t*)malloc(sizeof(ordered_graph_t));
    (*graph)->num_vertices = num_vertices;
    (*graph)->adjacency_lists = (slinked_list_t**)malloc(num_vertices * sizeof(slinked_list_t*));
    for (size_t i = 0; i < num_vertices; i++) {
        create_slinked_list(&(*graph)->adjacency_lists[i]);
    }
}

void free_ordered_graph(ordered_graph_t* graph) {
    for (size_t i = 0; i < graph->num_vertices; i++) {
        free_slinked_list(graph->adjacency_lists[i]);
        free(graph->adjacency_lists[i]);
    }
    free(graph->adjacency_lists);
    graph->num_vertices = 0;
}

void print_ordered_graph(ordered_graph_t* graph) {
    const size_t graph_size = graph->num_vertices;
    printf("Ordered graph size: %llu\n", graph_size);
    for (size_t i = 0; i < graph_size; i++) {
        print_slinked_list(graph->adjacency_lists[i]);
    }
}

void read_ordered_graph_from_file(ordered_graph_t** graph, FILE* graph_file) {
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
            const char* curr_list_head = (*graph)->adjacency_lists[i]->head->vert_name;
            if (strncmp(curr_list_head, edge_u, 32) == 0) {
                insert_node_at_end(&(*graph)->adjacency_lists[i], edge_v, edge_dist);
                break;
            }
        }
    }
}

void process_query(ordered_graph_t* graph, FILE* query_file) {
    char query_buffer[64];
    while (fgets(query_buffer, 64, query_file) != NULL) {
        size_t buffer_len = strlen(query_buffer);
        query_buffer[buffer_len - 1] = '\0';

        // Get the query type
        char query = query_buffer[0];
        char vertex[64];
        sscanf(&query_buffer[2], "%s", vertex);

        if (query == 'o') {
            for (size_t i = 0; i < graph->num_vertices; i++) {
                char* curr_head_vertex = graph->adjacency_lists[i]->head->vert_name;
                if (strncmp(curr_head_vertex, vertex, 64) == 0) {
                    printf("Out degree of vertex %s: %llu\n", vertex,
                           graph->adjacency_lists[i]->size - 1);
                    break;
                }
            }
        } else if (query == 'i') {
            int32_t in_degree = 0;
            for (size_t i = 0; i < graph->num_vertices; i++) {
                for (node_t* iter = graph->adjacency_lists[i]->head; iter != NULL;
                     iter = iter->next) {
                    if (strncmp(iter->vert_name, vertex, 64) == 0) {
                        in_degree++;
                    }
                }
            }
            printf("In degree of vertex %s: %d\n", vertex, in_degree - 1);
        }
    }
}

int32_t get_number_of_vertices(FILE* graph_file) {
    int32_t num_vertices = fgetc(graph_file) - 48;
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
    query_file_name = argv[2];

    FILE* graph_file = fopen(graph_file_name, "r");
    if (!graph_file) {
        perror("fopen() failed for graph file");
        exit(EXIT_FAILURE);
    }

    FILE* query_file = fopen(query_file_name, "r");
    if (!query_file) {
        perror("fopen() failed for query file");
        exit(EXIT_FAILURE);
    }

    // Read number of vertices in graph
    int32_t num_vertices = get_number_of_vertices(graph_file);

    // Create empty graph with given number of vertices
    ordered_graph_t* graph = NULL;
    create_ordered_graph(&graph, (size_t)num_vertices);

    // Read the graph from file
    read_ordered_graph_from_file(&graph, graph_file);
    // Print the read graph
    print_ordered_graph(graph);

    // Process queries
    process_query(graph, query_file);

    // Free graph memory
    free_ordered_graph(graph);
    free(graph);

    // Close files
    fclose(graph_file);
    fclose(query_file);

    return 0;
}
