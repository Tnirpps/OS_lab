#include <stdio.h>
#include <pthread.h>
#include "zmq_tools.h"

int NODES[MAX_NODES];

// indexing from 1
int tree[MAX_TREE_SIZE];

pthread_mutex_t mutex;

typedef struct {
    const int*   count;
    useconds_t   timeout;
} ping_token;

void* pinging(void* arg);
void start_ping_all(ping_token* token);
void stop_all_nodes(char* addr, message token, int count);
void send_receive_msg(message* token);
void node_append(int value);

int find_creator(int id);
int split_copy(const char* text, char* dest, int index);
bool node_exist(int value, int count);

int main (int argc, char const *argv[])  {
    bool heartbit = false;
    int nodes_count = 0;
    char query_line [MAX_LEN];
    char query_word[MAX_LEN];
    char query_str_int[MAX_LEN];
    char addr[MAX_LEN] = SERVER_SOCKET_PATTERN;

    printf("[%d] started\n", getpid());

    message token = {create, 0, 0, ""};
    ping_token ping_arg = {0, 1000000};

    for (int i = 0; i < MAX_TREE_SIZE; ++i) {
        tree[i] = -1;    // init tree with -1 value for each Node
    }

    while (fgets(query_line, MAX_LEN, stdin) != NULL) {
        // get Command from query line
        if (split_copy(query_line, query_word, 0) == 0) {
            printf("\t bad command\n");
            continue;
        }
        // get command's Argument from query line
        if (split_copy(query_line, query_str_int, 1) == 0) {
            printf("\t bad command\n");
            continue;
        }
        long int query_int = strtol(query_str_int, NULL, 10);
        if(query_int == LONG_MAX || query_int < 0) {
            printf("\t bad command's arg: it's too large or negative\n");
            continue;
        }

        if (strcmp(query_word, "create") == 0) {
            if (nodes_count == MAX_NODES) {
                printf("\tyou cannot create more than %d nodes\n", MAX_NODES);
                continue;
            }
            // (query int + MIN_ADDR <= 9999) means: id_process have no more than 4 digits
            if (query_int > 9999 - MIN_ADDR) {
                printf("\t bad command's arg: it's too large\n");
                continue;
            }
            int id_process = (int)query_int + MIN_ADDR;
            if (node_exist(id_process, nodes_count)) {
                printf("\t this node was created earlier\n");
                continue;
            }

            int tree_index = find_creator(id_process);
            if (tree_index != 0) {
                int parent_to_create_proc = tree[tree_index];
                if (ping_process(parent_to_create_proc)) {
                    // send msg to create
                    token.cmd = create;
                    token.dest_id = parent_to_create_proc;
                    token.value = id_process;
                    printf("\t ask %d to create a new node\n", parent_to_create_proc - MIN_ADDR);
                    send_receive_msg(&token);
                    node_append(id_process);
                    NODES[nodes_count] = id_process;
                    ++nodes_count;
                } else {
                    printf("\t cannot create a new node: parent is not available\n");
                    continue;
                }
            } else {
                // create by itself
                printf("\t I'm creating %d\n", id_process - MIN_ADDR);
                memset(query_str_int, 0, MAX_LEN);
                sprintf(query_str_int, "%d", id_process);
                char *Child_argv[] = {"node", query_str_int, NULL};
                int pid = fork();
                if (pid == -1) {
                    printf("\t fork error\n");
                    return 1;
                }
                if (pid == 0) {
                    execv("node", Child_argv);
                    return 0;
                }
                NODES[nodes_count] = id_process;
                node_append(id_process);
                ++nodes_count;
            }

        } else if (strcmp(query_word, "exec") == 0) {
            if (query_int > 9999 - MIN_ADDR) {
                printf("\t bad command's arg: it's too large\n");
                continue;
            }
            int id_process = (int)query_int + MIN_ADDR;
            if (node_exist(id_process, nodes_count) && ping_process(id_process)) {
                clear_token(&token);
                fgets(token.str, MAX_LEN, stdin);
                token.str[strlen(token.str) - 1] = '\0';
                token.cmd = exec;
                token.dest_id = id_process;
                send_receive_msg(&token);
            } else {
                printf("\t [%d] node hasn't connection\n", id_process - MIN_ADDR);
            }
        } else if (strcmp(query_word, "remove") == 0) {
            if (query_int > MIN_ADDR) {
                printf("\t bad command's arg: it's too large\n");
                continue;
            }
            int id_process = (int)query_int + MIN_ADDR;
            if (node_exist(id_process, nodes_count) && ping_process(id_process)) {
                token.cmd = delete;
                token.dest_id = id_process;
                send_receive_msg(&token);
            } else {
                printf("\t [%d] node hasn't connection\n", id_process - MIN_ADDR);
            }
        } else if (strcmp(query_word, "heartbit") == 0) {
            // only once can be started
            if (heartbit == true) continue;
            ping_arg.count = &nodes_count;
            ping_arg.timeout = (useconds_t) query_int;
            start_ping_all(&ping_arg);
            heartbit = true;
        } else {
            printf("\t bad command. Please, try again\n");
        }
        memset(query_line, 0, MAX_LEN);
        memset(query_word, 0, MAX_LEN);
        memset(query_str_int, 0, MAX_LEN);
    }
    stop_all_nodes(addr, token, nodes_count);
    return 0;
}

// get index-th word from text to dest
int split_copy(const char* text, char* dest, int index) {
    int i = 0;
    for (int j = 0; j < index; ++j) {
        // skip word in text
        while (text[i] != ' ' && text[i] != '\0' && text[i] != '\n') {
            ++i;
        }
        if (text[i] == ' ') {
            ++i;
        }
    }
    int k = i;
    while (text[i] != ' ' && text[i] != '\0' && text[i] != '\n') {
        dest[i - k] = text[i];
        ++i;
    }
    dest[i - k] = '\0';
    // return the number of characters have been copied
    return i - k;
}

bool node_exist(int value, int count) {
    for (int i = 0; i < count; ++i) {
        if (NODES[i] == value) return true;
    }
    return false;
}

void stop_all_nodes(char* addr, message token, int count) {
    void *context = zmq_ctx_new();
    void *requester = create_zmq_socket(context, ZMQ_REQ);

    for (int i = 0; i < count; ++i) {
        token.cmd = delete;
        if (ping_process(NODES[i])) {
            reconnect_zmq_socket(requester, NODES[i], addr);
            send_msg_wait(requester, &token);
            receive_msg_wait(requester, &token);
        }
    }
    close_zmq_socket(requester);
    destroy_zmq_context(context);
}

// infinite ping All nodes once every <token.timeout> microseconds
void* pinging(void* arg) {
    ping_token token = *((ping_token*) arg);
    int nodes_states[MAX_NODES];
    memset(nodes_states, 0, sizeof(nodes_states));
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < *token.count; ++i) {
            if (ping_process(NODES[i]) == false) {
                ++nodes_states[i];
                if (nodes_states[i] == 4) {
                    printf("Heartbit: node %d is unavailable\n", NODES[i] - MIN_ADDR);
                } else if (nodes_states[i] > 4) {
                    // not to overflow int type
                    nodes_states[i] = 404;
                }
            } else {
                nodes_states[i] = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(token.timeout);
    }
    return NULL;
}

void start_ping_all(ping_token* token) {
    pthread_t th;
    if (pthread_create(&th, NULL, &pinging, token) != 0) {
        printf("\t cannot create thread\n");
        return;
    }
}

void* send_receive_msg_thread(void* arg) {
    int id = ((message *)arg)->dest_id;
    void *context = zmq_ctx_new();
    void *requester = create_zmq_socket(context, ZMQ_REQ);
    char addr[MAX_LEN];

    create_addr(addr, id, tcp_serv);
    connect_zmq_socket(requester, addr);
    send_msg_wait(requester, (message*) arg);
    receive_msg_wait(requester, (message*) arg);

    if (((message *)arg)->cmd != success){
        printf("\t node cannot run your query\n");
    }

    close_zmq_socket(requester);
    destroy_zmq_context(context);
    return NULL;
}

void send_receive_msg(message * token) {
    pthread_t th;
    if (pthread_create(&th, NULL, &send_receive_msg_thread, token) != 0) {
        printf("\t cannot create thread\n");
        return;
    }
}

// tree insert
void node_append(int value) {
    int i = 1;
    while (i < MAX_TREE_SIZE) {
        if (tree[i] == -1) {
            tree[i] = value;
            return;
        } else {
            if (value < tree[i]) {
                // left child
                i = i * 2;
            } else {
                // right child
                i = i * 2 + 1;
            }
        }
    }
    printf("\t node cannot be added to the tree\n");
}
// tree find parent
int find_creator(int id) {
    int i = 1;
    while (i < MAX_TREE_SIZE) {
        if (tree[i] == -1) {
            return i/2;
        } else {
            if (id < tree[i]) {
                // left child
                i = i * 2;
            } else {
                // right child
                i = i * 2 + 1;
            }
        }
    }
    return 0;
}
