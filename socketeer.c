#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define TERMINALMAX 4096   // Maximum characters inside a terminal.

/* Usage of Socketeer
socketeer server [port]
socketeer client [address] [port] */

typedef struct packet_ts {  // Struct typedef for a Socketeer packet.
    unsigned char *data;
    size_t length;
} packet_ts;

typedef struct setupdata_ts {  // Struct typedef for passing setup data.
    struct addrinfo *result;
    SOCKET socketeer;
} setupdata_ts;

// Function declarations so that main() doesn't complain.
void exitsock(struct addrinfo *result, SOCKET socket, int code);
setupdata_ts commoninit(char **argv, int type);
void *safealloc(void *memory, size_t size);
SOCKET serverinit(setupdata_ts sockinfo);
void clientinit(setupdata_ts sockinfo);
void fetchinput(char *buffer);

int main(int argc, char **argv) {
    WSADATA wsadata; // Does Windows Sockets work?
    setupdata_ts setupdata;
    int retc = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if(retc != 0) {
        fprintf(stderr, "Windows Sockets failed to startup.\n");
        exitsock(NULL, INVALID_SOCKET, 1);
    }

    if(argc == 1) { // If no arguments have been passed in.
        printf("Arguments required. To use Socketeer in server mode:\n");
        printf("    socketeer server [port]\n");
        printf("To use Socketeer in client mode:\n");
        printf("    socketeer client [address] [port]\n");
    }

    else if(strcmp(argv[1], "server") == 0) {
        setupdata = commoninit(argv, 0);         // Setup a standard socket.
        SOCKET clients = serverinit(setupdata);  // Initialise server.
        char recvbuf[TERMINALMAX];               // Setup a recv buffer.
        int numbytes = 1;

        printf("Connection established with client.\n");

        while(numbytes > 0) {
            numbytes = recv(clients, recvbuf, TERMINALMAX, 0);
            printf("Message: %s\n", recvbuf);
        }

        if(numbytes == 0) {
            printf("Socketeer's connection has been closed.\n");
            exitsock(setupdata.result, clients, 0);
        } else {
            fprintf(stderr, "Socketeer has encountered an error, code %d.\n", WSAGetLastError());
            exitsock(setupdata.result, clients, 1);
        }
    }

    else if(strcmp(argv[1], "client") == 0) {
        setupdata = commoninit(argv, 1);        // Setup a standard socket.
        SOCKET conn = setupdata.socketeer;      // Prepare for later on.
        clientinit(setupdata);                  // Initialise client.
        int numbytes;

        char *sendbuf = (char*) safealloc(NULL, TERMINALMAX);
        if(sendbuf == NULL) {
            fprintf(stderr, "Socketeer failed to allocate a send buffer.\n");
            exitsock(setupdata.result, conn, 1);
        }

        printf("Connection established with server.\n");

        while(true) {
            printf("Message: ");
            fetchinput(sendbuf);
            numbytes = send(conn, sendbuf, strlen(sendbuf) + 1, 0);
            if(numbytes == SOCKET_ERROR) {
                fprintf(stderr, "Something went wrong with Socketeer, code %d.\n", WSAGetLastError());
                exitsock(setupdata.result, conn, 1);
            }

            printf("Sent %d bytes.\n\n", numbytes);
        }
    }

    return 0;
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