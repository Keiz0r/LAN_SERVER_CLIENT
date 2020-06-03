#include "ServerCore.h"

ServerCore::ServerCore(const char* IP, const char* port, std::vector<std::shared_ptr<ClientInfo>>& clientinfoVec, std::queue<std::string>& msgQ)
    :
    mainSocket(INVALID_SOCKET),
    m_pClients(clientinfoVec),
    m_pMessageQueue(msgQ)
{
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }

    ADDRINFOA serverHints = makeHints(AF_UNSPEC, SOCK_STREAM, NULL, AI_PASSIVE);
    ADDRINFOA* serverAddrinfo = makeAddrInfo(IP, port, serverHints);
    mainSocket = getSocketAndBind(serverAddrinfo);   //  IPv6 server blocked here
    displayAddrinfo(serverAddrinfo);
    FreeAddrInfoA(serverAddrinfo);

    listenPort(mainSocket, 10);

    // thread fills the ClientInfo form and dispatches conenctions to separate threads
    listenerThr = std::thread([this]() {this->connectionsListenerDispatcher(); });
    //  if FIFO is empty, retranslator polling at 1/50ms rate
    retranslatorThr = std::thread([this]() {this->serverRetranslator(); });
}

ServerCore::~ServerCore() {
    if (listenerThr.joinable()) {
        listenerThr.join();
    }
    if (retranslatorThr.joinable()) {
        retranslatorThr.join();
    }
    WSACleanup();
}

ADDRINFOA* ServerCore::makeAddrInfo(PCSTR nodename, PCSTR servicename, const ADDRINFOA& hints) {
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

ADDRINFOA ServerCore::makeHints(const int& family, const int& socktype, const int& protocol, const int& flags) {
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

void ServerCore::displayAddrinfo(ADDRINFOA* addrinfoStruct) {
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

SOCKET ServerCore::getSocketAndBind(ADDRINFOA* addrinfoStruct) {
    ADDRINFOA* p;
    SOCKET s = INVALID_SOCKET;
    // loop through all the results and connect to the first we can
    for (p = addrinfoStruct; p != NULL; p = p->ai_next) {

        //  blocking of IPv6 server. Comment out if network supports IPv6
        if (p->ai_family != AF_INET) {
            continue;
        }

        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            printWSAError();
            continue;
        }

        if (bind(s, p->ai_addr, p->ai_addrlen) != 0) {
            printWSAError();
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

void ServerCore::closeConnection(const SOCKET& socket) {
    if (int errmsg = closesocket(socket) != 0) {
        printWSAError();
        return;
    }
    std::cout << "connection closed!" << std::endl;
}

void ServerCore::listenPort(const SOCKET& socket, const int& maxQueue) {
    if(int errmsg = listen(socket, maxQueue) != 0) {
        printWSAError();
        return;
    }
    std::cout << "Server is listening for connections....." << std::endl;
}

SOCKET ServerCore::acceptConnection(const SOCKET& socket, sockaddr& sockaddrstorage, int& storageSize) {
    int result = accept(socket, &sockaddrstorage, &storageSize);
    return result;
    //  error checking done outside
}

bool ServerCore::sendMessage(const char* msg, const SOCKET& socket, const int& flags) {
    int result = 0;
    int len = strlen(msg);
    int totalSent = 0;
    while (totalSent < len) {
        result = send(socket, msg + totalSent, len-totalSent, flags);
        if (totalSent == -1) {
            printWSAError();
            return false;
        }
        totalSent += result;
    }
    return true;
}

int ServerCore::receiveMessage(char* msg, const int& maxmsglen, const SOCKET& socket, const int& flags) {
    int result = 0;
    result = recv(socket, msg, maxmsglen - 1, flags);
    if (result == -1) {
        printWSAError();
    }
    return result;
}

void* ServerCore::get_in_addr(sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(reinterpret_cast<sockaddr_in*>(sa)->sin_addr);
    }

    return &(reinterpret_cast<sockaddr_in6*>(sa)->sin6_addr);
}

void ServerCore::connectionsListenerDispatcher() {
    while (!stoplistening) {
        sockaddr remote_addr;
        int remoteaddr_size = sizeof(remote_addr);

        SOCKET dedicatedSocket = acceptConnection(mainSocket, remote_addr, remoteaddr_size);
        if (dedicatedSocket == -1 && !(stoplistening)) {
            printWSAError();
            continue;
        }
        if (stoplistening) {
            break;
        }
        char remoteIP[INET6_ADDRSTRLEN];
        char remotename[MAX_NAME_LENGTH];
        char remoteport[20];
        std::shared_ptr<ClientInfo> client = std::make_shared<ClientInfo>();
        getnameinfo(&remote_addr, remoteaddr_size, remotename, MAX_NAME_LENGTH, remoteport, 20, 0);
        InetNtopA(remote_addr.sa_family, get_in_addr(&remote_addr), remoteIP, INET6_ADDRSTRLEN);
        std::cout << "got a conenction from : " << remotename << " IP : " << remoteIP << std::endl;
        client->socket = dedicatedSocket;
        client->IPaddress = remoteIP;
        client->name = remotename;
        client->port = remoteport;

        clientsMutex.lock();
        m_pClients.push_back(client);
        clientsMutex.unlock();
        std::thread newListener([&](){this->serverListener(client);});
        newListener.detach();
    }
}

void ServerCore::serverListener(std::shared_ptr<ClientInfo> client) {
    client->threadID = std::this_thread::get_id();
    client->connected = true;
    int numbytes = 0;
    while (true) {
        char receivedmessage[MAX_MESSAGE_SIZE];
        numbytes = receiveMessage(receivedmessage, MAX_MESSAGE_SIZE, client->socket, 0);
        if (numbytes > 0) {
            //  insert null-terminator
            receivedmessage[numbytes] = '\0';
            std::cout << client->name << " :\n" << receivedmessage << "\n" << std::endl;
            //  fill FIFO with name(max 256 chars) and message
            auto tempNameStr = client->name;
            tempNameStr.resize(MAX_NAME_LENGTH);
            msgQueMutex.lock();
            m_pMessageQueue.push(tempNameStr + std::string{ receivedmessage });
            msgQueMutex.unlock();
        //    //  clear clientbuffer
        //    receivedmessage[0] = '\0';
        }
        if (numbytes == SOCKET_ERROR) {
            std::cout << "Client " << client->name << ", " << client->IPaddress << " disconnected" << std::endl;
            client->connected = false;
            closeConnection(client->socket);
            break;
        }
    }
}

void ServerCore::serverRetranslator() {
    while (!stoplistening) {
        if (!m_pMessageQueue.empty()) {
            std::string sender = m_pMessageQueue.front().substr(0, 256);
            //  clear null terminators
            sender.erase(std::find(sender.begin(), sender.end(), '\0'), sender.end());
            std::string msg = m_pMessageQueue.front().substr(256);

            std::vector<std::shared_ptr<ClientInfo>>::iterator endloop = m_pClients.end();
            for (std::vector<std::shared_ptr<ClientInfo>>::iterator i = m_pClients.begin(); i != endloop;) {
                if ((*i)->connected) {
                    sendMessage((sender + " : \n" + msg).c_str(), (*i)->socket, 0);
                    i++;
                }
                else {  
                    // cleanup of dead ClientInfo
                    clientsMutex.lock();
                    i = m_pClients.erase(i);
                    endloop = m_pClients.end();
                    clientsMutex.unlock();
                }
            }
            msgQueMutex.lock();
            m_pMessageQueue.pop();
            msgQueMutex.unlock();
        }
        else {
            // for some reason FIFO is empty. wait and don't waste CPU cycles
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void ServerCore::stopServer() {
    msgQueMutex.lock();
    m_pMessageQueue.push("SERVER MESSAGE : Server shutdown in 1:00");
    msgQueMutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    msgQueMutex.lock();
    m_pMessageQueue.push("SERVER MESSAGE : Server shutdown in 0:30");
    msgQueMutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    msgQueMutex.lock();
    m_pMessageQueue.push("SERVER MESSAGE : Server shutdown in 0:10");
    msgQueMutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    msgQueMutex.lock();
    m_pMessageQueue.push("SERVER MESSAGE : Server shutdown in 0:05");
    msgQueMutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    msgQueMutex.lock();
    m_pMessageQueue.push("SERVER MESSAGE : Server shutdown in 0:01");
    msgQueMutex.unlock();

    stoplistening = true;
    closeConnection(mainSocket);

    // kill all clients on exit
    std::vector<std::shared_ptr<ClientInfo>>::iterator loopend = m_pClients.end();
    clientsMutex.lock();
    for (std::vector<std::shared_ptr<ClientInfo>>::iterator i = m_pClients.begin(); i != loopend; i++) {
        (*i)->connected = false;
    }
    clientsMutex.unlock();
}
