#pragma once
#include "SocketCore.h"

class Client {
public:
	Client();
	~Client();
private:
	void Run();
public:
	std::string input = "";
private:
	WSAData wsaData;
	SOCKET mainSocket;
	char serverIP[INET6_ADDRSTRLEN];
//	std::vector<otherclientID, hisACTION> dataofothers;	// TODO : for any game listener should fill this
};