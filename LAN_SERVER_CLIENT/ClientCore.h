#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Ws2tcpip.h"     //extension on winsock
#include "WSAErrorList.h"
#include <iostream>
#include <vector>
#include <thread>
#include "ClientInfo.h"
#include <queue>
#include <mutex>
#include <memory>
//#include "openssl/ssl.h"
//#include "openssl/err.h"

#pragma comment (lib, "ws2_32")
//#pragma comment (lib, "crypt32")
//#pragma comment (lib, "static/libssl64MT")
//#pragma comment (lib, "static/libcrypto64MT")

#define MAX_MESSAGE_SIZE 4096
#define MAX_NAME_LENGTH 256

class ClientCore {
public:
	ClientCore(const char* IP, const char* port);
	~ClientCore();
	bool sendMessage(const char* msg, const int& flags);	//returns #bytes sent
	int receiveMessage(char* msg, const int& maxmsglen, const SOCKET& socket, const int& flags);	//returns 0 if remote connection closed
	void disconnect();
private:
	ADDRINFOA makeHints(const int& family, const int& socktype, const int& protocol, const int& flags);
	ADDRINFOA* makeAddrInfo(PCSTR nodename, PCSTR servicename, const ADDRINFOA& hints);
	void displayAddrinfo(ADDRINFOA* addrinfoStruct);
	SOCKET getSocketAndConnect(ADDRINFOA* addrinfoStruct);
	bool makeConnection(const SOCKET& socket, ADDRINFOA* addrinfo);
	void closeConnection(const SOCKET& socket);
	void* get_in_addr(sockaddr* sa);	/// get sockaddr, IPv4 or IPv6
	void listener();
private:
	WSAData wsaData;
	SOCKET mainSocket;
	std::thread listenerThr;
};