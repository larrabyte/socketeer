#include "head/compatlayer.h"

// Calls the appropriate OS error handler.
int lasterror(void) {
    #ifdef _WIN32
        return WSAGetLastError();
    #else
        return errno;
    #endif
}

unsigned int socksleep(unsigned int seconds) {
    #ifdef _WIN32
        Sleep(seconds * 1000);
        return seconds;
    #else
        return sleep(seconds);
    #endif
}

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
