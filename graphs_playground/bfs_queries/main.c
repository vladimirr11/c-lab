#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <crtdbg.h>
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

typedef struct queue {
    node_t* start;
    node_t* end;
    size_t size;
} queue_t;

void create_slinked_list(slinked_list_t** list) {
    (*list) = (slinked_list_t*)malloc(sizeof(slinked_list_t));
    (*list)->head = (*list)->tail = NULL;
    (*list)->size = 0;
}

void insert_node_at_end(slinked_list_t** list, const char* data) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    size_t data_len = strlen(data) + 1;
    char* copy_data = (char*)malloc(data_len);
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

void sort_slinked_list(slinked_list_t* list) {
    if (!list->head) {
        return;
    }

    slinked_list_t* copy_list;
    create_slinked_list(&copy_list);

    node_t* temp = list->head;
    while (temp) {
        insert_node_at_end(&copy_list, temp->data);
        temp = temp->next;
    }

    free_list(list);
    insert_node_at_end(&list, copy_list->head->data);

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
        insert_node_at_end(&list, min->data);
        if (prev_min != min) {
            delete_slinked_list_node(prev_min, min);
        } else {
            curr_head = curr_head->next;
        }
    }

    // Free heap memory
    free_list(copy_list);
    free(copy_list);
}

bool data_in_list(slinked_list_t* list, char* data) {
    for (node_t* iter = list->head; iter != NULL; iter = iter->next) {
        if (strncmp(iter->data, data, 64) == 0) {
            return true;
        }
    }
    return false;
}

void print_slinked_list(const slinked_list_t* list) {
    for (node_t* nptr = list->head; nptr != NULL; nptr = nptr->next) {
        printf("%s - ", nptr->data);
    }
    printf("NULL\n");
}

void create_queue(queue_t** queue) {
    *queue = (queue_t*)malloc(sizeof(queue_t));
    (*queue)->start = (*queue)->end = NULL;
    (*queue)->size = 0;
}

void push_at_queue(queue_t** queue, char* data) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    size_t data_len = strlen(data) + 1;
    char* copy_data = (char*)malloc(data_len);
    strcpy(copy_data, data);
    new_node->data = copy_data;
    new_node->next = NULL;
    if ((*queue)->start == NULL) {
        (*queue)->start = new_node;
    } else {
        (*queue)->end->next = new_node;
    }

    (*queue)->end = new_node;
    (*queue)->size++;
}

char* pop_from_queue(queue_t* queue) {
    if (queue->size == 0) {
        return NULL;
    }

    char* return_data = queue->start->data;
    if (queue->size == 1) {
        queue->start = queue->end;
        free(queue->start);
        queue->start = queue->end = NULL;
        queue->size = 0;
        return return_data;
    }

    node_t* temp = queue->start;
    queue->start = queue->start->next;
    queue->size--;
    free(temp);

    return return_data;
}

void free_queue_data(char* data) { free(data); }

void free_queeu(queue_t* queue) {
    while (queue->size != 0) {
        char* data = pop_from_queue(queue);
        free_queue_data(data);
    }
}

void create_undirected_graph(undirected_graph_t** graph, int num_vertices) {
    *graph = (undirected_graph_t*)malloc(sizeof(undirected_graph_t));
    (*graph)->vertices_count = num_vertices;
    (*graph)->adjacency_lists = (slinked_list_t**)malloc(num_vertices * sizeof(slinked_list_t*));
}

void print_undirected_graph(const undirected_graph_t* graph) {
    printf("Unordered graph size: %llu\n", graph->vertices_count);
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

void bfs_graph(undirected_graph_t* graph, char* src_vertex) {
    queue_t* bfs_queue = NULL;
    create_queue(&bfs_queue);

    slinked_list_t* traversed_vert = NULL;
    create_slinked_list(&traversed_vert);

    push_at_queue(&bfs_queue, src_vertex);
    while (bfs_queue->size > 0) {
        char* vertex = pop_from_queue(bfs_queue);
        if (!data_in_list(traversed_vert, vertex)) {
            insert_node_at_end(&traversed_vert, vertex);
        }

        for (size_t i = 0; i < graph->vertices_count; i++) {
            node_t* curr_head = graph->adjacency_lists[i]->head;
            char* curr_head_data = curr_head->data;
            if (strncmp(curr_head_data, vertex, 64) == 0) {
                for (node_t* iter = curr_head->next; iter != NULL; iter = iter->next) {
                    if (!data_in_list(traversed_vert, iter->data)) {
                        push_at_queue(&bfs_queue, iter->data);
                    }
                }
                break;
            }
        }
        free_queue_data(vertex);
    }

    // Print the traversed vertices
    for (node_t* iter = traversed_vert->head; iter != NULL; iter = iter->next) {
        printf("%s ", iter->data);
    }
    printf("\n");

    // Free heap memory
    free_list(traversed_vert);
    free(traversed_vert);
    free(bfs_queue);
}

void process_bfs_queries(undirected_graph_t* graph, FILE* query_file) {
    char query_buffer[64];
    while (fgets(query_buffer, 64, query_file) != NULL) {
        query_buffer[strlen(query_buffer) - 1] = '\0';
        bfs_graph(graph, query_buffer);
    }
}

int32_t get_number_of_vertices(FILE* graph_file) {
    int32_t num_vertices = fgetc(graph_file) - 48;
    int32_t pos = ftell(graph_file);
    fseek(graph_file, ++pos, SEEK_CUR);
    return num_vertices;
}

int32_t main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Incorrect number of arguments: %i provided instead of 3\n", argc);
        exit(EXIT_FAILURE);
    }

    const char* graph_file_name = argv[1];
    const char* query_file_name = argv[2];

    FILE* graph_file = fopen(graph_file_name, "r");
    if (graph_file == NULL) {
        fprintf(stderr, "Cannot open %s graph file\n", graph_file_name);
        fclose(graph_file);
        exit(EXIT_FAILURE);
    }

    FILE* query_file = fopen(query_file_name, "r");
    if (query_file == NULL) {
        fprintf(stderr, "Cannot open %s quary file\n", query_file_name);
        fclose(graph_file);
        fclose(query_file);
        exit(EXIT_FAILURE);
    }

    int32_t num_vertices = get_number_of_vertices(graph_file);

    // Create empty graph
    undirected_graph_t* graph = NULL;
    create_undirected_graph(&graph, num_vertices);

    // Read unordered graph from file
    read_graph_from_file(graph, graph_file);

    // Sort the graph adjacency lists
    for (int32_t i = 0; i < num_vertices; i++) {
        sort_slinked_list(graph->adjacency_lists[i]);
    }

    // Print sorted graph
    print_undirected_graph(graph);

    // Process bfs queries
    process_bfs_queries(graph, query_file);

    // Free memory
    free_graph(graph);
    free(graph);

    // Close the opened streams
    fclose(graph_file);
    fclose(query_file);

    return 0;
}
