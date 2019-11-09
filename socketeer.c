#include "socketeer.h"
#include <stdbool.h>
#include <string.h>

/* Usage of Socketeer
socketeer server [port]
socketeer client [address] [port] */

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
        int numbytes = 1;

        printf("Connection established with client.\n");
        size_t recvdatasize;
        char *recvbuf;

        while(numbytes > 0) {
            numbytes = recv(clients, (char*) &recvdatasize, sizeof(size_t), 0);
            
            if(numbytes != sizeof(size_t)) {
                fprintf(stderr, "Socketeer failed to receive buffer size.\n");
                exitsock(setupdata.result, clients, 1);
            } else {
                recvbuf = (char*) safealloc(NULL, recvdatasize);
                numbytes = recv(clients, recvbuf, recvdatasize, 0);
            }

            printf("Message: %s\n", recvbuf);
            free(recvbuf);
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
        setupdata = commoninit(argv, 1);         // Setup a standard socket.
        SOCKET conn = setupdata.socketeer;       // Set conn to the setup socket.
        clientinit(setupdata);                   // Initialise client.

        char termbuf[TERMINALMAX];               // Setup terminal buffer.
        int numbytes;                            // Number of bytes sent.

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
                numbytes = send(conn, (char*) &file.size, sizeof(size_t), 0);
                
                if(numbytes != sizeof(size_t)) {
                    fprintf(stderr, "Socketeer failed to send buffer size.\n");
                    exitsock(setupdata.result, conn, 1);
                } else numbytes = send(conn, file.data, file.size, 0);
                
                free(file.data);
            } else {
                size_t bufsize = strlen(termbuf) + 1;
                numbytes = send(conn, (char*) &bufsize, sizeof(size_t), 0);
                if(numbytes != sizeof(size_t)) {
                    fprintf(stderr, "Socketeer failed to send buffer size.\n");
                    exitsock(setupdata.result, conn, 1);
                } else numbytes = send(conn, termbuf, bufsize, 0);
            }

            if(numbytes == SOCKET_ERROR) {
                fprintf(stderr, "Something went wrong with Socketeer, code %d.\n", WSAGetLastError());
                exitsock(setupdata.result, conn, 1);
            }

            printf("Sent %d bytes.\n\n", numbytes);
        }
    }

    return 0;
}