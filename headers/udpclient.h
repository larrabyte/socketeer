#pragma once

#include "socketeer.h"
#include <inttypes.h>

typedef struct listnode {
    struct castinfo data;
    listnode_ts *next;
} listnode_ts;

void printlist(listnode_ts *startptr) {
    
}

void recvonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    int clilen = sizeof(clientaddr);
    struct castinfo castdata;

    printf("Awaiting broadcast...\n");

    while(1) {
        recvfrom(*socket, (char*) &castdata, sizeof(castdata), 0, (struct sockaddr*) &clientaddr, &clilen);
        printf("\nBroadcast received! Version %" PRIu64 ", hostname: %s\n", castdata.version, castdata.hostname);
    }
}