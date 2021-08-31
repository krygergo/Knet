#ifndef KNET_UTIL_H
#define KNET_UTIL_H

#include <string.h>

#define READ 0
#define WRITE 1

#define MINIMUM_PORT_NUMBER 1024
#define MAXIMUM_PORT_NUMBER 49151

#define string_equals(s1, s2) (strcmp(s1, s2) == 0) ? 1 : 0

#define EXIT_MESSAGE_SIZE 64

extern char exitMessage[EXIT_MESSAGE_SIZE];

#endif // KNET_UTIL_H