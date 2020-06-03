#pragma once
#include "Ws2tcpip.h"     //extension on winsock
#include <iostream>
#include <vector>
#include <thread>
#include "ClientInfo.h"
#include <queue>
#include <mutex>
#include <memory>

#pragma comment (lib, "ws2_32")

#define MAX_MESSAGE_SIZE 4096
#define MAX_NAME_LENGTH 256

class ServerCore {
public:
	ServerCore(const char* IP, const char* port, std::vector<std::shared_ptr<ClientInfo>>& clientinfoVec, std::queue<std::string>& msgQ);
	~ServerCore();
	bool sendMessage(const char* msg, const SOCKET& socket, const int& flags);	//returns #bytes sent
	int receiveMessage(char* msg, const int& maxmsglen, const SOCKET& socket, const int& flags);	//returns 0 if remote connection closed
	void startDispatcher();
	void startRetranslator();
	void stopServer();
private:
	ADDRINFOA* makeAddrInfo(PCSTR nodename, PCSTR servicename, const ADDRINFOA& hints);
	ADDRINFOA makeHints(const int& family, const int& socktype, const int& protocol, const int& flags);
	void displayAddrinfo(ADDRINFOA* addrinfoStruct);
	void* get_in_addr(sockaddr* sa);	/// get sockaddr, IPv4 or IPv6
	SOCKET getSocketAndBind(ADDRINFOA* addrinfoStruct);	//Binds to the first suitable addrinfo. could be listening on the wrong network cuz of it
	SOCKET acceptConnection(const SOCKET& socket, sockaddr& sockaddrstorage, int& storageSize);
	void closeConnection(const SOCKET& socket);
	void listenPort(const SOCKET& socket, const int& maxQueue);
	void connectionsListenerDispatcher();
	void serverListener(std::shared_ptr<ClientInfo> client);
	void serverRetranslator();
private:
	WSAData wsaData;
	SOCKET mainSocket;
	std::thread listenerThr;
	std::thread retranslatorThr;
	bool stoplistening = false;
	std::mutex clientsMutex;
	std::mutex msgQueMutex;
	std::vector<std::shared_ptr<ClientInfo>>& m_pClients;
	std::queue<std::string>& m_pMessageQueue;
};

//		//tests for manual insertion
//
//		//  “pton” stands for “presentation to network” Translates string into byte address in struct
//		if (InetPtonA(AF_INET, "10.12.110.57", &(sa.sin_addr)) <= 0) { // IPv4 
//		    std::cerr << "InetPton failed." << std::endl;
//		}
//		if (InetPtonA(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)) <= 0) { // IPv6
//		    std::cerr << "InetPton failed." << std::endl;
//		}
//		
//		// reverse:
//		// IPv4:
//		char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
//		inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
//		std::cout << "The IPv4 address is: " << ip4 << std::endl;
//		
//		// IPv6:
//		char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
//		inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
//		std::cout << "The IPv6 address is: " << ip6 << std::endl;

//   Internet address (a structure for historical reasons)
//  struct in_addr {
//      uint32_t s__addr; // that's a 32-bit int (4 bytes) in Network Byte Order
//  };
//  
//  struct in6_addr {
//      unsigned char   s6__addr[16];   // IPv6 address
//  };

//  struct sockaddr {
//      unsigned short    sa_family;    // address family, AF_xxx   AF_INET (IPv4) or AF_INET6 (IPv6)
//      char              sa_data[14];  // 14 bytes of protocol address
//  };
//  
//  struct sockaddr_in {
//      short int          sin_family;  // Address family, AF_INET
//      unsigned short int sin_port;    // Port number  must be in Network Byte Order (by using htons()!)
//      in_addr            sin_addr;    // Internet address
//      unsigned char      sin_zero[8]; // Same size as struct sockaddr included to pad the structure to the length of a struct sockaddr should be set to all zeros with the function memset()
//  };
//  
//  struct sockaddr_in6 {
//      uint16_t       sin6_family;   // address family, AF_INET6
//      uint16_t       sin6_port;     // port number, Network Byte Order
//      uint32_t       sin6_flowinfo; // IPv6 flow information
//      in6_addr       sin6_addr;     // IPv6 address
//      uint32_t       sin6_scope_id; // Scope ID
//  };
//  
//  
//  struct addrinfo {
//      int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
//      int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
//      int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
//      int              ai_protocol;  // use 0 for "any"
//      size_t           ai_addrlen;   // size of ai_addr in bytes
//      sockaddr*        ai_addr;      // struct sockaddr_in or _in6
//      char*            ai_canonname; // full canonical hostname
//  
//      struct addrinfo* ai_next;      // linked list, next node
//  };
//  
//  struct sockaddr_storage {
//      uint16_t  ss_family;     // address family
//  
//      // all this is padding, implementation specific, ignore it:
//      char      __ss_pad1[_SS_PAD1SIZE];
//      int64_t   __ss_align;
//      char      __ss_pad2[_SS_PAD2SIZE];
//  };