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
        char recvbuf[TERMINALMAX];               // Setup a recv buffer.
        int numbytes = 1;

        printf("Connection established with client.\n");

        while(numbytes > 0) {
            memset(recvbuf, 0, TERMINALMAX);                    // Clear buffer data.
            numbytes = recv(clients, recvbuf, TERMINALMAX, 0);  // Move data into recv buffer.
            printf("Message: %s\n", recvbuf);                   // Print data to terminal.
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

        char sendbuf[TERMINALMAX];               // Setup send buffer.
        int numbytes;                            // Number of bytes sent.

        printf("Connection established with server.\n");

        while(true) {
            printf("Message: ");
            fetchinput(sendbuf);

            // Commands.
            if(strcmp(sendbuf, ":sendfile") == 0 || strcmp(sendbuf, ":sendfile\n") == 0) {
                printf("Type absolute path to file.\n");
                char *abspath = (char*) safealloc(NULL, TERMINALMAX);       // Create buffer for filepath.
                fetchinput(abspath);                                        // Write path into abspath buffer.

                fileattr_ts file = readfile(abspath);                       // Fetch file data pointer and size.
                numbytes = send(conn, file.data, file.size, 0);             // Send data down the network.
                free(file.data);                                            // Free the file data array.
                free(abspath);                                              // Free filepath buffer.             
            } else numbytes = send(conn, sendbuf, strlen(sendbuf) + 1, 0);  // If no command, send message buffer instead.

            if(numbytes == SOCKET_ERROR) {
                fprintf(stderr, "Something went wrong with Socketeer, code %d.\n", WSAGetLastError());
                exitsock(setupdata.result, conn, 1);
            }

            printf("Sent %d bytes.\n\n", numbytes);
        }
    }

    return 0;
}