#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define TERMINALMAX 4096   // Maximum characters inside a terminal.

typedef struct setupdata_ts {  // Struct typedef for passing setup data.
    struct addrinfo *result;
    SOCKET socketeer;
} setupdata_ts;

typedef struct fileattr_ts {  // Struct typedef for passing a file and its attributes.
    size_t size;
    char *data;
} fileattr_ts;

// Function definitions.
void exitsock(struct addrinfo *result, SOCKET socket, int code);
setupdata_ts commoninit(char **argv, int type);
void *safealloc(void *memory, size_t size);
SOCKET serverinit(setupdata_ts sockinfo);
void clientinit(setupdata_ts sockinfo);
fileattr_ts readfile(char *abspath);
void fetchinput(char *buffer);

setupdata_ts commoninit(char **argv, int type) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int retc;

    if(type == 0) {
        hints.ai_flags = AI_PASSIVE;                                // Integer types:
        hints.ai_family = AF_INET;                                  // 0 is for server
        retc = getaddrinfo(NULL, argv[2], &hints, &result);         // 1 is for client
    } else if(type == 1) {
        hints.ai_family = AF_UNSPEC;
        retc = getaddrinfo(argv[2], argv[3], &hints, &result);
    }

    if(retc != 0) {
        fprintf(stderr, "Socketeer failed to get address info.\n");
        exitsock(NULL, INVALID_SOCKET, 1);
    }

    SOCKET commonsock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(commonsock == INVALID_SOCKET) {
        fprintf(stderr, "Socketeer failed to create socket, code %d.\n", WSAGetLastError());
        exitsock(result, commonsock, 1);
    }

    setupdata_ts retdata = {result, commonsock};
    return retdata; // Return a setupdata_ts struct.
}

SOCKET serverinit(setupdata_ts sockinfo) {
    int retc = bind(sockinfo.socketeer, sockinfo.result->ai_addr, (int) sockinfo.result->ai_addrlen);
    if(retc == SOCKET_ERROR) {
        fprintf(stderr, "Socketeer failed to create/bind socket, code %d.\n", WSAGetLastError());
        exitsock(sockinfo.result, sockinfo.socketeer, 1);
    }

    freeaddrinfo(sockinfo.result);  // Free result struct, we don't need it anymore.
    sockinfo.result = NULL;         // Just to make sure, set pointer to NULL.

    retc = listen(sockinfo.socketeer, SOMAXCONN);   // Try to listen on the socket.
    if(retc == SOCKET_ERROR) {                      // If a listening attempt on the socket fails?
        fprintf(stderr, "Socketeer failed to listen on socket, code %d.\n", WSAGetLastError());
        exitsock(sockinfo.result, sockinfo.socketeer, 1);
    }

    SOCKET clients = accept(sockinfo.socketeer, NULL, NULL);
    if(clients == INVALID_SOCKET) {
        fprintf(stderr, "Socketeer failed to accept a connection, code %d.\n", WSAGetLastError());
        exitsock(sockinfo.result, clients, 1);
    }

    return clients; // Return the connected socket.
}

fileattr_ts readfile(char *abspath) {
    FILE *fstream = fopen(abspath, "rb");
    if(fstream == NULL) {
        fprintf(stderr, "Invalid path provided.\n");
        exitsock(NULL, INVALID_SOCKET, 1);
    }

    fileattr_ts filetoret;
    fseek(fstream, 0L, SEEK_END);
    filetoret.size = ftell(fstream);
    fseek(fstream, 0L, 0);
    
    filetoret.data = (char*) safealloc(NULL, filetoret.size);
    fread(filetoret.data, 1, filetoret.size, fstream);
    fclose(fstream);
    return filetoret;
}

void clientinit(setupdata_ts sockinfo) {
    int retc = connect(sockinfo.socketeer, sockinfo.result->ai_addr, (int) sockinfo.result->ai_addrlen);
    if(retc == SOCKET_ERROR) {
        fprintf(stderr, "Socketeer failed to connect, code %d.\n", WSAGetLastError());
        exitsock(sockinfo.result, sockinfo.socketeer, 1);
    }
}

void fetchinput(char *buffer) {
    memset(buffer, 0, TERMINALMAX);  // Zero out the buffer.
    size_t buffersize = 0;           // Cache size of actual data.
    int onechar;

    while((onechar = fgetc(stdin)) != EOF && onechar != '\n') buffer[buffersize++] = (char) onechar;
}

void *safealloc(void *memory, size_t size) {
    if(memory == NULL) memory = malloc(size);
    else memory = realloc(memory, size);

    if(memory == NULL) {
        fprintf(stderr, "Socketeer ran out of memory.\n");
        exitsock(NULL, INVALID_SOCKET, 1);
    }

    return memory;
}

void exitsock(struct addrinfo *result, SOCKET socket, int code) {
    if(socket != INVALID_SOCKET) closesocket(socket);
    if(result != NULL) freeaddrinfo(result);
    WSACleanup();
    exit(code);
}