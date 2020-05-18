#include "SocketCore.h"

ADDRINFOA* SocketCore::makeAddrInfo(PCSTR nodename, PCSTR servicename, const ADDRINFOA& hints) {
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

ADDRINFOA SocketCore::makeHints(const int& family, const int& socktype, const int& protocol, const int& flags) {
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

void SocketCore::displayAddrinfo(ADDRINFOA* addrinfoStruct) {
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

SOCKET SocketCore::getSocketAndBind(ADDRINFOA* addrinfoStruct) {
    ADDRINFOA* p;
    SOCKET s = INVALID_SOCKET;
    // loop through all the results and connect to the first we can
    for (p = addrinfoStruct; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "Socket error :" << WSAGetLastError() << std::endl;
            continue;
        }

        //  blocking of IPv6 server. Comment out if network supports IPv6
        if (p->ai_family != AF_INET) {
            continue;
        }

        if (bind(s, p->ai_addr, p->ai_addrlen) != 0) {
            std::cerr << WSAGetLastError() << " Bind Error" << std::endl;
            closeConnection(s);
            continue;

            // lose the pesky "Address already in use" error message
            //   const char yes = 1;
            //   if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
            //       perror("setsockopt");
            //   }
        }

        break;
    }
    if (p == NULL) {
        std::cout << "Failed to connect" << std::endl;
        return -1;
    }
    return s;
}

SOCKET SocketCore::getSocketAndConnect(ADDRINFOA* addrinfoStruct)
{
    ADDRINFOA* p;
    SOCKET s = INVALID_SOCKET;
    // loop through all the results and connect to the first we can
    for (p = addrinfoStruct; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "Socket error :" << WSAGetLastError() << std::endl;
            continue;
        }

        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            std::cerr << " Connect error : " << WSAGetLastError() << std::endl;
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

void SocketCore::makeConnection(const SOCKET& socket, ADDRINFOA* addrinfo) {
   if( int errmsg = connect(socket, addrinfo->ai_addr, addrinfo->ai_addrlen) != 0) {
       std::cerr << WSAGetLastError() << " Connect Error" << std::endl;
       return;
   }
   std::cout << "connection established!" << std::endl;
}

void SocketCore::closeConnection(const SOCKET& socket) {
    if (int errmsg = closesocket(socket) != 0) {
        std::cerr << WSAGetLastError() << " Close socket Error" << std::endl;
        return;
    }
    std::cout << "connection closed!" << std::endl;
}

void SocketCore::listenPort(const SOCKET& socket, const int& maxQueue) {
    if(int errmsg = listen(socket, maxQueue) != 0) {
        std::cout << "Listen Error : " << WSAGetLastError() << std::endl;
        return;
    }
    std::cout << "Server is listening for connections....." << std::endl;
}

SOCKET SocketCore::acceptConnection(const SOCKET& socket, sockaddr& sockaddrstorage, int& storageSize) {
    int result = accept(socket, &sockaddrstorage, &storageSize);
    return result;
    //  error checking done outside
}

bool SocketCore::sendMessage(const char* msg, const SOCKET& socket, const int& flags) {
    int result = 0;
    int len = strlen(msg);
    int totalSent = 0;
    while (totalSent < len) {
        result = send(socket, msg + totalSent, len-totalSent, flags);
        if (totalSent == -1) {
            std::cerr << "Send error : " << WSAGetLastError() << std::endl;
            return false;
        }
        totalSent += result;
    }
    return true;
}

int SocketCore::receiveMessage(char* msg, const int& maxmsglen, const SOCKET& socket, const int& flags) {
    int result = 0;
    result = recv(socket, msg, maxmsglen - 1, flags);
    if (result == -1) {
        std::cerr << "Recv error : " << WSAGetLastError() << std::endl;
    }
    return result;
}

void* SocketCore::get_in_addr(sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(reinterpret_cast<sockaddr_in*>(sa)->sin_addr);
    }

    return &(reinterpret_cast<sockaddr_in6*>(sa)->sin6_addr);
}

void SocketCore::connectionsListenerDispatcher(std::vector<ClientInfo*>* clients, SOCKET* socketin, std::vector<std::thread>* threads, std::queue<std::string>* messageQue, bool* stop, std::mutex* clientsMutex, std::mutex* msgQueMutex) {
    while (!(*stop)) {
        sockaddr remote_addr;
        int remoteaddr_size = sizeof(remote_addr);

        SOCKET dedicatedSocket = acceptConnection(*socketin,remote_addr, remoteaddr_size);
        if (dedicatedSocket == -1 && !(*stop)) {
            std::cerr << WSAGetLastError() << " Accept Error" << std::endl;
            continue;
        }
        if (*stop) {
            break;
        }
        char remoteIP[INET6_ADDRSTRLEN];
        char remotename[MAX_NAME_LENGTH];
        char remoteport[20];
        ClientInfo* client = new ClientInfo;
        getnameinfo(&remote_addr, remoteaddr_size, remotename, MAX_NAME_LENGTH, remoteport, 20, 0);
        InetNtopA(remote_addr.sa_family, SocketCore::get_in_addr(&remote_addr), remoteIP, INET6_ADDRSTRLEN);
        std::cout << "got a conenction from : " << remotename << " IP : " << remoteIP << std::endl;
        client->socket = dedicatedSocket;
        client->IPaddress = remoteIP;
        client->name = remotename;
        client->port = remoteport;

        clientsMutex->lock();
        clients->push_back(client);
        clientsMutex->unlock();
        threads->push_back(std::thread(serverListener, client, messageQue, msgQueMutex));
    }
}

void SocketCore::serverListener(ClientInfo* client, std::queue<std::string>* messageQue, std::mutex* msgQueMutex) {
    client->threadID = std::this_thread::get_id();
    client->connected = true;
    int numbytes = 0;
    while (true) {
        char receivedmessage[MAX_MESSAGE_SIZE];
        numbytes = receiveMessage(receivedmessage, MAX_MESSAGE_SIZE, client->socket, 0);
        if (numbytes > 0) {
            //  insert null-terminator
            receivedmessage[numbytes] = '\0';
            std::cout << client->name << " : " << receivedmessage << std::endl;
            //  fill FIFO with name(max 256 chars) and message
            auto tempNameStr = client->name;
            tempNameStr.resize(MAX_NAME_LENGTH);
            msgQueMutex->lock();
            messageQue->push(tempNameStr + std::string{ receivedmessage });
            msgQueMutex->unlock();
        //    //  clear clientbuffer
        //    receivedmessage[0] = '\0';
        }
        if (numbytes == SOCKET_ERROR) {
            std::cout << "Client " << client->name << ", " << client->IPaddress << " disconnected" << std::endl;
            client->connected = false;
            return;
        }
    }
}

void SocketCore::clientListener(SOCKET* socket) {
    const int buffersize = 4096;
    char receivebuffer[buffersize];
    int numbytes = 0;
    while (true) {
        numbytes = receiveMessage(receivebuffer, buffersize, *socket, 0);
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

void SocketCore::serverRetranslator(std::vector<ClientInfo*>* clients, std::queue<std::string>* messageQue, bool* stop, std::mutex* clientsMutex, std::mutex* msgQueMutex) {
    while (!(*stop)) {
        if (!messageQue->empty()) {
            std::string sender = messageQue->front().substr(0, 256);
            //  clear null terminators
            sender.erase(std::find(sender.begin(), sender.end(), '\0'), sender.end());
            std::string msg = messageQue->front().substr(256);

            std::vector<ClientInfo*>::iterator endloop = clients->end();
            for (std::vector<ClientInfo*>::iterator i = clients->begin(); i != endloop;) {
                if ((*i)->connected) {
                    sendMessage((sender + " : " + msg).c_str(), (*i)->socket, 0);
                    i++;
                }
                else {
                    clientsMutex->lock();
                    delete (*i);
                    i = clients->erase(i);
                    clientsMutex->unlock();
                    endloop = clients->end();
                }
            }
            msgQueMutex->lock();
            messageQue->pop();
            msgQueMutex->unlock();
        }
        else {
            // for some reason FIFO is empty. wait and don't waste CPU cycles
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}
