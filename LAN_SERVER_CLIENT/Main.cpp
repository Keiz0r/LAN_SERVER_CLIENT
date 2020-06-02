#define UNICODE
#include <iostream>
#include <string>
#include "Server.h"
#include "Client.h"

int main() {

    std::string input;
    std::cout << "client/server?\n>> ";
    std::getline(std::cin, input);
    if (input == "client") {
        Client client;
    }
    else if (input == "server") {
        Server server;
    }
    return 0;
}