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

    int lasterror(void) {
        return WSAGetLastError();
    }
#else
    #define closesocket close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <netdb.h>

    typedef int SOCKET;

    int lasterror(void) {
        return errno;
    }
#endif

enum socktype { UNINITIALISED, TCPSERVER, TCPCLIENT, UDPRECVER, UDPCASTER };

typedef struct header_ts {
    uint64_t size;
    uint8_t type;
} header_ts;

typedef struct file_ts {
    size_t size;
    char *data;
} file_ts;

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

// Returns a ready-to-use TCP socket based on socktype passed in and address/portno.
SOCKET tcpsocketinit(char **argv, enum socktype stype) {
    if(stype == UDPCASTER || stype == UDPRECVER) return (SOCKET) ~0;

    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int retcode;

    if(stype == TCPSERVER) {
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        retcode = getaddrinfo(NULL, argv[2], &hints, &result);
    } else if(stype == TCPCLIENT) {
        hints.ai_family = AF_UNSPEC;
        retcode = getaddrinfo(argv[2], argv[3], &hints, &result);
    }

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

// Runs appropriate send() depending on OS.
ssize_t posixsend(SOCKET socket, void *buffer, size_t length, int flags, int datatype) {
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

// Runs appropriate recv() depending on OS.
ssize_t posixrecv(SOCKET socket, void *buffer, size_t length, int flags, int datatype) {
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

// Returns a struct with file size and pointer to data.
file_ts readfile(char *filepath) {
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
