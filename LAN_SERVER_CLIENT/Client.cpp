#include "Client.h"

Client::Client() {
    Run();
}

Client::~Client() {
}

void Client::Run() {
    ClientCore clnt(serverIP.c_str(), serverPort.c_str());
    clnt.startListener();

    //client input loop
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin >> std::ws, input);
        if (input == "DISCONNECT") {
            clnt.disconnect();
            break;
        }
        clnt.sendMessage(input.c_str(), 0);
//      std::string request = "GET / HTTP/1.1\n";
//      request += "Host: proplay.ru\n";
//        request += "Connection: keep-alive\n";
//        request += "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\n";
//        request += "Accept-Language: fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3\n";
//        request += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n";
//        request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\n\
//            Accept: text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, */*;q=0.8\n\
//            Accept-Language: en-US,en;q=0.5\n\
//            Accept-Encoding: gzip, deflate, br\n\
//            Upgrade-Insecure-Requests: 1\n\
//            Connection: keep-alive\n";
//      request += "\n";
//      clnt.sendMessage(request.c_str(), 0);
    }
}
