#ifndef KNET_SERVER_H
#define KNET_SERVER_H

#include "../util.h"

struct server {
    int client[2];
    int threadQuantity;
    int port;
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