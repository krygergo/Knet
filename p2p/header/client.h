#ifndef KNET_CLIENT_H
#define KNET_CLIENT_H

#include "../util.h"

struct client {
    int server[2];
};

int client_start( struct client client );

#endif // KNET_CLIENT_H