#pragma once
#include "Ws2tcpip.h"
#include <iostream>

#define OUTPUTWSAERRSTRING(string)  std::cerr << string << std::endl

static void printWSAError() {
	int error = WSAGetLastError();
    switch (error) {
        case WSAECONNREFUSED:
            OUTPUTWSAERRSTRING("Error: <Connection refused>");
            break;
        case WSAENOTCONN:
            OUTPUTWSAERRSTRING("Error: <Socket not connected>");
            break;
        case WSAENOTSOCK:
            OUTPUTWSAERRSTRING("Error: <Invalid socket>");
            break;
        case WSAEINVAL:
            OUTPUTWSAERRSTRING("Error: <Socket not bound>");
            break;
        case WSAEADDRINUSE:
            OUTPUTWSAERRSTRING("Error: <Socket already in use>");
            break;
        case WSAECONNRESET:
            OUTPUTWSAERRSTRING("Error: <Lost connection to Remote client>");
            break;
        default:
            std::cerr << "ERROR # " << error << std::endl;
    }
}