#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <winbase.h>

    int lasterror(void) {
        return WSAGetLastError();
    }

    void socksleep(unsigned int seconds) {
        return Sleep(seconds * 1000);
    }
#else
    #define closesocket close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1

    #ifdef __USE_XOPEN2K  // GCC complains if xopen2k is already defined.
    #undef __USE_XOPEN2K  // This line undefines it for the statements below.
    #endif                // Endif.

    #define __USE_XOPEN2K  // Stops Visual Studio Code complaining about incomplete types.

    #include <sys/socket.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <netdb.h>

    typedef int SOCKET;

    int lasterror(void) {
        return errno;
    }

    void socksleep(unsigned int seconds) {
        return sleep(seconds);
    }
#endif