#pragma once

#include <winsock2.h>

typedef struct packet_ts {  // Struct typedef for a Socketeer packet.
    unsigned char *data;
    size_t length;
} packet_ts;

typedef struct setupdata_ts {  // Struct typedef for passing setup data.
    struct addrinfo *result;
    SOCKET socketeer;
} setupdata_ts;

// Function definitions.
void exitsock(struct addrinfo *result, SOCKET socket, int code);
setupdata_ts commoninit(char **argv, int type);
void *safealloc(void *memory, size_t size);
SOCKET serverinit(setupdata_ts sockinfo);
void clientinit(setupdata_ts sockinfo);
void fetchinput(char *buffer);