#include "ClientCore.h"

ClientCore::ClientCore(const char* IP, const char* port)
    :
    mainSocket(INVALID_SOCKET)
{
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }

    ADDRINFOA clientHints = makeHints(AF_INET, SOCK_STREAM, 0, AI_CANONNAME);
    ADDRINFOA* clientAddrinfo = makeAddrInfo(IP, port, clientHints);
    displayAddrinfo(clientAddrinfo);

    mainSocket = getSocketAndConnect(clientAddrinfo);
    if (mainSocket != INVALID_SOCKET) {
    //    InetNtopA(clientTypeAddrinfo->ai_family, get_in_addr(reinterpret_cast<sockaddr*>(clientTypeAddrinfo->ai_addr)), array[IP], INET6_ADDRSTRLEN);
        std::cout << "Connected to :" << IP << "!" << std::endl;
    }
    else {
        std::cout << "connection failed!" << std::endl;
    }
    FreeAddrInfoA(clientAddrinfo);
}

ClientCore::~ClientCore() {
    if (listenerThr.joinable()) {
        listenerThr.join();
    }
    WSACleanup();
}

ADDRINFOA* ClientCore::makeAddrInfo(PCSTR nodename, PCSTR servicename, const ADDRINFOA& hints) {
    ADDRINFOA* result = NULL;
    //call getaddrinfo
    DWORD dwRetval;
    if (dwRetval = GetAddrInfoA(nodename, servicename, &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed with error: " << dwRetval << std::endl;
        return NULL;
    }
    std::cout << "getaddrinfo returned success" << std::endl;
    return result;
}

ADDRINFOA ClientCore::makeHints(const int& family, const int& socktype, const int& protocol, const int& flags) {
    ADDRINFOA hints;
    // Setup the hints address info structure
    // which is passed to the getaddrinfo() function
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_protocol = protocol;
    hints.ai_flags = flags;
    return hints;
}

void ClientCore::displayAddrinfo(ADDRINFOA* addrinfoStruct) {
    SOCKADDR_IN* sockaddr_ipv4;
    SOCKADDR_IN6* sockaddr_ipv6;
    char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
    char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
//    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);

    // Retrieve each address and print out the hex bytes
    addrinfo* ptr = addrinfoStruct;
    for (int i = 0; ptr != NULL; ptr = ptr->ai_next) {
        std::cout << "getaddrinfo response # " << ++i << std::endl;
        std::cout << "\tFlags: 0x" << ptr->ai_flags << std::endl;
        std::cout << "\tFamily: ";
        switch (ptr->ai_family) {
        case AF_UNSPEC:
            std::cout << "Unspecified" << std::endl;
            break;
        case AF_INET:
            std::cout << "AF_INET (IPv4)" << std::endl;
            sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
            InetNtopA(AF_INET, &sockaddr_ipv4->sin_addr, ip4, INET_ADDRSTRLEN);
            std::cout << "\tIPv4 address " << ip4 << std::endl;
            break;
        case AF_INET6:
            std::cout << "AF_INET6 (IPv6)" << std::endl;

            sockaddr_ipv6 = reinterpret_cast<sockaddr_in6*>(ptr->ai_addr);
            InetNtopA(AF_INET6, &sockaddr_ipv6->sin6_addr, ip6, INET6_ADDRSTRLEN);
            std::cout << "\tIPv6 address " << ip6 << std::endl;
            break;
        case AF_NETBIOS:
            std::cout << "AF_NETBIOS(NetBIOS)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_family << std::endl;
            break;
        }
        std::cout << "\tSocket type: ";
        switch (ptr->ai_socktype) {
        case 0:
            std::cout << "Unspecified" << std::endl;
            break;
        case SOCK_STREAM:
            std::cout << "SOCK_STREAM (stream)" << std::endl;
            break;
        case SOCK_DGRAM:
            std::cout << "SOCK_DGRAM (datagram)" << std::endl;
            break;
        case SOCK_RAW:
            std::cout << "SOCK_RAW (raw)" << std::endl;
            break;
        case SOCK_RDM:
            std::cout << "SOCK_RDM (reliable message datagram)" << std::endl;
            break;
        case SOCK_SEQPACKET:
            std::cout << "SOCK_SEQPACKET (pseudo-stream packet)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_socktype << std::endl;
            break;
        }
        std::cout << "\tProtocol: ";
        switch (ptr->ai_protocol) {
        case 0:
            std::cout << "Unspecified" << std::endl;
            break;
        case IPPROTO_TCP:
            std::cout << "IPPROTO_TCP (TCP)" << std::endl;
            break;
        case IPPROTO_UDP:
            std::cout << "IPPROTO_UDP (UDP)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_protocol << std::endl;
            break;
        }
        std::cout << "\tLength of this sockaddr: " << ptr->ai_addrlen << std::endl;
        if (ptr->ai_canonname != NULL) {
            std::cout << "\tCanonical name: " << ptr->ai_canonname << std::endl;
        }
    }
}

SOCKET ClientCore::getSocketAndConnect(ADDRINFOA* addrinfoStruct)
{
    ADDRINFOA* p;
    SOCKET s = INVALID_SOCKET;
    // loop through all the results and connect to the first we can
    for (p = addrinfoStruct; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "Socket error :" << WSAGetLastError() << std::endl;
            continue;
        }

        if (!makeConnection(s, p)) {
            closeConnection(s);
            continue;
        }

        break;
    }
    if (p == NULL) {
        std::cout << "Failed to connect" << std::endl;
        return -1;
    }
    return s;
}

bool ClientCore::makeConnection(const SOCKET& socket, ADDRINFOA* addrinfo) {
    if (int errmsg = connect(socket, addrinfo->ai_addr, addrinfo->ai_addrlen) != 0) {
        std::cerr << WSAGetLastError() << " Connect Error" << std::endl;
        return false;
    }
    std::cout << "connection established!" << std::endl;
    return true;
}

void ClientCore::closeConnection(const SOCKET& socket) {
    if (int errmsg = closesocket(socket) != 0) {
        std::cerr << WSAGetLastError() << " Close socket Error" << std::endl;
        return;
    }
    std::cout << "connection closed!" << std::endl;
}

bool ClientCore::sendMessage(const char* msg, const int& flags) {
    int result = 0;
    int len = strlen(msg);
    int totalSent = 0;
    while (totalSent < len) {
        result = send(mainSocket, msg + totalSent, len - totalSent, flags);
        if (totalSent == -1) {
            std::cerr << "Send error : " << WSAGetLastError() << std::endl;
            return false;
        }
        totalSent += result;
    }
    return true;
}

int ClientCore::receiveMessage(char* msg, const int& maxmsglen, const SOCKET& socket, const int& flags) {
    int result = 0;
    result = recv(socket, msg, maxmsglen - 1, flags);
    if (result == -1) {
        std::cerr << "Recv error : " << WSAGetLastError() << std::endl;
    }
    return result;
}

void* ClientCore::get_in_addr(sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(reinterpret_cast<sockaddr_in*>(sa)->sin_addr);
    }

    return &(reinterpret_cast<sockaddr_in6*>(sa)->sin6_addr);
}

void ClientCore::listener() {
    const int buffersize = 4096;
    char receivebuffer[buffersize];
    int numbytes = 0;
    while (true) {
        numbytes = receiveMessage(receivebuffer, buffersize, mainSocket, 0);
        if (numbytes > 0) {
            receivebuffer[numbytes] = '\0';
            std::cout << receivebuffer << std::endl;
        }
        if (numbytes == SOCKET_ERROR) {
            std::cout << "Server disconnected" << std::endl;
            return;
        }
    }
}

void ClientCore::startListener() {
    // thread fills the ClientInfo form and dispatches conenctions to separate threads
    listenerThr = std::thread([this](){this->listener();});
}

void ClientCore::disconnect() {
    closeConnection(mainSocket);
}
