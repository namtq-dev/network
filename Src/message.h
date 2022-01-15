#include <stdio.h>

typedef struct Message {
    char msg_type[5];
    char msg_payload[1000];
} message;