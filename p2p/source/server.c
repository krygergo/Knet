#include <stdio.h>
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

static void *server_job_socket( void *arg ) {
    struct socket_communication socketCommunication = *(struct socket_communication*) arg;
    char messageBuffer[MESSAGE_SIZE];
    while( read( socketCommunication.acceptedSocketFileDescriptor, messageBuffer, MESSAGE_SIZE ) != 0 ) {
        write( socketCommunication.clientWriteFileDescriptor, messageBuffer, MESSAGE_SIZE );
        memset( messageBuffer, 0, MESSAGE_SIZE );
    }
}

static struct client_communication client_communication_create( int clientReadFileDescriptor, int socketFileDescriptor ) {
    return (struct client_communication) {
        .clientReadFileDescriptor = clientReadFileDescriptor,
        .socketFileDescriptor = socketFileDescriptor
    };
}

static void *server_job_client( void *arg ) {
    struct client_communication clientCommunication = *(struct client_communication*) arg;
    char messageBuffer[MESSAGE_SIZE];

    sleep(10);

    shutdown( clientCommunication.socketFileDescriptor, SHUT_RDWR );
    /*
    while( read( clientCommunication.clientReadFileDescriptor, messageBuffer, MESSAGE_SIZE ) != 0 ) {
        if( string_equals( messageBuffer, "stop" ) ) {
            *clientCommunication.serverTerminator.serverStopPointer = 1;
            close( clientCommunication.serverTerminator.socketFileDescriptor );
        }
    }
    */
}

static int server_controller( struct server server, int socketFileDescriptor ) {
    struct thread_pool threadPool = thread_pool_create( server.threadQuantity == 0 ? DEFAULT_THREAD_QUANTITY : server.threadQuantity );
    thread_pool_start( &threadPool );
    struct client_communication clientCommunication = client_communication_create( server.client[READ], socketFileDescriptor );
    thread_pool_add_job( &threadPool, server_job_client, &clientCommunication );
    struct pollfd pollFileDescriptor[1];
    pollFileDescriptor[0].fd = socketFileDescriptor;
    pollFileDescriptor[0].events = POLLIN;
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
    return thread_pool_destroy( threadPool );
}

static int start_listen( struct server server, int socketFileDescriptor ) {
    if( listen( socketFileDescriptor, CONNECTION_QUEUE_SIZE ) == 0 )
        return server_controller( server, socketFileDescriptor );
    else
        return errno;
}

static int start_socket( struct server server, int socketFileDescriptor ) {
    struct sockaddr_in sockaddrIn;
    memset( &sockaddrIn, 0, sizeof( sockaddrIn ) );
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = htonl( INADDR_ANY );
    sockaddrIn.sin_port = htons( server.port == 0 ? DEFAULT_PORT : server.port );
    if( bind( socketFileDescriptor, (struct sockaddr*)&sockaddrIn, sizeof( sockaddrIn ) ) == 0 )
        return start_listen( server, socketFileDescriptor );
    else
        return errno;
}

int server_start( struct server server ) {
    int socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if( socketFileDescriptor >= 0 )
        return start_socket( server, socketFileDescriptor );
    else
        return errno;
}
