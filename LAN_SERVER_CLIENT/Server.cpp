#include "Server.h"

Server::Server()
    :
    mainSocket(INVALID_SOCKET)
{
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }
    Run();
}

Server::~Server() {
    WSACleanup();
}

void Server::Run() {
    ADDRINFOA serverHints = SocketCore::makeHints(AF_UNSPEC, SOCK_STREAM, NULL, AI_PASSIVE);
    ADDRINFOA* serverTypeAddrinfo = SocketCore::makeAddrInfo(IPaddress, Port, serverHints);
    mainSocket = SocketCore::getSocketAndBind(serverTypeAddrinfo);   //  IPv6 server blocked here
    SocketCore::displayAddrinfo(serverTypeAddrinfo);
    FreeAddrInfoA(serverTypeAddrinfo);


    SocketCore::listenPort(mainSocket, 10);

    // thread fills the ClientInfo form and dispatches conenctions to separate threads
    std::thread listenerThr(SocketCore::connectionsListenerDispatcher, &clients, &mainSocket, &clientThreads, &messageQueue, &stoplistening, &msgQueMutex);
    //  if FIFO is empty, retranslator polling at 1/50ms rate
    std::thread retranslator(SocketCore::serverRetranslator, &clients, &messageQueue, &stoplistening, &msgQueMutex);

    // server input loop
    while (1) {
        std::cout << ">> ";
        std::getline(std::cin >> std::ws, input);
        if (input == "STOPSERVER") {
            stoplistening = true;
            SocketCore::closeConnection(mainSocket);
            break;
        }
        for (std::vector<ClientInfo*>::iterator i = clients.begin(); i != clients.end(); i++) {
            SocketCore::sendMessage(("SERVER MESSAGE : " + input).c_str(), (*i)->socket, 0);
        }
    }

    // kill all clients on exit
    for (std::vector<ClientInfo*>::iterator i = clients.begin(); i != clients.end(); i++) {
        SocketCore::sendMessage("You were disconnected from the server. Server shutdown", (*i)->socket, 0);
        SocketCore::closeConnection((*i)->socket);
        delete* i;
    }
    // check if all threads are killed on exit
    for (std::vector<std::thread>::iterator i = clientThreads.begin(); i != clientThreads.end(); i++) {
        i->join();
    }
    listenerThr.join();
    retranslator.join();
}
