#include "Server.h"

Server::Server() {
    Run();
}

Server::~Server() {
}

void Server::Run() {
    ServerCore srv(IPaddress.c_str(), Port.c_str(), clients, messageQueue);

    // server input loop
    while (1) {
        std::cout << ">> ";
        std::cin >> input;
//        std::getline(std::cin >> std::ws, input);
        if (input == "STOPSERVER") {
            std::cout << "Are you sure you want to stop server? (y/n)";
            std::cin >> input;
//            std::getline(std::cin >> std::ws, input);
            if (input == "y") {
                srv.stopServer();
                break;
            }
            continue;
        }
        for (std::vector<std::shared_ptr<ClientInfo>>::iterator i = clients.begin(); i != clients.end(); i++) {
            srv.sendMessage(("SERVER MESSAGE : " + input).c_str(), (*i)->socket, 0);
        }
    }

}
