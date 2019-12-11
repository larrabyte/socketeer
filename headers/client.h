#pragma once

#include "socketeer.h"
#include <stdbool.h>

int interpretcmd(SOCKET *socket, header_ts *header, char *userinput) {
    // Interprets user input for commands.

    if(userinput[0] == ':') {
        if(strcmp(userinput, ":sendfile") == 0 || strcmp(userinput, ":sendfile\n") == 0) {
            printf("Type path to file.\n");
            fgets(userinput, TERMINALMAX, stdin);
            userinput[strcspn(userinput, "\n")] = '\0';

            file_ts file = readfile(userinput);
            if(file.data == NULL) fprintf(stderr, "Invalid path provided.\n\n");
            else {
                header->size = file.size;
                header->type = RAWDATA;

                posixsend(*socket, (void*) header, sizeof(*header), 0, HEADERS);
                posixsend(*socket, (void*) file.data, header->size, 0, 0);
                free(file.data);
            }
        }

        else if(strcmp(userinput, ":quit") == 0 || strcmp(userinput, ":quit\n") == 0) {
            socketdestruct(*socket);
            exitsock("Socketeer now exiting.\n", 0);
        }

        else fprintf(stderr, "Invalid command.\n\n");
        return 1;
    }

    return 0;
}

void sendthread(void *args) {
    // Performs data sending functions.

    SOCKET *socket = (SOCKET*) args;
    char termbuf[TERMINALMAX];
    header_ts header;

    while(true) {
        printf("Message: ");
        fgets(termbuf, TERMINALMAX, stdin);
        termbuf[strcspn(termbuf, "\n")] = '\0';

        int iscmd = interpretcmd(socket, &header, termbuf);
        if(iscmd == 0) {
            header.size = strlen(termbuf) + 1;
            header.type = MESSAGE;

            posixsend(*socket, (void*) &header, sizeof(header), 0, HEADERS);
            posixsend(*socket, (void*) termbuf, header.size, 0, 0);
        }
    }
}