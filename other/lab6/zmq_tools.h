#ifndef OS_LAB6_ZMQ_TOOLS_H
#define OS_LAB6_ZMQ_TOOLS_H

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <zmq.h>
#include <limits.h>

#define MAX_LEN                128
#define MAX_NODES               50
#define MAX_TREE_SIZE          512

#define ERR_ZMQ_CTX            100
#define ERR_ZMQ_SOCKET         101
#define ERR_ZMQ_BIND           102
#define ERR_ZMQ_CLOSE          103
#define ERR_ZMQ_CONNECT        104
#define ERR_ZMQ_DISCONNECT     105
#define ERR_ZMQ_MSG            106
#define ERR_ZMQ_ADDR           107

#define SERVER_SOCKET_PATTERN        "tcp://localhost:"
#define PING_SOCKET_PATTERN          "inproc://ping"
#define TCP_SOCKET_PATTERN           "tcp://*:"
#define MIN_ADDR 5555

typedef enum {
    create,
    delete,
    exec,
    success
} cmd_type;

typedef enum {
    tcp_serv,
    tcp_node,
    inproc
} addr_pattern;

typedef struct {
    cmd_type  cmd;
    int       dest_id;
    int       value;
    char      str[MAX_LEN];
} message;

void  clear_token(message* msg);
void  create_addr(char* addr, int id, addr_pattern);
void  bind_zmq_socket(void* socket, char* endpoint);

void* create_zmq_context();
void* create_zmq_socket(void* context, int type);

void  connect_zmq_socket(void* socket, char* endpoint);
void  disconnect_zmq_socket(void* socket, char* endpoint);
void  reconnect_zmq_socket(void* socket, int to, char* addr);
void  close_zmq_socket(void* socket);
void  destroy_zmq_context(void* context);

void receive_msg_wait(void* socket, message* token);
void send_msg_wait(void* socket, message* token);
bool ping_process(int id);

#endif //OS_LAB6_ZMQ_TOOLS_H
