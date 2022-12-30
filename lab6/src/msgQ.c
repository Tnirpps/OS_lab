#include "../headers/msgQ.h"

void create_addr(char* addr, int id) {
    char str[MN];
    memset(str, 0, MN);
    sprintf(str, "%d", id);
    memset(addr, 0, MN);
    memcpy(addr, SERVER_SOCKET_PATTERN, sizeof(SERVER_SOCKET_PATTERN));
    strcat(addr, str);
}

void clear_token(message* msg) {
    msg->cmd = delete;
    msg->value = 0;
    memset(msg->str, 0, MAX_LEN);
    memset(msg->sub, 0, MAX_LEN);
    for (int i = 0; i < MAX_LEN; ++i) {
        msg->res[i] = -1;
    }
}

void* create_zmq_context() {
    void* context = zmq_ctx_new();
    if (context == NULL) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_ctx_new ");
        exit(ERR_ZMQ_CTX);
    }
    return context;
}

void bind_zmq_socket(void* socket, char* endpoint) {
    if (zmq_bind(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_bind ");
        exit(ERR_ZMQ_BIND);
    }
}

void disconnect_zmq_socket(void* socket, char* endpoint) {
    if (zmq_disconnect(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_disconnect ");
        exit(ERR_ZMQ_DISCONNECT);
    }
}

void connect_zmq_socket(void* socket, char* endpoint) {
    if (zmq_connect(socket, endpoint) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_connect ");
        exit(ERR_ZMQ_CONNECT);
    }
}


void* create_zmq_socket(void* context, const int type) {
    void* socket = zmq_socket(context, type);
    if (socket == NULL) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_socket ");
        exit(ERR_ZMQ_SOCKET);
    }
    return socket;
}

void reconnect_zmq_socket(void* socket, int to, char* addr) {
    if (addr[16] != '\0') {
        disconnect_zmq_socket(socket, addr);
    }
    create_addr(addr, to);
    connect_zmq_socket(socket, addr);
}

void close_zmq_socket(void* socket) {
    if (zmq_close(socket) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_close ");
        exit(ERR_ZMQ_CLOSE);
    }
}

void destroy_zmq_context(void* context) {
    if (zmq_ctx_destroy(context) != 0) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_ctx_destroy ");
        exit(ERR_ZMQ_CLOSE);
    }
}


int* create_vector() {
    int* ptr = malloc(sizeof(int));
    if (ptr == NULL) return NULL;
    ptr[0] = TERMINATOR;
    return ptr;
}

int length(const int* ptr) {
    if (ptr == NULL) -1;
    int i = 0;
    while (ptr[i] != TERMINATOR) {
        ++i;
    }
    return i;
}


// very bad, but essy solution: not to use capacity
int* push_back(int* ptr, int value) {
    if (ptr == NULL) return NULL;
    int size = 0;
    while (ptr[size] != TERMINATOR) {
        ++size;
    }
    int* tmp = realloc(ptr, (size + 2) * sizeof(int));
    if (tmp == NULL) {
        free(ptr);
        return NULL;
    }
    ptr = tmp;
    ptr[size] = value;
    ptr[size + 1] = TERMINATOR;
    return ptr;
}

void erase(int* ptr, int value) {
    if (ptr == NULL) return;
    int i = 0;
    while (ptr[i] != TERMINATOR) {
        if (ptr[i] == value) break;
        ++i;
    }
    // shift array after deleting the element
    while (ptr[i] != TERMINATOR) {
        ptr[i] = ptr[i + 1];
        i++;
    }
}

void destroy(int* ptr) {
    if (ptr == NULL) return;
    free(ptr);
}


bool receive_msg_wait(void* socket, message* token) {
    zmq_msg_t reply;
    zmq_msg_init(&reply);
    if (zmq_msg_recv(&reply, socket, 0) == -1) {
        zmq_msg_close(&reply);
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_recv ");
        exit(ERR_ZMQ_MSG);
    }
    (*token) = * ((message*) zmq_msg_data(&reply));
    if (zmq_msg_close(&reply) == -1) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_close ");
        exit(ERR_ZMQ_MSG);
    }
    return true;
}

bool send_msg_wait(void* socket, message* token) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_msg_init_size(&msg, sizeof(message)) == -1) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_init ");
        exit(ERR_ZMQ_MSG);
    }
    if (zmq_msg_init_data(&msg, token, sizeof(message), NULL, NULL) == -1) {
        fprintf(stderr, "[%d] ", getpid());
        perror("ERROR zmq_msg_init ");
        exit(ERR_ZMQ_MSG);
    }
    if (zmq_msg_send(&msg, socket, 0) == -1) {
        zmq_msg_close(&msg);
        return false;
    }
    return true;
}

bool ping_process(int id) {
    char addr_monitor[30];
    char addr_connection[30];
    char str[30];
    memset(str, 0, 30);
    sprintf(str, "%d", id);
    memset(addr_monitor, 0, 30);
    memcpy(addr_monitor, PING_SOCKET_PATTERN, sizeof(PING_SOCKET_PATTERN));
    strcat(addr_monitor, str);
    memset(addr_connection, 0, 30);
    memcpy(addr_connection, SERVER_SOCKET_PATTERN, sizeof(SERVER_SOCKET_PATTERN));
    strcat(addr_connection, str);

    void* context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    zmq_socket_monitor(requester, addr_monitor, ZMQ_EVENT_CONNECTED | ZMQ_EVENT_CONNECT_RETRIED);
    void *socet = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socet, addr_monitor);
    zmq_connect(requester, addr_connection);

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_msg_recv(&msg, socet, 0);
    uint8_t* data = (uint8_t*)zmq_msg_data(&msg);
    uint16_t event = *(uint16_t*)(data);

    zmq_close(requester);
    zmq_close(socet);
    zmq_msg_close(&msg);
    zmq_ctx_destroy(context);
    if (event == ZMQ_EVENT_CONNECT_RETRIED) {
        return false;
    } else {
        return true;
    }
}


