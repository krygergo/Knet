#include <stdio.h>
#include <unistd.h>

#include "../header/client.h"

#define MESSAGE_SIZE 1024

int client_start( struct client client ) {
    char messageBuffer[MESSAGE_SIZE];
    while( read( client.server[READ], messageBuffer, MESSAGE_SIZE ) >= 0 ) {
        printf("%s", messageBuffer );
    }
    return 0;
}