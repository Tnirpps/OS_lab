#include <stdio.h>
#include <string.h>

#include "../headers/msgQ.h"

int* z_function(const char* s) {
    int n = (int) strlen(s);
    int* z = malloc(n * sizeof(int));
    if (z == NULL) {
        return NULL;
    }
    int l = 0, r = 0;
    z[0] = 0;
    for (int i = 1; i < n; ++i) {
        if (i <= r) {
//          here is z[i] = min(z[i - l], r - i) :
            z[i] = (z[i - l] < r - i) ? z[i - l] : r - i;
        }
        while (i + z[i] < n && s[i + z[i]] == s[z[i]]) {
            ++z[i];
        }
        if (i + z[i] > r) {
            l = i;
            r = i + z[i];
        }
    }
    return z;
}


void find_substr(message* token) {
    /*
        z_function(<pattern> + '#' + <text>)
        here we use '\n' as '#'
     */
    int n = (int) strlen(token->str);
    int m = (int) strlen(token->sub);
    char string [n + m + 1];
    memcpy(string, token->sub, m);
    memcpy(string + m, token->str, n);
    string[n + m] = '\0';
    int* z = z_function(string);
    clear_token(token);
    if (z == NULL) {
        return;
    }
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (z[m + i] == m - 1) {
            // one indexing
            token->res[k++] = i + 1;
        }
    }
    token->cmd = success;
    free(z);
}

int main(int argc, char const *argv[])  {
    if (argc < 2) {
        printf("argv error\n");
        return 1;
    }
    void *context = create_zmq_context();
    void *responder = create_zmq_socket(context, ZMQ_REP);
    char adr[30] = TCP_SOCKET_PATTERN;
    strcat(adr, argv[1]);
    printf("\t[%d] has been created\n", getpid());
    bind_zmq_socket(responder, adr);

    while (1) {
        message token;
        receive_msg_wait(responder, &token);
        int id_process;
        char query_int[30];
        switch (token.cmd) {
            case delete:
                token.cmd = success;
                printf("\t[%d] has been destroyed\n", getpid());
                send_msg_wait(responder, &token);
                close_zmq_socket(responder);
                destroy_zmq_context(context);
                return 0;
            case create:
                id_process = token.value;
                memset(query_int, 0, 30);
                sprintf(query_int, "%d", id_process);
                char *Child_argv[] = {"node", query_int, NULL};
                int pid = fork();
                if (pid == -1) {
                    return 1;
                }
                if (pid == 0) {
                    execv("node", Child_argv);
                    return 0;
                } else {
                    token.cmd = success;
                    send_msg_wait(responder, &token);
                }
                break;
            case exec:
                find_substr(&token);
                send_msg_wait(responder, &token);
                break;
            default:
                // equals "delete brunch"
                token.cmd = success;
                send_msg_wait(responder, &token);
                close_zmq_socket(responder);
                destroy_zmq_context(context);
                return 0;
        }
    }
}
