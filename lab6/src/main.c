#include <stdio.h>
#include "../headers/msgQ.h"

int* NODES;

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
    return i - k;
}

int __str_to_int(const char* str) {
    int res = 0;
    int protect = 0;
    for (int i = 0; str[i] > 0; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            if (protect++ > 4) return -100000;
            res = res * 10 + str[i] - '0';
        }
    }
    return res;
}

int node_exist(int value) {
    if (NODES == NULL) return 0;
    int i = 0;
    while (NODES[i] != TERMINATOR) {
        if (NODES[i] == value) return 1;
        ++i;
    }
    return 0;
}

void stop_all_children(void* requester, char* addr, message token) {
    for (int i = 0; NODES[i] != TERMINATOR; ++i) {
        token.cmd = delete;
        if (ping_process(NODES[i])) {
            reconnect_zmq_socket(requester, NODES[i], addr);
            send_msg_wait(requester, &token);
            receive_msg_wait(requester, &token);
        }
    }
}

int main (int argc, char const *argv[])  {
    void *context = zmq_ctx_new();
    void *requester = create_zmq_socket(context, ZMQ_REQ);
    NODES = create_vector();
    if (NODES == NULL) return 1;
    char query_line [MN];
    char query_word[MN];
    char query_int[MN];
    char addr[MN] = SERVER_SOCKET_PATTERN;
    int last_created = -1;
    printf("ME: %d\n", getpid());
    message token = {create, 0, "", ""};

    while (fgets(query_line, MN, stdin) != NULL) {
        split_copy(query_line, query_word, 0);

        if (strcmp(query_word, "create") == 0) {
            if (split_copy(query_line, query_int, 1) == 0) {
                printf("\tbad id, try another one\n");
                continue;
            }
            int id_process = __str_to_int(query_int) + MIN_ADDR;
            if (node_exist(id_process)) {
                printf("\tthis node was created earlier\n");
                continue;
            }
            char flag_creation_str[MN];
            int flag_creation = 0;
            int l = split_copy(query_line, flag_creation_str, 2);
            if (l == 2) {
                if (flag_creation_str[0] == '-' && flag_creation_str[1] == '1') {
                    flag_creation = 1;
                }
            }
            if (length(NODES) == 0 || flag_creation) {
                if (id_process > MIN_ADDR) {
                    printf("\tI'm creating %d\n", id_process);
                    NODES = push_back(NODES, id_process);
                    if (NODES == NULL) {
                        printf("\tSm-th wrong with dynamic array\n");
                        return 1;
                    }
                    reconnect_zmq_socket(requester, id_process, addr);
                } else {
                    printf("\tSm-th wrong with number\n");
                    continue;
                }
                memset(query_int, 0, MN);
                sprintf(query_int, "%d", id_process);
                char *Child_argv[] = {"node", query_int, NULL};
                int pid = fork();
                if (pid == -1) {
                    return 1;
                }
                if (pid == 0) {
                    execv("node", Child_argv);
                    return 0;
                }
                last_created = id_process;
            } else {
                if (id_process < MIN_ADDR) {
                    printf("\tbad id\n");
                    continue;
                }
                if (ping_process(last_created) == false) {
                    printf("\tCannot create Node from [%d]\n", last_created - MIN_ADDR);
                    continue;
                }
                token.cmd = create;
                token.value = id_process;
                create_addr(addr, last_created);
                printf("\tsend to %s\n", addr);
                send_msg_wait(requester, &token);
                receive_msg_wait(requester, &token);
                if (token.cmd == success) {
                    NODES = push_back(NODES, id_process);
                    last_created = id_process;
                    if (NODES == NULL) {
                        printf("\tSm-th wrong with dynamic array\n");
                        return 1;
                    }
                } else {
                    printf("\t%d was successfully created\n", id_process - MIN_ADDR);
                }
            }

        } else if (strcmp(query_word, "exec") == 0) {
            split_copy(query_line, query_int, 1);
            split_copy(query_line, query_int, 1);
            int id_process = __str_to_int(query_int) + MIN_ADDR;
            if (node_exist(id_process) && ping_process(id_process)) {
                fgets(token.str, MAX_LEN, stdin);
                fgets(token.sub, MAX_LEN, stdin);
                for (int i = MAX_LEN - 1; i >= 0; --i) {
                    if (token.str[i] == '\n') {
                        token.str[i] = '\0';
                        break;
                    }
                }
                token.cmd = exec;
                reconnect_zmq_socket(requester, id_process, addr);
                send_msg_wait(requester, &token);
                receive_msg_wait(requester, &token);
                if (token.cmd == success) {
                    printf("\tall matches: ");
                    for (int i = 0; i < MAX_LEN; ++i) {
                        if (token.res[i] == -1) {
                            if (i == 0) printf("NO");
                            break;
                        }
                        printf("%d ", token.res[i]);
                    }
                    printf("\n");
                } else {
                    printf("\tcannot exec %d\n", id_process - MIN_ADDR);
                }
            } else {
                printf("\t[%d] Node hasn't connection\n", id_process - MIN_ADDR);
            }

        } else if (strcmp(query_word, "remove") == 0) {
            split_copy(query_line, query_int, 1);
            int id_process = __str_to_int(query_int) + MIN_ADDR;
            if (node_exist(id_process) && ping_process(id_process)) {
                reconnect_zmq_socket(requester, id_process, addr);
                token.cmd = delete;
                send_msg_wait(requester, &token);
                receive_msg_wait(requester, &token);
                if (token.cmd != success) {
                    printf("\tcannot remove %d\n", id_process - MIN_ADDR);
                }
            } else {
                printf("\t[%d] Node hasn't connection\n", id_process - MIN_ADDR);
            }

        } else if (strcmp(query_word, "pingall") == 0) {
            printf("\t");
            for (int i = 0; NODES[i] != TERMINATOR; ++i) {
                if (ping_process(NODES[i]) == true) {
                    printf("[%d is OKE] ", NODES[i] - MIN_ADDR);
                } else {
                    printf("[%d is BAD] ", NODES[i] - MIN_ADDR);
                }
            }
            printf("\n");
        } else {
            printf("\tBad command. Please, try again\n");
        }
        memset(query_line, 0, MN);
        memset(query_word, 0, MN);
        memset(query_int, 0, MN);
    }
    stop_all_children(requester, addr, token);
    close_zmq_socket(requester);
    destroy_zmq_context(context);
    destroy(NODES);
    return 0;
}
