#pragma once

#include "posixcompat.h"
#include "sockutils.h"
#include <stdint.h>

#define TERMINALMAX 4096
#define CASTVERSION 1

enum socktype { UNINITIALISED, TCPSERVER, TCPCLIENT, UDPRECVER, UDPCASTER };
enum datatype { TEXT, RAWDATA, HEADERDATA };
struct sockaddr_in serveraddr, clientaddr;

struct castinfo {
    char hostname[16];
    uint16_t portno;
    uint64_t version;
};

struct header {
    enum datatype type;
    uint64_t size;
};

// Returns a ready-to-use UDP socket based on socktype passed in.
SOCKET udpsocketinit(int portno, enum socktype stype) {
    if(stype == TCPSERVER || stype == TCPCLIENT) return (SOCKET) ~0;
    int retcode;

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sockfd == INVALID_SOCKET) exitsock("Socketeer failed to create a socket.\n", lasterror());

    #ifdef _WIN32
        char castperms = '1';
    #else
        socklen_t castperms = '1';
    #endif

    retcode = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &castperms, sizeof(castperms));
    if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to set socket broadcast options.\n", lasterror());

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_port = htons(portno);
    serveraddr.sin_family = AF_INET;

    if(stype == UDPRECVER) {
        serveraddr.sin_addr.s_addr = INADDR_ANY;
        retcode = bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to bind a socket.\n", lasterror());
    } else {
        serveraddr.sin_addr.s_addr = INADDR_BROADCAST;
    }

    return sockfd;
}

// Returns a ready-to-use TCP socket based on socktype passed in.
SOCKET tcpsocketinit(const char *address, const char *portstr, enum socktype stype) {
    if(stype == UDPCASTER || stype == UDPRECVER) return (SOCKET) ~0;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int retcode;

    if(stype == TCPSERVER) {
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
    } else if(stype == TCPCLIENT) {
        hints.ai_family = AF_UNSPEC;
    }

    retcode = getaddrinfo(address, portstr, &hints, &result);
    if(retcode != 0) exitsock("Socketeer failed to get address info.\n", -1);
    SOCKET sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sockfd == INVALID_SOCKET) exitsock("Socketeer failed to create a socket.\n", lasterror());

    if(stype == TCPSERVER) {
        retcode = bind(sockfd, result->ai_addr, (int) result->ai_addrlen);
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to bind a socket.\n", lasterror());
        freeaddrinfo(result);

        retcode = listen(sockfd, SOMAXCONN);
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to listen on a socket.\n", lasterror());
        sockfd = accept(sockfd, NULL, NULL);
        if(sockfd == INVALID_SOCKET) exitsock("Socketeer failed to accept a connection.\n", lasterror());
    }

    else if(stype == TCPCLIENT) {
        retcode = connect(sockfd, result->ai_addr, (int) result->ai_addrlen);
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to establish a connection.\n", lasterror());
    }

    return sockfd;
}

