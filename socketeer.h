#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include <stdbool.h>
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
void *recvthread(void *args);
void *sendthread(void *args);

void *sendthread(void *args) {
    char **argv = (char**) args;
    setupdata_ts setup = commoninit(argv, 1);  // Setup a standard socket.
    SOCKET conn = setup.socketeer;             // Set conn to the setup socket.
    clientinit(setup);                         // Initialise client.

    char termbuf[TERMINALMAX];                 // Setup terminal buffer.
    int numbytes;                              // Number of bytes sent.

    printf("Connection established with server.\n");

    while(true) {
        printf("Message: ");
        fetchinput(termbuf);

        // Commands.
        if(strcmp(termbuf, ":sendfile") == 0 || strcmp(termbuf, ":sendfile\n") == 0) {
            printf("Type absolute path to file.\n");
            char abspath[TERMINALMAX];
            fetchinput(abspath);

            fileattr_ts file = readfile(abspath);
            numbytes = send(conn, (char*) &file.size, sizeof(size_t), 0);    // Send 8 bytes (size_t)
                
            if(numbytes != sizeof(size_t)) {
                fprintf(stderr, "Socketeer failed to send buffer size.\n");
                exitsock(setup.result, conn, 1);
            } else numbytes = send(conn, file.data, file.size, 0);          // Send actual data (char*)

            free(file.data);
        }
            
        else {
            size_t bufsize = strlen(termbuf) + 1;
            numbytes = send(conn, (char*) &bufsize, sizeof(size_t), 0);     // Send 8 bytes (size_t)
            if(numbytes != sizeof(size_t)) {
                fprintf(stderr, "Socketeer failed to send buffer size.\n");
                exitsock(setup.result, conn, 1);
            } else numbytes = send(conn, termbuf, bufsize, 0);              // Send actual data (char*)
        }

        if(numbytes == SOCKET_ERROR) {
            fprintf(stderr, "Something went wrong with Socketeer, code %d.\n", WSAGetLastError());
            exitsock(setup.result, conn, 1);
        }

        printf("Sent %d bytes.\n\n", numbytes);
    }
}

void *recvthread(void *args) {
    char **argv = (char**) args;
    setupdata_ts setup = commoninit(argv, 0);
    SOCKET clients = serverinit(setup);

    printf("Connection established with client.\n");
    size_t buffersize;
    int numbytes = 1;
    char *bufferptr;

    while(numbytes > 0) {
        numbytes = recv(clients, (char*) &buffersize, sizeof(size_t), 0);

        if(numbytes != sizeof(size_t)) {
            fprintf(stderr, "Socketeer failed to receive buffer size.\n");
            exitsock(setup.result, clients, 1);
        } else {
            bufferptr = (char*) safealloc(NULL, buffersize);
            numbytes = recv(clients, bufferptr, buffersize, 0);
        }

        printf("Message: %s\n", bufferptr);
        free(bufferptr);
    }

    if(numbytes == 0) {
        printf("Socketeer's connection has been closed.\n");
        exitsock(setup.result, clients, 0);
    } else {
        fprintf(stderr, "Socketeer has encountered an error, code %d.\n", WSAGetLastError());
        exitsock(setup.result, clients, 1);
    }

    return NULL;
}

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