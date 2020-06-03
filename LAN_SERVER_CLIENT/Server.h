#pragma once
#include "ServerCore.h"

class Server {
public:
	Server();
	~Server();
private:
	void Run();
public:
	std::string input ="";
private:
	std::string IPaddress{"localhost"};    // or in format "255.255.255.255"
	std::string Port{"36484"};
	std::vector<std::shared_ptr<ClientInfo>> clients;	//probably make it a linked list
	std::queue<std::string> messageQueue;
};