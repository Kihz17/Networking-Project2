#pragma once

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

#include "Client.h"
#include <PacketJoinRoom.h>
#include <PacketLeaveRoom.h>

class Server
{
public:
	Server(PCSTR authServerIp, PCSTR authPort, PCSTR port);
	~Server();

	bool Initialize();

	void Start();

	void SendToClient(Client* client, char* dataToSend, int dataLength);

	void BroadcastMessage(char* dataToSend, int dataLength);

	void BroadcastMessageExcludeClient(Client* exclude, char* dataToSend, int dataLength);

	void JoinRoom(Client* name, netutils::PacketJoinRoom& packet);

	void LeaveRoom(Client* name, netutils::PacketLeaveRoom& packet);

	void BroadcastToRoom(std::string roomName, char* dataToSend, int dataLength);

	void BroadcastToRoomExcludeClient(std::string roomName, Client* exclude, char* dataToSend, int dataLength);

	std::string FindClientRoom(Client* client);

	void SendToAuthServer(netutils::Buffer& buffer);

private:
	void ShutDown();

	bool ResolveAuthServer();

	PCSTR port;
	SOCKET connectionSocket; // This socket is used to listen for incoming connections
	SOCKET acceptSocket; // Will hold a new connection

	// Holds data to our auth server
	PCSTR authServerIp;
	PCSTR authServerPort;
	SOCKET authServerSocket;

	//create a map with rooms and clients ?
	std::map<std::string, std::vector<Client*>> rooms;
	std::map<Client*, std::string> clientToRoomMap; // Holds what room a client is in

	std::map<unsigned int, Client*> unauthenticatedClients; 
	std::vector<Client*> clients; // Holds our connected clients
};