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
    std::thread listenerThr(SocketCore::connectionsListenerDispatcher, &clients, &mainSocket, &clientThreads, &messageQueue, &stoplistening, &clientsMutex, &msgQueMutex);
    //  if FIFO is empty, retranslator polling at 1/50ms rate
    std::thread retranslator(SocketCore::serverRetranslator, &clients, &messageQueue, &stoplistening, &clientsMutex, &msgQueMutex);

    // server input loop
    while (1) {
        std::cout << ">> ";
        std::getline(std::cin >> std::ws, input);
        if (input == "STOPSERVER") {
            stoplistening = true;
            SocketCore::closeConnection(mainSocket);
            //make a server shutdown procedure, wit several announcements like wow ingame
            break;
        }
        for (std::vector<ClientInfo*>::iterator i = clients.begin(); i != clients.end(); i++) {
            SocketCore::sendMessage(("SERVER MESSAGE : " + input).c_str(), (*i)->socket, 0);
        }
    }

    //then add mutex for that. also make socketcore an entity, and pass all shit by reference, so threads need not arguments
    // kill all clients on exit
    std::vector<ClientInfo*>::iterator loopend = clients.end();
    clientsMutex.lock();
    for (std::vector<ClientInfo*>::iterator i = clients.begin(); i != loopend; i++) {
        SocketCore::sendMessage("You were disconnected from the server. Server shutdown", (*i)->socket, 0);
        SocketCore::closeConnection((*i)->socket);
    }
    clientsMutex.unlock();

    messageQueue.push("APOCALYPSE");
    // check if all threads are killed on exit
    for (std::vector<std::thread>::iterator i = clientThreads.begin(); i != clientThreads.end(); i++) {
        i->join();
    }
    listenerThr.join();
    retranslator.join();
}
