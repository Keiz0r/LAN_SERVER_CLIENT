#pragma once
#include "SocketCore.h"
#include "ClientInfo.h"
#include <mutex>

class Server {
public:
	Server();
	~Server();
private:
	void Run();
public:
	std::string input ="";
	
private:
	WSAData wsaData;
	const char* IPaddress = NULL;    // or in format "255.255.255.255"
	const char* Port = "36484";
	SOCKET mainSocket;
	bool stoplistening = false;
	std::vector<ClientInfo*> clients;
	std::vector<std::thread> clientThreads;
	std::queue<std::string> messageQueue;
	std::mutex msgQueMutex;
};