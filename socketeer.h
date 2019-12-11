#pragma once

#define TERMINALMAX 4096
#define SERVERMODE 0
#define CLIENTMODE 1
#define MESSAGE 0
#define RAWDATA 1
#define HEADERS 2

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
    #undef __USE_MINGW_ANSI_STDIO
    #define __USE_MINGW_ANSI_STDIO 1
    #include <winsock2.h>
    #include <ws2tcpip.h>

    struct addrinfo *result = NULL, hints;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <netdb.h>

    struct sockaddr_in servaddr, cliaddr;
    struct hostent *server;
    socklen_t clientlen;

    typedef int SOCKET;
    int WSAGetLastError(void) {
        return errno;
    }
#endif

typedef struct castinfo_ts {
    uint64_t sockversion;
    char hostname[16];
} castinfo_ts;

typedef struct header_ts {
    uint64_t size;
    uint8_t type;
} header_ts;

typedef struct file_ts {
    size_t size;
    char *data;
} file_ts;

void exitsock(const char *message, int error) {
    // Prints message to standard error and exits with error.

    fprintf(stderr, message);
    if(error != 0) fprintf(stderr, "Error code: %d\n", error);
    exit(-1);
}

void *safealloc(void *memory, size_t size) {
    // Performs malloc/realloc while also checking the resulting pointers.

    if(memory == NULL) memory = malloc(size);
    else memory = realloc(memory, size);
    if(memory == NULL) exitsock("Socketeer encountered a memory issue.\n", 0);

    return memory;
}

SOCKET serverinit(SOCKET socket) {
    // Initialises a server on the passed in socket.

    #ifdef _WIN32
        int retcode = bind(socket, result->ai_addr, (int) result->ai_addrlen);
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to bind a socket.\n", WSAGetLastError());
        freeaddrinfo(result);

        if((retcode = listen(socket, SOMAXCONN)) == SOCKET_ERROR) exitsock("Socketeer failed to listen on a socket.\n", WSAGetLastError());
        SOCKET clientsock = accept(socket, NULL, NULL);

        if(clientsock == INVALID_SOCKET) exitsock("Socketeer failed to accept a connection.\n", WSAGetLastError());
    #else
        if(bind((int) socket, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) exitsock("Socketeer failed to bind a socket.\n", 0);
        listen((int) socket, 5);

        SOCKET clientsock = (SOCKET) accept((int) socket, (struct sockaddr*) &cliaddr, &clientlen);
        if(clientsock < 0) exitsock("Socketeer failed to accept new connections.\n", 0);
    #endif

    return clientsock;
}

void clientinit(SOCKET socket) {
    // Initialises a client on the passed in socket.

    int retcode;

    #ifdef _WIN32
        retcode = connect(socket, result->ai_addr, (int) result->ai_addrlen);
        if(retcode == SOCKET_ERROR) exitsock("Socketeer failed to connect.\n", WSAGetLastError());
    #else
        bcopy((char*) server->h_addr, (char*) &servaddr.sin_addr.s_addr, server->h_length);
        retcode = connect((int) socket, (struct sockaddr*) &servaddr, sizeof(servaddr));
        if(retcode < 0) exitsock("Socketeer encountered an error connecting.\n", 0);
    #endif
}

SOCKET commoninit(char **argv, int type) {
    // Initiates a socket and returns it depending on type.

    #ifdef _WIN32
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int retcode;

        if(type == SERVERMODE) {
            hints.ai_flags = AI_PASSIVE;
            hints.ai_family = AF_INET;
            retcode = getaddrinfo(NULL, argv[2], &hints, &result);
        } else if(type == CLIENTMODE) {
            hints.ai_family = AF_UNSPEC;
            retcode = getaddrinfo(argv[2], argv[3], &hints, &result);
        }

        if(retcode != 0) exitsock("Socketeer failed to get address info.\n", -1);
        SOCKET commonsock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(commonsock == INVALID_SOCKET) exitsock("Socketeer failed to create a socket.\n", WSAGetLastError());
    #else
        SOCKET commonsock = (SOCKET) socket(AF_INET, SOCK_STREAM, 0);
        if(commonsock < 0) exitsock("Socketeer failed to create a socket.\n", -1);

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;

        if(type == SERVERMODE) {
            servaddr.sin_addr.s_addr = INADDR_ANY;
            servaddr.sin_port = htons(atoi(argv[2]));
        } else if(type == CLIENTMODE) {
            server = gethostbyname(argv[2]);
            servaddr.sin_port = htons(atoi(argv[3]));
        }
    #endif

    return commonsock;
}

ssize_t posixsend(SOCKET socket, void *buffer, size_t length, int flags, int datatype) {
    // Runs appropriate send() depending on OS.

    ssize_t bytes;

    #ifdef _WIN32
        bytes = (ssize_t) send(socket, (char*) buffer, (int) length, flags);
    #else
        bytes = send((int) socket, buffer, length, flags);
    #endif

    if(bytes != (ssize_t) length) {
        switch(datatype) {
            case HEADERS:
                exitsock("Socketeer failed to send header data.\n", -1);
                break;
            default:
                exitsock("Socketeer failed to send data.\n", -1);
                break;
        }
    }

    return bytes;
}

ssize_t posixrecv(SOCKET socket, void *buffer, size_t length, int flags, int datatype) {
    // Runs appropriate recv() depending on OS.

    ssize_t bytes;

    #ifdef _WIN32
        bytes = (ssize_t) recv(socket, (char*) buffer, (int) length, flags);
    #else
        bytes = recv((int) socket, buffer, length, flags);
    #endif

    if(bytes != (ssize_t) length && bytes > 0LL) {
        switch(datatype) {
            case HEADERS:
                exitsock("Socketeer failed to receive header data.\n", -1);
                break;
            default:
                exitsock("Socketeer failed to receive data.\n", -1);
                break;
        }
    }

    return bytes;
}

int socketdestruct(SOCKET socket) {
    // Runs appropriate routines to close a socket depending on OS.

    #ifdef _WIN32
        return closesocket(socket);
    #else
	return shutdown((int) socket, SHUT_RDWR);
    #endif
}

file_ts readfile(char *filepath) {
    // Returns a struct with file size and pointer to data.

    FILE *fstream = fopen(filepath, "rb");
    if(fstream == NULL) return (file_ts) {0, NULL};

    file_ts file;
    fseek(fstream, 0L, SEEK_END);
    file.size = (size_t) ftell(fstream);
    fseek(fstream, 0L, 0);

    file.data = (char*) safealloc(NULL, file.size);
    fread(file.data, 1, file.size, fstream);
    fclose(fstream);
    return file;
}
