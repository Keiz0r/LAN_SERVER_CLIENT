#pragma once
#include "ClientCore.h"

class Client {
public:
	Client();
	~Client();
private:
	void Run();
public:
	std::string input = "";
private:
	std::string serverIP = "127.0.0.1";
	std::string serverPort = "36484";
//	std::vector<otherclientID, hisACTION> dataofothers;	// TODO : for any game listener should fill this
};