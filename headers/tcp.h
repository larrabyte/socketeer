#pragma once

#include "posixcompat.h"
#include "sockutils.h"
#include "sockets.h"
#include <inttypes.h>
#include <stdio.h>

ssize_t sentbytes;

// Interprets user input. Requires pointers to socket and header struct.
// This is because this function can send data based on interpreted commands.
int interpretcmd(SOCKET *socket, struct header *header, char *userinput) {
    if(userinput[0] == ':') {
        if(strcmp(userinput, ":sendfile") == 0 || strcmp(userinput, ":sendfile\n") == 0) {
            printf("Type path to file.\n");
            fgets(userinput, TERMINALMAX, stdin);
            userinput[strcspn(userinput, "\n")] = '\0';

            struct fileattr file = readfile(userinput);
            if(file.data == NULL) fprintf(stderr, "Invalid path provided.\n\n");
            else {
                header->size = file.size;
                header->type = RAWDATA;

                sentbytes = socksend(*socket, header, sizeof(*header), 0);
                if(sentbytes != sizeof(*header)) exitsock("Socketeer failed to send header data.\n", lasterror());

                sentbytes = socksend(*socket, file.data, header->size, 0);
                if(sentbytes != header->size) exitsock("Socketeer failed to send data.\n", lasterror());

                free(file.data);
            }
        }

        else if(strcmp(userinput, ":quit") == 0 || strcmp(userinput, ":quit\n") == 0) {
            closesocket(*socket);
            exitsock("Socketeer now exiting.\n", 0);
        }

        else fprintf(stderr, "Invalid command.\n\n");
        return 1;
    }

    return 0;
}

// Performs data sending functions using the UDP protocol.
void sendonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    struct castinfo castdata = {1, "192.168.0.96"};
    printf("Broadcasting...\n");

    while(1) {
        sentbytes = sendto(*socket, (char*) &castdata, sizeof(castdata), 0, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
        socksleep(1);
    }
}

// Performs data sending functions using the TCP protocol.
void sendontcp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    char termbuf[TERMINALMAX];
    struct header header;

    printf("Connection established.\n");

    while(1) {
        printf("Message: ");
        fgets(termbuf, TERMINALMAX, stdin);
        termbuf[strcspn(termbuf, "\n")] = '\0';

        int iscmd = interpretcmd(socket, &header, termbuf);
        if(!iscmd) {
            header.size = strlen(termbuf) + 1;
            header.type = TEXT;

            sentbytes = socksend(*socket, &header, sizeof(header), 0);
            if(sentbytes != sizeof(header)) exitsock("Socketeer failed to send header data.\n", lasterror());

            sentbytes = socksend(*socket, termbuf, header.size, 0);
            if(sentbytes != header.size) exitsock("Socketeer failed to send data.\n", lasterror());
        }
    }
}

// Performs data receiving functions using the TCP protocol.
void recvontcp(void *args) { 
    SOCKET *socket = (SOCKET*) args;
    struct header header;
    char *databuffer;
    ssize_t numbytes;

    printf("Connection established.\n");

    while(1) {
        numbytes = sockrecv(*socket, &header, sizeof(header), 0);
        if(numbytes == 0) exitsock("Connection has been closed by remote end.\n", 0);
        if(numbytes != sizeof(header)) exitsock("Socketeer failed to receive header data.\n", lasterror());

        databuffer = (char*) safealloc(NULL, header.size);
        numbytes = sockrecv(*socket, databuffer, header.size, MSG_WAITALL);
        if(numbytes != header.size) exitsock("Socketeer failed to receive data.\n", lasterror());

        if(header.type == TEXT) printf("Remote: %s\n", databuffer);

        else if(header.type == RAWDATA) {
            FILE *fstream = fopen("file.raw", "wb");
            if(fstream == NULL) fprintf(stderr, "Socketeer failed to open a new file.\n");
            else {
                fwrite(databuffer, 1, header.size, fstream);
                fclose(fstream);
            }

            printf("Data has been written to file.raw (%" PRId64 " bytes.)\n", numbytes);
        }

        free(databuffer);
    }
}