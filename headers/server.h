#pragma once

#include "socketeer.h"
#include <inttypes.h>

ssize_t numbytes;

void recvonudp(void *args) {
    SOCKET *socket = (SOCKET*) args;
    int clilen = sizeof(clientaddr);
    struct castinfo castdata;

    printf("Awaiting broadcast...\n");

    while(1) {
        numbytes = recvfrom(*socket, (char*) &castdata, sizeof(castdata), 0, (struct sockaddr*) &clientaddr, &clilen);
        printf("\nBroadcast received! Version %" PRIu64 ", hostname: %s\n", castdata.version, castdata.hostname);
    }
}

// Performs data receiving functions using the TCP protocol.
void recvontcp(void *args) { 
    SOCKET *socket = (SOCKET*) args;
    struct header header;
    char *databuffer;

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