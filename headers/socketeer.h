#pragma once

#define TERMINALMAX 4096
#define CASTVERSION 1

#include "posixcompat.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

enum socktype { UNINITIALISED, TCPSERVER, TCPCLIENT, UDPRECVER, UDPCASTER };
enum datatype { TEXT, RAWDATA, HEADERDATA };
struct sockaddr_in serveraddr, clientaddr;

struct castinfo {
    uint64_t version;
    char hostname[16];
};

struct header {
    enum datatype type;
    uint64_t size;
};

struct fileattr {
    size_t size;
    char *data;
};

// Prints message to standard error and exits with error.
void exitsock(const char *message, int error) {
    fprintf(stderr, message);
    if(error != 0) fprintf(stderr, "Error code: %d\n", error);
    exit(-1);
}

// Performs malloc/realloc while also checking the resulting pointers.
void *safealloc(void *memory, size_t size) {
    if(memory == NULL) memory = malloc(size);
    else memory = realloc(memory, size);
    if(memory == NULL) exitsock("Socketeer encountered a memory issue.\n", 0);

    return memory;
}

// Returns a ready-to-use UDP socket based on socktype passed in.
SOCKET udpsocketinit(int portno, enum socktype stype) {
    if(stype == TCPSERVER || stype == TCPCLIENT) return (SOCKET) ~0;
    int retcode;

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sockfd == INVALID_SOCKET) exitsock("Socketeer failed to create a socket.\n", lasterror());

    char castperms = '1';
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

// Runs send() with appropriate arguments, depending on OS.
ssize_t socksend(SOCKET socket, void *buffer, size_t length, int flags) {
    #ifdef _WIN32
        return send(socket, (char*) buffer, (int) length, flags);
    #else
        return send(socket, buffer, length, flags);
    #endif
}

// Runs recv() with appropriate arguments, depending on OS.
ssize_t sockrecv(SOCKET socket, void *buffer, size_t length, int flags) {
    #ifdef _WIN32
        return recv(socket, (char*) buffer, (int) length, flags);
    #else
        return recv(socket, buffer, length, flags);
    #endif
}

// Returns a struct with file size and pointer to data.
struct fileattr readfile(char *filepath) {
    FILE *fstream = fopen(filepath, "rb");
    if(fstream == NULL) return (struct fileattr) {0, NULL};

    struct fileattr file;
    fseek(fstream, 0L, SEEK_END);
    file.size = ftell(fstream);
    fseek(fstream, 0L, 0);

    file.data = (char*) safealloc(NULL, file.size);
    fread(file.data, 1, file.size, fstream);
    fclose(fstream);
    return file;
}
