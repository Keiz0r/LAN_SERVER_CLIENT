#pragma once
#include "Ws2tcpip.h"
#include <string>
#include <thread>

class ClientInfo {
public:
	SOCKET socket = INVALID_SOCKET;
	std::string name = "";
	std::string IPaddress = "";
	std::string port = "";
	bool connected = false;
	std::thread::id threadID{};
};