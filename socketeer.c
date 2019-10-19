#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdio.h>

#define BUFFERSIZE 1048576 // megabyte

/* Usage of Socketeer
socketeer server [port]
socketeer client [address] [port]

For memory leak testing :)
strcpy(buffer, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
*/

size_t datasize;

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
    } 
    
    return memory;
}

void *fetchinput() {
    // Allocate some buffers and cache the size.
    char *buffer = (char*) safealloc(NULL, BUFFERSIZE);
    size_t sizenow = 0;
    int onechar;
    
    while((onechar = fgetc(stdin)) != EOF && onechar != '\n') {
        buffer[sizenow++] = (char) onechar;
    }

    // NULL terminate & return buffer.
    buffer[sizenow] = '\0';
    return buffer;
}

void *parsecmd(char *msgbuffer, SOCKET sockptr) {
    if(strcmp(msgbuffer, ":quit") == 0 || strcmp(msgbuffer, ":quit\n") == 0) checkret(0, 0, 1, NULL, sockptr);

    if(strcmp(msgbuffer, ":sendfile") == 0 || strcmp(msgbuffer, ":sendfile\n") == 0) {
        printf("Type absolute path to file.\n");
        
        // Get path and then try and open path.
        char *abspath = (char*) fetchinput();
        FILE *fstream = fopen(abspath, "rb");
        if(fstream == NULL) checkret(69001, 0, 1, NULL, sockptr);
        free(abspath);
        
        // Get filesize and cache it.s
        fseek(fstream, 0L, SEEK_END);
        datasize = ftell(fstream);
        fseek(fstream, 0L, 0);

        // Allocate a buffer, read the data and return it.
        char *rawdata = (char*) safealloc(NULL, datasize);
        fread(rawdata, 1, datasize, fstream);
        fclose(fstream);
        return rawdata;
    }

    return NULL;
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
    // Setup addrinfo struct for creating a socket.
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_UNSPEC;

    // Fetch address and port information and then make a socket.
    int retcode = getaddrinfo(argv[2], argv[3], &hints, &result);
    checkret(retcode, 0, 0, NULL, INVALID_SOCKET);
    SOCKET conn = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(conn == INVALID_SOCKET) checkret(WSAGetLastError(), 0, 1, result, INVALID_SOCKET);
    retcode = connect(conn, result->ai_addr, (int) result->ai_addrlen);
    if(retcode == SOCKET_ERROR) checkret(WSAGetLastError(), 0, 1, NULL, conn);
    printf("Remote connection established. Type :quit to quit.\n");

    // Get user input, parse and send it!
    while(1) {
        printf("Message: ");
        char *msgbuffer = (char*) fetchinput();
        char *databuffer = (char*) parsecmd(msgbuffer, conn);
        
        // Send the msgbuffer if there's no data at databuffer.
        if(databuffer == NULL) { retcode = send(conn, msgbuffer, strlen(msgbuffer) + 1, 0); }
        else { retcode = send(conn, databuffer, datasize, 0); }

        // Check for errors, print bytes sent and free up for the next round.
        if(retcode == SOCKET_ERROR) checkret(WSAGetLastError(), 0, 1, NULL, conn);
        printf("Sent %d bytes.\n\n", retcode);
        free(databuffer);
        free(msgbuffer);
    }
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