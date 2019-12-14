#include "headers/sockets.h"
#include "headers/server.h"
#include "headers/client.h"

int main(int argc, char **argv) {
    #ifdef _WIN32
        DWORD cflags;
        WSADATA wsadata;
        int retcode = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if(retcode != 0) exitsock("Windows Sockets failed to startup.\n", -1);

        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if(console == INVALID_HANDLE_VALUE) exitsock("Socketeer failed to access console handle.\n", GetLastError());
        if(!GetConsoleMode(console, &cflags)) exitsock("Socketeer failed to cache console modes.\n", GetLastError());
        cflags |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        if(!SetConsoleMode(console, cflags)) exitsock("Socketeer failed to enable ANSI escape sequences.\n", GetLastError());
    #endif

    SOCKET sockfd;
    int portno;

    if(argc == 1) { // If no arguments have been passed in.
        printf("Arguments required. To use Socketeer in server mode:\n");
        printf("    socketeer server [port]\n");
        printf("To use Socketeer in client mode:\n");
        printf("    socketeer client [address] [port]\n");
    }

    else if(argc == 3 && strcmp(argv[1], "server") == 0) {
        // argv[0]      argv[1]   argv[2]
        // socketeer    server    1047

        sockfd = tcpsocketinit(NULL, argv[2], TCPSERVER);
        recvontcp(&sockfd);
    }

    else if(argc == 4 && strcmp(argv[1], "client") == 0) {
        // argv[0]      argv[1]   argv[2]      argv[3]
        // socketeer    client    127.0.0.1    1047

        sockfd = tcpsocketinit(argv[2], argv[3], TCPCLIENT);
        sendontcp(&sockfd);
    }

    fprintf(stderr, "Invalid arguments.\n");
    return 0;
}