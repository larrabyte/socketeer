#pragma once

#include <inttypes.h>

#define CASTVERSION 1

struct castinfo {
    char hostname[16];
    uint16_t portno;
    uint64_t version;
};

struct listnode {
    struct castinfo remote;
    struct listnode *next;
};

enum udpretcodes { ADDTOLIST, DONTADDTOLIST };

int checkcastlist(struct castinfo recvdata);
void addtolist(struct castinfo recvdata);
void sendonudp(void *args);
void recvonudp(void *args);
void printcastlist();