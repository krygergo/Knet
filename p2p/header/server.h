#ifndef KNET_SERVER_H
#define KNET_SERVER_H

#include "node.h"
#include "../util.h"

struct server {
    struct node_info nodeInfo;
    int client[2];
};

struct socket_communication {
    int clientWriteFileDescriptor;
    int acceptedSocketFileDescriptor;
};

struct client_communication {
    int clientReadFileDescriptor;
    int socketFileDescriptor;
};

int server_start( struct server server );

#endif // KNET_SERVER_H