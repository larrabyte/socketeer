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

void servermain(char **argv) {

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