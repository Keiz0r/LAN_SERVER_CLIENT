#include "Client.h"

Client::Client()
    :
    mainSocket(INVALID_SOCKET)
{
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }
    Run();
}

Client::~Client() {
    WSACleanup();
}

void Client::Run() {
    ADDRINFOA clientHints = SocketCore::makeHints(AF_INET, SOCK_STREAM, 0, AI_CANONNAME);
    ADDRINFOA* clientTypeAddrinfo = SocketCore::makeAddrInfo("132.66.210.206", "36484", clientHints);
    SocketCore::displayAddrinfo(clientTypeAddrinfo);

    mainSocket = SocketCore::getSocketAndConnect(clientTypeAddrinfo);
    if (mainSocket != INVALID_SOCKET) {
        
        InetNtopA(clientTypeAddrinfo->ai_family, SocketCore::get_in_addr(reinterpret_cast<sockaddr*>(clientTypeAddrinfo->ai_addr)), serverIP, INET6_ADDRSTRLEN);
        std::cout << "Connected to :" << serverIP << "!" << std::endl;
        FreeAddrInfoA(clientTypeAddrinfo);

        std::thread listenerThr(SocketCore::clientListener, &mainSocket);

        //client input loop
        while (true) {
            std::cout << ">> ";
            std::getline(std::cin >> std::ws, input);
            if (input == "DISCONNECT") {
                SocketCore::closeConnection(mainSocket);
                break;
            }
            SocketCore::sendMessage(input.c_str(), mainSocket, 0);
        }
        listenerThr.join();
    }
    else {
        std::cout << "connection failed!" << std::endl;
    }
}
