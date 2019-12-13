#pragma once

#include "socketeer.h"
#include <inttypes.h>
#include <stdbool.h>

// Performs data receiving functions.
void recvthread(void *args) { 
    SOCKET *socket = (SOCKET*) args;
    struct header header;
    ssize_t numbytes = 1;
    char *databuffer;

    while(numbytes > 0) {
        numbytes = sockrecv(*socket, &header, sizeof(header), 0);
        if(sentbytes != sizeof(header)) exitsock("Socketeer failed to send header data.\n", lasterror());

        databuffer = (char*) safealloc(NULL, header.size);
        numbytes = sockrecv(*socket, databuffer, header.size, MSG_WAITALL);
        if(sentbytes != header.size) exitsock("Socketeer failed to send data.\n", lasterror());

        if(header.type == TEXT) printf("Remote: %s\n", databuffer);

        else if(header.type == RAWDATA) {
            FILE *fstream = fopen("file.raw", "wb");
            if(fstream == NULL) fprintf(stderr, "Socketeer failed to open a new file.\n");
            else {
                fwrite(databuffer, 1, header.size, fstream);
                fclose(fstream);
            }

            #ifdef _WIN32
                printf("Data has been written to file.raw (%I64d bytes.)\n", numbytes);
            #else 
                printf("Data has been written to file.raw (%I64ld bytes.)\n", numbytes);
            #endif
        }

        free(databuffer);
    }

    if(numbytes == 0) exitsock("Connection has been closed by remote end.\n", 0);
    else exitsock("Socketeer encounted an error with the connection.\n", lasterror());
}