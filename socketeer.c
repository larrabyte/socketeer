#include "headers/sockets.h"
#include "headers/tcp.h"
#include "headers/udp.h"

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

    if(argc == 1) { // If no arguments have been passed in.
        printf("Arguments required. To use Socketeer in TCP server mode:\n");
        printf("    %s tcpserver [port]\n", argv[0]);
        printf("To use Socketeer in TCP client mode:\n");
        printf("    %s tcpclient [address] [port]\n\n", argv[0]);
        printf("To use Socketeer in UDP server mode:\n");
        printf("    %s udpcaster [port]\n", argv[0]);
        printf("To use Socketeer in UDP client mode:\n");
        printf("    %s udprecver [port]\n", argv[0]);
    }

    else if(argc == 3 && strcmp(argv[1], "tcpserver") == 0) {
        // argv[0]      argv[1]      argv[2]
        // socketeer    tcpserver    1047

        sockfd = tcpsocketinit(NULL, argv[2], TCPSERVER);
        recvontcp(&sockfd);
    }

    else if(argc == 4 && strcmp(argv[1], "tcpclient") == 0) {
        // argv[0]      argv[1]      argv[2]      argv[3]
        // socketeer    tcpclient    127.0.0.1    1047

        sockfd = tcpsocketinit(argv[2], argv[3], TCPCLIENT);
        sendontcp(&sockfd);
    }

    else if(argc == 3 && strcmp(argv[1], "udpcaster") == 0) {
        // argv[0]      argv[1]      argv[2]
        // socketeer    udpcaster    1047

        sockfd = udpsocketinit(atoi(argv[2]), UDPCASTER);
        sendonudp(&sockfd);
    }

    else if(argc == 3 && strcmp(argv[1], "udprecver") == 0) {
        // argv[0]      argv[1]      argv[2]
        // socketeer    udprecver    1047

        sockfd = udpsocketinit(atoi(argv[2]), UDPRECVER);
        recvonudp(&sockfd);
    }

    fprintf(stderr, "Invalid arguments.\n");
    return 0;
}