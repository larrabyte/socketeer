#pragma once

#include "sockets.h"
#include <inttypes.h>

// Performs data sending functions using the UDP protocol.
void sendonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    struct castinfo data = {"192.168.0.96", 1047, 1};
    printf("Broadcasting...\n");

    while(1) {
        sendto(*socket, (char*) &data, sizeof(data), 0, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
        socksleep(1);
    }
}

// Performs data receiving functions using the UDP protocol.
void recvonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    int clilen = sizeof(clientaddr);
    struct castinfo data;

    printf("Awaiting broadcast...\n");

    while(1) {
        recvfrom(*socket, (char*) &data, sizeof(data), 0, (struct sockaddr*) &clientaddr, &clilen);
        printf("Broadcast from %s:%" PRIu16 " (version %" PRIu64 ")\n", data.hostname, data.portno, data.version);
    }
}