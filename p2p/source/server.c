#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>

#include "../header/server.h"
#include "../header/threadpool.h"

#define DEFAULT_PORT 8000
#define CONNECTION_QUEUE_SIZE 64
#define MESSAGE_SIZE 1024
#define INFINTE_TIMEOUT -1

static struct socket_communication socket_communication_create( int clientWriteFileDescriptor, int acceptedSocketFileDescriptor ) {
    return (struct socket_communication) {
        .clientWriteFileDescriptor = clientWriteFileDescriptor,
        .acceptedSocketFileDescriptor = acceptedSocketFileDescriptor
    };
}

static void socket_handler_add( int socket );
static void socket_handler_remove( int socket );

static void *server_job_socket( void *arg ) {
    struct socket_communication socketCommunication = *(struct socket_communication*) arg;
    socket_handler_add( socketCommunication.acceptedSocketFileDescriptor );
    char messageBuffer[MESSAGE_SIZE];
    struct pollfd pollFileDescriptor[1];
    pollFileDescriptor[0].fd = socketCommunication.acceptedSocketFileDescriptor;
    pollFileDescriptor[0].events = POLLIN;
    int loop = 1;
    while( loop ) {
        poll( pollFileDescriptor, 1, INFINTE_TIMEOUT );
        if( pollFileDescriptor[0].revents & POLLIN ) {
            read( socketCommunication.acceptedSocketFileDescriptor, messageBuffer, MESSAGE_SIZE );
            write( socketCommunication.clientWriteFileDescriptor, messageBuffer, MESSAGE_SIZE );
            memset( messageBuffer, 0, MESSAGE_SIZE );
        } else {
            loop = 0;
        }
    }
    socket_handler_remove( socketCommunication.acceptedSocketFileDescriptor );
}

static struct client_communication client_communication_create( int clientReadFileDescriptor, int socketFileDescriptor ) {
    return (struct client_communication) {
        .clientReadFileDescriptor = clientReadFileDescriptor,
        .socketFileDescriptor = socketFileDescriptor
    };
}

static void shutdown_and_close( int socketFiledDescriptor ) {
    shutdown( socketFiledDescriptor, SHUT_RDWR );
    close( socketFiledDescriptor );
}

static void *server_job_client( void *arg ) {
    struct client_communication clientCommunication = *(struct client_communication*) arg;
    char messageBuffer[MESSAGE_SIZE];
    int loop = 1;
    while( loop ) {
        read( clientCommunication.clientReadFileDescriptor, messageBuffer, MESSAGE_SIZE );
        if( string_equals( messageBuffer, "stop" ) ) {
            shutdown_and_close( clientCommunication.socketFileDescriptor );
            loop = 0;
        }
    }
}

struct socket_handler {
    int **sockets;
    int size;
    pthread_mutex_t mutex;
};

static struct socket_handler socketHandler;

static struct socket_handler socket_handler_initialize( int size ) {
    socketHandler = (struct socket_handler) {
        .sockets = malloc( size * sizeof( int *) ),
        .size = size,
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };
}

static void socket_handler_add( int socket ) {
    pthread_mutex_lock( &socketHandler.mutex );
    int i;
    for( i = 0; i < socketHandler.size && socketHandler.sockets[i] != NULL; i++ );
    socketHandler.sockets[i] = malloc( sizeof( int ) );
    *socketHandler.sockets[i] = socket;
    pthread_mutex_unlock( &socketHandler.mutex );
}

static void socket_handler_remove( int socket ) {
    pthread_mutex_lock( &socketHandler.mutex );
    int i;
    for( i = 0; i < socketHandler.size && (socketHandler.sockets[i] == NULL || *socketHandler.sockets[i] != socket ); i++ );
    free( socketHandler.sockets[i] );
    socketHandler.sockets[i] = NULL;
    pthread_mutex_unlock( &socketHandler.mutex );
}

static void socket_handler_stop_and_destroy( void ) {
    for( int i = 0; i < socketHandler.size; i++ ) {
        if( socketHandler.sockets[i] != NULL ) {
            shutdown_and_close( *socketHandler.sockets[i] );
        }
    }
    for( int i = 0; i < socketHandler.size; i++ ) {
        free( socketHandler.sockets[i] );
    }
    free( socketHandler.sockets );
    pthread_mutex_destroy( &socketHandler.mutex );
}

static int server_controller( struct server server, int socketFileDescriptor ) {
    struct thread_pool threadPool = thread_pool_create( server.nodeInfo.threadQuantity == 0 ? DEFAULT_THREAD_QUANTITY : server.nodeInfo.threadQuantity );
    thread_pool_start( &threadPool );
    struct client_communication clientCommunication = client_communication_create( server.client[READ], socketFileDescriptor );
    thread_pool_add_job( &threadPool, server_job_client, &clientCommunication );
    struct pollfd pollFileDescriptor[1];
    pollFileDescriptor[0].fd = socketFileDescriptor;
    pollFileDescriptor[0].events = POLLIN;
    socket_handler_initialize( threadPool.size );
    while( !threadPool.stop ) {
        poll( pollFileDescriptor, 1, INFINTE_TIMEOUT );
        if( pollFileDescriptor[0].revents & POLLIN ) {
            struct sockaddr_in client;
            socklen_t size = sizeof( client );
            int acceptedSocketFileDescriptor = accept( socketFileDescriptor, (struct sockaddr*)&client, &size );
            struct socket_communication socketCommunication = socket_communication_create( server.client[WRITE], acceptedSocketFileDescriptor );
            thread_pool_add_job( &threadPool, server_job_socket, &socketCommunication );
        } else {
            threadPool.stop = 1;
        }
    }
    socket_handler_stop_and_destroy();
    thread_pool_destroy( threadPool );
    return errno;
}

static int start_listen( struct server server, int socketFileDescriptor ) {
    if( listen( socketFileDescriptor, CONNECTION_QUEUE_SIZE ) == 0 ) {
        return server_controller( server, socketFileDescriptor );
    } else {
        return errno;
    }
}

static int start_socket( struct server server, int socketFileDescriptor ) {
    struct sockaddr_in sockaddrIn;
    memset( &sockaddrIn, 0, sizeof( sockaddrIn ) );
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = htonl( INADDR_ANY );
    sockaddrIn.sin_port = htons( server.nodeInfo.port == 0 ? DEFAULT_PORT : server.nodeInfo.port );
    if( bind( socketFileDescriptor, (struct sockaddr*)&sockaddrIn, sizeof( sockaddrIn ) ) == 0 ) {
        return start_listen( server, socketFileDescriptor );
    } else {
        return errno;
    }
}

int server_start( struct server server ) {
    int socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if( socketFileDescriptor >= 0 ) {
        return start_socket( server, socketFileDescriptor );
    } else {
        return errno;
    }
}
