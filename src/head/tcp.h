#pragma once

#include "compatlayer.h"
#include <inttypes.h>

#define TERMINALMAX 4096

enum datatype { TEXT, RAWDATA, HEADERDATA };

struct header {
    enum datatype type;
    uint64_t size;
};

int interpretcmd(SOCKET *socket, struct header *header, char *userinput);
void sendontcp(void *args);
void recvontcp(void *args);
