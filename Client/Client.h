#pragma once

#include "Buffer.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

class Client
{
public:
	Client(PCSTR serverIp, PCSTR port);
	~Client();

	bool Initialize();

	void Start();

	bool SendToServer(char* data, int dataLength);

	netutils::Buffer buffer;

	bool authenticated;
	bool waitingForServerResponse;
private:
	void ShutDown();

	void Login(const std::string& email, const std::string& password);
	void Register(const std::string& email, const std::string& password);

	void HandleKeyboardInput(bool& sendEnterMessage, std::vector<char>& message, bool& roomChange);

	void AskToLogin();

	bool running;

	std::string currentRoom;
	std::string name;

	PCSTR serverIp;
	PCSTR serverPort;
	SOCKET serverSocket;
};