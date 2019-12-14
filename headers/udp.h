#pragma once

#include "posixcompat.h"
#include "sockets.h"
#include <inttypes.h>

// Performs data sending functions using the UDP protocol.
void sendonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    printf("Broadcasting...\n");
    struct castinfo data;
    char hostname[128];

    int retcode = gethostname(hostname, 128);
    if(retcode == SOCKET_ERROR) strcpy(data.hostname, "0.0.0.0");
    else {
        struct hostent *host = gethostbyname(hostname);
        struct in_addr address = {0};

        memcpy(&address, host->h_addr_list[0], sizeof(struct in_addr));
        printf("Address: %s\n", inet_ntoa(address));
    }

    data.version = CASTVERSION;
    data.portno = 1047;

    while(1) {
        sendto(*socket, (char*) &data, sizeof(data), 0, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
        socksleep(1);
    }
}

// Performs data receiving functions using the UDP protocol.
void recvonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    socklen_t clilen = sizeof(clientaddr);
    struct castinfo data;

    printf("Awaiting broadcast...\n");

    while(1) {
        recvfrom(*socket, (char*) &data, sizeof(data), 0, (struct sockaddr*) &clientaddr, &clilen);
        printf("Broadcast from %s:%" PRIu16 " (version %" PRIu64 ")\n", data.hostname, data.portno, data.version);
    }
}