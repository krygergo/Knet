#ifndef KNET_CLIENT_H
#define KNET_CLIENT_H

#include "node.h"
#include "../util.h"

struct client {
    struct node_info nodeInfo;
    int client[2];
    int server[2];
};

int client_start( struct client client );

#endif // KNET_CLIENT_H