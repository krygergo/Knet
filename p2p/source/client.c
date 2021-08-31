#include <stdio.h>
#include <unistd.h>

#include "../header/client.h"

#define MESSAGE_SIZE 1024

int client_start( struct client client ) {
    char messageBuffer[MESSAGE_SIZE];
    while( scanf( "%s", messageBuffer ) >= 0 ) {
        if( string_equals( messageBuffer, "stop") ) {
            write( client.server[WRITE], messageBuffer, MESSAGE_SIZE );
        } else {
            read( client.server[READ], messageBuffer, MESSAGE_SIZE );
            printf("%s", messageBuffer );
        }
    }
    return 0;
}