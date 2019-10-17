#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdio.h>

#define BUFFERSIZE 4096

/* Usage of Socketeer
socketeer server [port]
socketeer client [address] [port] */

void cleanexit(struct addrinfo *result, SOCKET socket, int code) {
    if(socket != INVALID_SOCKET) closesocket(socket);
    if(result != NULL) freeaddrinfo(result);
    WSACleanup();
    exit(code);
}

void checkret(int retcode, int ifsuccess, int ifexit, struct addrinfo *result, SOCKET socket) {
    if(retcode != ifsuccess) {
        fprintf(stderr, "Error: %d\n", retcode);
        cleanexit(result, socket, 1);
    } else if(retcode == ifsuccess && ifexit == 1) {
        printf("Socketeer is now exiting.\n");
        cleanexit(result, socket, 0);
    }
}

void *safealloc(void *memory, size_t size) {
    if(memory == NULL) memory = malloc(size);
    else memory = realloc(memory, size);

    if(memory == NULL) {
        fprintf(stderr, "Socketeer ran out of memory.\n");
        cleanexit(NULL, INVALID_SOCKET, 1);
    } else { return memory; }
}

void servermain(char **argv) {
    // Setup addrinfo struct for creating a socket.
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    // Fetch address/port information and then create a listener.
    int retcode = getaddrinfo(NULL, argv[2], &hints, &result);
    checkret(retcode, 0, 0, NULL, INVALID_SOCKET);
    SOCKET listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(listener == INVALID_SOCKET) checkret(WSAGetLastError(), 0, 1, NULL, INVALID_SOCKET);
    retcode = bind(listener, result->ai_addr, (int) result->ai_addrlen);
    if(retcode == SOCKET_ERROR) checkret(WSAGetLastError(), 0, 1, NULL, listener);
    freeaddrinfo(result);

    // Listen on the listener socket.
    retcode = listen(listener, SOMAXCONN);
    if(retcode == SOCKET_ERROR) checkret(WSAGetLastError(), 0, 1, NULL, listener);
    SOCKET clients = accept(listener, NULL, NULL);
    if(clients == INVALID_SOCKET) checkret(WSAGetLastError(), 0, 1, NULL, clients);
    printf("Connection accepted. Awaiting data...\n");

    // Setup a memory buffer.
    char *recvbuffer = (char*) safealloc(NULL, BUFFERSIZE);

    // Receive data.
    int recvcode = 1;
    while(recvcode > 0) {
        recvcode = recv(clients, recvbuffer, BUFFERSIZE, 0);
        printf("Message from client: %s\n", recvbuffer);
    }

    // Shutdown the connection.
    if(recvcode == 0) checkret(0, 0, 1, NULL, clients);
    else checkret(WSAGetLastError(), 0, 1, NULL, clients);
}

void clientmain(char **argv) {
    
}

int main(int argc, char **argv) {
    // Winsock work?    
    WSADATA wsadata;
    int retcode = WSAStartup(MAKEWORD(2, 2), &wsadata);
    checkret(retcode, 0, 0, NULL, INVALID_SOCKET);
    
    if(argc == 1) {
        printf("Arguments required. To use Socketeer in server mode:\n");
        printf("    socketeer server [port]\n");
        printf("To use Socketeer in client mode:\n");
        printf("    socketeer client [address] [port]\n");
    }

    else if(strcmp(argv[1], "server") == 0) servermain(argv);
    else if(strcmp(argv[1], "client") == 0) clientmain(argv);
    return 0;
}