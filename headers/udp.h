#pragma once

#include "posixcompat.h"
#include "terminal.h"
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

enum retcodes { ADDTOLIST, DONTADDTOLIST };

struct listnode *sptr = NULL;  // Start pointer of the received casts list.

// Checks recvdata against the cast list.
int checkcastlist(struct castinfo recvdata) {
    if(recvdata.version != CASTVERSION) return DONTADDTOLIST;
    struct listnode *local = sptr;
    int addrsame, portsame;

    while(local->next != NULL) {
        addrsame = !strcmp(recvdata.hostname, local->remote.hostname);
        portsame = (recvdata.portno == local->remote.portno);
        if(addrsame && portsame) return DONTADDTOLIST;
        local = local->next;
    }

    return ADDTOLIST;
}

// Traverses and prints each member of the cast list.
void printcastlist() {
    struct listnode *local = sptr;
    unsigned char counter = 0;

    clearterm();
    printf("Broadcast list:\n\n");

    while(local->next != NULL) {
        printf("%.2u) %s:%hu\n", counter++, local->remote.hostname, local->remote.portno);
        local = local->next;
    }
}

// Adds recvdata to the list of received broadcasts.
// Assumes that this is a brand new member.
void addtolist(struct castinfo recvdata) {
    struct listnode *local = sptr;

    while(local->next != NULL) local = local->next;  // Traverse the linked list.
    memcpy(local, &recvdata, sizeof(recvdata));      // Copy recvdata into a new member.

    local->next = (struct listnode*) safealloc(NULL, sizeof(struct listnode));
    memset(local->next, 0, sizeof(struct listnode));
}

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
        strncpy(data.hostname, inet_ntoa(address), sizeof(data.hostname));
    }

    data.version = CASTVERSION;  // Should be 1.
    data.portno = 50647;         // some random number.

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

    // Initialising received broadcast list.
    sptr = (struct listnode*) safealloc(NULL, sizeof(struct listnode));
    memset(sptr, 0, sizeof(struct listnode));
    printf("Awaiting broadcast...\n");

    while(1) {
        recvfrom(*socket, (char*) &data, sizeof(data), 0, (struct sockaddr*) &clientaddr, &clilen);
        if(checkcastlist(data) == ADDTOLIST) addtolist(data);
        printcastlist();
    }
}