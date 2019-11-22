#include "socketeer.h"
#include <string.h>

/* Usage of Socketeer
socketeer server [port]
socketeer client [address] [port] */

int main(int argc, char **argv) {
    WSADATA wsadata; // Does Windows Sockets work?
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
        pthread_t tidserver;
        pthread_create(&tidserver, NULL, recvthread, (void*) argv);
        pthread_join(tidserver, NULL);
    }

    else if(strcmp(argv[1], "client") == 0) {
        pthread_t tidclient;
        pthread_create(&tidclient, NULL, sendthread, (void*) argv);
        pthread_join(tidclient, NULL);
    }

    return 0;
}