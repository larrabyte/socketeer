#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <winbase.h>

    typedef int socklen_t;

    // Calls WSAGetLastError().
    int lasterror(void) {
        return WSAGetLastError();
    }

    // Calls the OS' version of sleep().
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
    #include <arpa/inet.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <errno.h>
    #include <netdb.h>

    typedef int SOCKET;

    // Returns errno.
    int lasterror(void) {
        return errno;
    }

    // Calls the OS' version of sleep().
    unsigned int socksleep(unsigned int seconds) {
        return sleep(seconds);
    }
#endif

// Runs send() with appropriate arguments, depending on OS.
ssize_t socksend(SOCKET socket, void *buffer, size_t length, int flags) {
    #ifdef _WIN32
        return send(socket, (char*) buffer, (int) length, flags);
    #else
        return send(socket, buffer, length, flags);
    #endif
}

// Runs recv() with appropriate arguments, depending on OS.
ssize_t sockrecv(SOCKET socket, void *buffer, size_t length, int flags) {
    #ifdef _WIN32
        return recv(socket, (char*) buffer, (int) length, flags);
    #else
        return recv(socket, buffer, length, flags);
    #endif
}
