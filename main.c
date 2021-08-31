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
#include "p2p/header/exitcodes.h"

static int options( int argc, char *argv[] );
static int exit_handler( int exitCode );

char exitMessage[EXIT_MESSAGE_SIZE];

int main( int argc, char *argv[] ) {
    return exit_handler( argc == 1 ? node_start( 0, NULL ) : options( argc, argv ) );
}

static int exit_handler( int exitCode ) {
    switch ( exitCode ) {
        case WRONG_ARGUMENT:
            printf("Wrong argument: %s\n", exitMessage );            
            return 1;
        case WRONG_NUMBER_OF_ARGUMENTS:
            printf("Wrong number of arguments\n");
            return 1;
        default:
            return 0;
    }
}

static int options( int argc, char *argv[] ) {
    if( ( argc % 2 ) ) {
        struct node_info_pair *nodeInfoPairVector = malloc( ( (argc - 1) / 2) * sizeof( struct node_info_pair ) );
        int index = 0;
        for( int i = 1; i < argc; i+=2 ) {
            if( string_equals( argv[i], "-P" ) ) {
                int *port = malloc( sizeof( int ) );
                *port = atoi( argv[i + 1] );
                if( *port == 0 || ( *port < MINIMUM_PORT_NUMBER || *port > MAXIMUM_PORT_NUMBER ) ) {
                    strcpy( exitMessage, argv[i + 1] );
                    return WRONG_ARGUMENT;
                }
                nodeInfoPairVector[index++] = create_node_info_pair( Port, port );
            } else if( string_equals( argv[i], "-TQ" ) ) {
                int *threadQuantity = malloc( sizeof( int ) );
                *threadQuantity = atoi( argv[i + 1] );
                if( *threadQuantity == 0 ) {
                    strcpy( exitMessage, argv[i + 1] );
                    return WRONG_ARGUMENT;
                }
                nodeInfoPairVector[index++] = create_node_info_pair( ThreadQuantity, threadQuantity );
            } else {
                strcpy( exitMessage, argv[i] );
                return WRONG_ARGUMENT;
            }
        }
        return node_start( index, nodeInfoPairVector );
    } else {
        return WRONG_NUMBER_OF_ARGUMENTS;
    }
}