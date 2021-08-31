#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "../header/node.h"
#include "../header/server.h"
#include "../header/client.h"

static void end_inter_process_communication( int interProcessCommunication[2][2] ) {
    close( interProcessCommunication[1][1] );
    close( interProcessCommunication[1][0] );
    close( interProcessCommunication[0][1] );
    close( interProcessCommunication[0][0] );
}

static int start_server( struct node_info nodeInfo, int interProcessCommunication[2][2] ) {
    close( interProcessCommunication[0][0] );
    close( interProcessCommunication[1][1] );
    int serverStart = server_start( (struct server) {
        .nodeInfo = nodeInfo,
        .client = { interProcessCommunication[1][0], interProcessCommunication[0][1] }
    } );
    close( interProcessCommunication[1][0] );
    close( interProcessCommunication[0][1] );
    return serverStart;
}

static int start_client( struct node_info nodeInfo, int interProcessCommunication[2][2] ) {
    int clientStart = client_start( (struct client) {
        .nodeInfo = nodeInfo,
        .client = { interProcessCommunication[1][0], interProcessCommunication[0][1] },
        .server = { interProcessCommunication[0][0], interProcessCommunication[1][1] }
    } );
    end_inter_process_communication( interProcessCommunication );
    return clientStart;
}

static void destroy_node_info_pair_vector( int size, struct node_info_pair nodeInfoPairVector[] ) {
    for(int i = 0; i < size; i++ ) {
        free(nodeInfoPairVector[i].value);
    }
    free(nodeInfoPairVector);
}

int node_start( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] ) {
    int interProcessCommunication[2][2];
    for( int i = 0; i < 2; i++ ) {
        if( pipe( interProcessCommunication[i] ) < 0 ) {
            return errno;
        }
    }
    struct node_info nodeInfo = create_node_info( nodeInfoPairCounter, nodeInfoPairVector );
    destroy_node_info_pair_vector( nodeInfoPairCounter, nodeInfoPairVector );
    pid_t pid = fork();
    if ( pid == 0 ) {
        return start_server( nodeInfo, interProcessCommunication );
    } else if ( pid > 0 ) {
        return start_client( nodeInfo, interProcessCommunication );
    } else {
        return errno;
    }
}

struct node_info create_node_info( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] ) {
    struct node_info nodeInfo = {
        .port = 0,
        .threadQuantity = 0
    };
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