#include "zmq_tools.h"

void execution(message* token) {
    char* str = token->str;
    char* cur = str;
    long long int sum = 0;
    while (cur != str + strlen(str)) {
        long int x = strtol(cur, &cur, 10);
        if (x == LONG_MAX || x == LONG_MIN) {
            // here token->cmd still equals "exec"
            // it will mean an error
            return;
        }
        while (*cur ==' ' || *cur == '\n'){
            cur++;
        }
        sum += x;
    }
    printf("\t[%d] sum = %lld\n", getpid(), sum);
    token->cmd = success;
}

int main(int argc, char const *argv[])  {
    if (argc < 2) {
        printf("\t[%d] argv error\n", getpid());
        return 1;
    }
    void *context = create_zmq_context();
    void *responder = create_zmq_socket(context, ZMQ_REP);
    char adr[MAX_LEN] = TCP_SOCKET_PATTERN;
    strcat(adr, argv[1]);

    printf("\t[%d] has been created\n", getpid());
    bind_zmq_socket(responder, adr);

    while (1) {
        message token;
        receive_msg_wait(responder, &token);
        /* to demonstrate async work: */
        // printf("\t[%d] get command\n", getpid());
        // sleep(5);
        int id_process;
        char query_str_int[MAX_LEN];
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
                memset(query_str_int, 0, MAX_LEN);
                sprintf(query_str_int, "%d", id_process);
                char *Child_argv[] = {"node", query_str_int, NULL};
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
                execution(&token);
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
