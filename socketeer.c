#include "headers/socketeer.h"
#include "headers/client.h"
#include "headers/server.h"

int main(int argc, char **argv) {
    #ifdef _WIN32
        WSADATA wsadata;
        int retcode = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if(retcode != 0) exitsock("Windows Sockets failed to startup.\n", -1);
    #endif

    SOCKET sockfd;

    if(argc == 1) { // If no arguments have been passed in.
        printf("Arguments required. To use Socketeer in server mode:\n");
        printf("    socketeer server [port]\n");
        printf("To use Socketeer in client mode:\n");
        printf("    socketeer client [address] [port]\n");
    }

    else if(strcmp(argv[1], "server") == 0) {
        sockfd = tcpsocketinit(argv, TCPSERVER);
        recvthread(&sockfd);
    }

    else if(strcmp(argv[1], "client") == 0) {
        sockfd = tcpsocketinit(argv, TCPCLIENT);
        sendthread(&sockfd);
    }

    fprintf(stderr, "Invalid arguments.\n");
    return 0;
}