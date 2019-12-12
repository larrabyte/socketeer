#pragma once

#include "socketeer.h"
#include <inttypes.h>
#include <stdbool.h>

void recvthread(void *args) {
    // Performs data receiving functions.

    SOCKET *socket = (SOCKET*) args;
    ssize_t numbytes = 1;
    header_ts header;
    char *databuffer;

    while(true) {
        numbytes = posixrecv(*socket, (void*) &header, sizeof(header), 0, HEADERS);
        if(numbytes == 0LL) exitsock("Connection has been closed by remote end.\n", 0);

        databuffer = (char*) safealloc(NULL, header.size);
        numbytes = posixrecv(*socket, (void*) databuffer, header.size, MSG_WAITALL, 0);

        if(header.type == MESSAGE) printf("Remote: %s\n", databuffer);

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
}