#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "../header/node.h"
#include "../header/server.h"
#include "../header/client.h"

static int start_server( struct node_info nodeInfo, int clientProcessCommunication[] ) {
    return server_start( (struct server) {
        .client = { clientProcessCommunication[0], clientProcessCommunication[1] },
        .port = nodeInfo.port,
        .threadQuantity = nodeInfo.threadQuantity
    } );
}

static int start_client( struct node_info nodeInfo, int serverProcessCommunication[] ) {
    return client_start( (struct client) {
        .server = { serverProcessCommunication[0], serverProcessCommunication[1] }
    } );
}

static void destroy_node_info_pair_vector( int size, struct node_info_pair nodeInfoPairVector[] ) {
    for(int i = 0; i < size; i++ ) {
        free(nodeInfoPairVector[i].value);
    }
    free(nodeInfoPairVector);
}

int node_start( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] ) {
    int interProcessCommunication[2];
    if( pipe( interProcessCommunication ) < 0 )
        return errno;
    struct node_info nodeInfo = create_node_info( nodeInfoPairCounter, nodeInfoPairVector );
    destroy_node_info_pair_vector( nodeInfoPairCounter, nodeInfoPairVector );
    pid_t pid = fork();
    if ( pid == 0 ) {
        return start_server( nodeInfo, interProcessCommunication );
    } else if ( pid > 0 )
        return start_client( nodeInfo, interProcessCommunication );
    else
        return errno;
}

struct node_info create_node_info( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] ) {
    struct node_info nodeInfo;
    for( int i = 0; i < nodeInfoPairCounter; i++ ) {
        switch ( nodeInfoPairVector[i].nodeInfoType ) {
            case Port:
                nodeInfo.port = *(int *)nodeInfoPairVector[i].value;
                break;
            case ThreadQuantity:
                nodeInfo.threadQuantity = *(int *)nodeInfoPairVector[i].value;
                break;
            default:
                break;
        }
    }
    return nodeInfo;
}

struct node_info_pair create_node_info_pair( enum node_info_type nodeInfoType, void *value ) {
    return (struct node_info_pair) {
        .nodeInfoType = nodeInfoType,
        .value = value
    };
}