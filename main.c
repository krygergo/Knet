#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>

#include "p2p/util.h"
#include "p2p/header/node.h"

static int options( int argc, char *argv[] );

int main( int argc, char *argv[] ) {
    return argc == 1 ? node_start( 0, NULL ) : options( argc, argv );
}

static int options( int argc, char *argv[] ) {
    struct node_info_pair *nodeInfoPairVector = malloc( ((argc - 1) / 2) * sizeof( struct node_info_pair ) );
    int index = 0;
    for(int i = 1; i < argc; i+=2 ) {
        if( string_equals( argv[i], "-P" ) ) {
            int *port = malloc( sizeof( int ) );
            *port = atoi( argv[i + 1] );
            nodeInfoPairVector[index++] = create_node_info_pair( Port, port );
        } else if (string_equals( argv[i], "-TQ" ) ) {
            int *threadQuantity = malloc( sizeof( int ) );
            *threadQuantity = atoi( argv[i + 1] );
            nodeInfoPairVector[index++] = create_node_info_pair( ThreadQuantity, threadQuantity );
        } else {
            break;
        }
    }
    return node_start( index, nodeInfoPairVector );
}