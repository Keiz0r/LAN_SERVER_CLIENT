#include "Server.h"

Server::Server() {
    Run();
}

Server::~Server() {
}

void Server::Run() {
    ServerCore srv(IPaddress.c_str(), Port.c_str(), clients, messageQueue);
    srv.startDispatcher();
    srv.startRetranslator();

    // server input loop
    while (1) {
        std::cout << ">> ";
        std::getline(std::cin >> std::ws, input);
        if (input == "STOPSERVER") {
            srv.stopServer();
            break;
        }
        for (std::vector<std::shared_ptr<ClientInfo>>::iterator i = clients.begin(); i != clients.end(); i++) {
            srv.sendMessage(("SERVER MESSAGE : " + input).c_str(), (*i)->socket, 0);
        }
    }

}
