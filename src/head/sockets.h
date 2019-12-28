#pragma once

#include "compatlayer.h"

enum socktype { UNINITIALISED, TCPSERVER, TCPCLIENT, UDPRECVER, UDPCASTER };
struct sockaddr_in serveraddr, clientaddr;

SOCKET tcpsocketinit(const char *address, const char *portstr, enum socktype stype);
SOCKET udpsocketinit(int portno, enum socktype stype);
