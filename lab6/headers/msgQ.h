#ifndef LAB_6_MSGQ_H
#define LAB_6_MSGQ_H

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <zmq.h>
#include <assert.h>

#define MAX_LEN 64
#define MN 30

#define ERR_ZMQ_CTX            100
#define ERR_ZMQ_SOCKET         101
#define ERR_ZMQ_BIND           102
#define ERR_ZMQ_CLOSE          102
#define ERR_ZMQ_CONNECT        104
#define ERR_ZMQ_DISCONNECT     105
#define ERR_ZMQ_MSG            106

#define SERVER_SOCKET_PATTERN        "tcp://localhost:"
#define PING_SOCKET_PATTERN          "inproc://ping"
#define TCP_SOCKET_PATTERN           "tcp://*:"
#define MIN_ADDR 5555

#define TERMINATOR (-3000)





typedef enum {
    create,
    delete,
    exec,
    success
} cmd_type;

typedef struct {
    cmd_type  cmd;
    int       value;
    char      str[MAX_LEN];
    char      sub[MAX_LEN];
    int       res[MAX_LEN];
} message;

void  clear_token(message* msg);
void  create_addr(char* addr, int id);
void  bind_zmq_socket(void* socket, char* endpoint);

void* create_zmq_context();
void* create_zmq_socket(void* context, const int type);

void  connect_zmq_socket(void* socket, char* endpoint);
void  disconnect_zmq_socket(void* socket, char* endpoint);
void  reconnect_zmq_socket(void* socket, int to, char* addr);
void  close_zmq_socket(void* socket);
void  destroy_zmq_context(void* context);

bool receive_msg_wait(void* socket, message* token);
bool send_msg_wait(void* socket, message* token);
bool ping_process(int id);

// TODO move implementation "std:vector<int>" to a special .h and .c files
int* create_vector();
int length(const int* ptr);
int* push_back(int* ptr, int value);
void erase(int* ptr, int value);
void destroy(int* ptr);

#endif //LAB_6_MSGQ_H