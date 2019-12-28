#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <winbase.h>

    typedef int socklen_t;
#else
    #define closesocket close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1

    #ifdef __USE_XOPEN2K  // GCC complains if xopen2k is already defined.
    #undef __USE_XOPEN2K  // This line undefines it for the statements below.
    #endif                // Endif.

    #define __USE_XOPEN2K  // Stops Visual Studio Code complaining about incomplete types.

    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <errno.h>
    #include <netdb.h>

    typedef int SOCKET;
#endif

ssize_t socksend(SOCKET socket, void *buffer, size_t length, int flags);
ssize_t sockrecv(SOCKET socket, void *buffer, size_t length, int flags);
unsigned int socksleep(unsigned int seconds);
int lasterror(void);
