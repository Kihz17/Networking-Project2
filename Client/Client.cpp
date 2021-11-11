#include "Client.h"
#include "PacketManager.h"

#include <PacketJoinRoom.h>
#include <PacketLeaveRoom.h>
#include <PacketSendMessage.h>
#include <PacketRegisterRequest.h>
#include <PacketLoginRequest.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512 // Default buffer length of our buffer in characters

Client::Client(PCSTR serverIp, PCSTR port)
	: buffer(DEFAULT_BUFLEN), authenticated(false), currentRoom(""), waitingForServerResponse(false)
{
	this->serverIp = serverIp;
	this->serverPort = port;
	this->serverSocket = INVALID_SOCKET;
	this->running = false;
}

Client::~Client()
{

}

bool Client::Initialize()
{
	std::cout << "Initializing client..." << std::endl;
	struct addrinfo* infoResult = NULL; // Holds the address information of our server
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	int result;								

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve server
	result = getaddrinfo(this->serverIp, this->serverPort, &hints, &infoResult);
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		WSACleanup();
		return 1;
	}

	// Try to connect to an address
	for (ptr = infoResult; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		this->serverSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (this->serverSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		result = connect(this->serverSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (result == SOCKET_ERROR)
		{
			closesocket(this->serverSocket);
			this->serverSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(infoResult); // Done connecting, free from memory

	if (this->serverSocket == INVALID_SOCKET)
	{
		printf("Failed to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Force our connection socket to be non-blocking
	DWORD nonBlock = 1;
	result = ioctlsocket(this->serverSocket, FIONBIO, &nonBlock);
	if (result == SOCKET_ERROR)
	{
		printf("Failed to make connection socket non-blocking! %d\n", WSAGetLastError());
		closesocket(this->serverSocket);
		WSACleanup();
		return false;
	}

	printf("Connection successful!\n");
	return true;
}

void Client::Start()
{
	int total;
	int result;
	DWORD flags;
	DWORD bytesReceived;
	FD_SET readSet;

	this->running = true;

	std::vector<char> message;
	bool sendInputMessage = true;
	bool roomChange = false;
	while (this->running)
	{
		// Only handle keyboard input if we have authenticated
		if (this->authenticated)
		{
			this->HandleKeyboardInput(sendInputMessage, message, roomChange);	// Handle non-blocking keyboard input
		}
		else if(!this->waitingForServerResponse) // We aren't authenticated yet, are we waiting for a response? If not then ask the user for an action
		{
			AskToLogin();
		}

		// READ INCOMING DATA
		FD_ZERO(&readSet); // Wipe out our read set
		FD_SET(this->serverSocket, &readSet); // Add connection socket to the read set (AKA: Keep listening for connections)

		timeval timeoutValue = { 0 };

		// Find the sockets that need updating
		total = select(0, &readSet, NULL, NULL, &timeoutValue);
		if (total == SOCKET_ERROR) {
			printf("select() has failed! %d\n", WSAGetLastError());
			this->running = false;
			break;
		}

		// Handle incoming connections
		if (FD_ISSET(this->serverSocket, &readSet))
		{
			total--;
			DWORD flags = 0;

			bytesReceived = recv(this->serverSocket, this->buffer.data, this->buffer.Length(), flags); // Recieve the data (THIS IS BLOCKING, which is we we only call it when the socket has new data)
			if (bytesReceived == SOCKET_ERROR)
			{
				printf("recv() has failed!");

				if (WSAGetLastError() == 10054)
				{
					printf("Disconnected from server!\n");
					this->running = false;
					break;
				}

			}
			else if (bytesReceived == 0)
			{
				printf("Disconnected from server!\n");
				this->running = false;
				break;
			}

			int packetHeader = buffer.ReadInt();
			PacketManager::GetInstance()->HandlePacket(*this, this->serverSocket, packetHeader);
			this->buffer.Clear();
		}
	}

	this->ShutDown();
}

void Client::ShutDown()
{
	std::cout << "Client shutting down..." << std::endl;

	int result;
	// Shut down the connection 
	result = shutdown(this->serverSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(this->serverSocket);
		return;
	}

	// Cleanup
	closesocket(this->serverSocket);
}

bool Client::SendToServer(char* data, int dataLength)
{
	int result;
	result = send(this->serverSocket, data, dataLength, 0);
	if (result == SOCKET_ERROR)
	{
		printf("Failed to send message: %d\n", WSAGetLastError());
		closesocket(this->serverSocket);
		return false;
	}

	return true;
}

void Client::HandleKeyboardInput(bool& sendEnterMessage, std::vector<char>& message, bool& roomChange)
{
	if (sendEnterMessage)
	{
		if (this->currentRoom == "") // We havent joined a room yet
		{
			std::cout << "Enter the room you'd like to join: " << std::endl;
			std::cout << std::endl;

		}
		else
		{
			std::cout << "Press 'esc' to Change Room OR Enter Message: " << std::endl;
			std::cout << std::endl;
		}

		sendEnterMessage = false;
	}

	if (_kbhit())
	{
		char key = _getch();
		if (key == 8 && !message.empty()) // Backspace
		{
			message.pop_back();
			std::string msg(message.begin(), message.end());
			std::cout << "\r" << msg;
		}
		else if (key == 13) // Enter
		{
			std::string msg(message.begin(), message.end());
			message.clear();

			if (this->currentRoom == "") // We aren't in a room yet, consider this message as our room argument
			{
				netutils::PacketJoinRoom packet(msg, this->name);
				netutils::Buffer buffer(packet.GetSize());
				packet.Serialize(buffer);

				this->running = this->SendToServer(buffer.data, buffer.Length());

				this->currentRoom = packet.roomName;
				std::cout << std::endl;
				std::cout << std::endl;
				sendEnterMessage = true;

			}
			else if (roomChange)
			{
				roomChange = false;

				if (msg == "leave")
				{
					netutils::PacketLeaveRoom packet(this->currentRoom, this->name);
					netutils::Buffer buffer(packet.GetSize());
					packet.Serialize(buffer);

					this->running = this->SendToServer(buffer.data, buffer.Length());

					std::cout << std::endl;
					std::cout << std::endl;
					this->running = false; // Stop client
				}
				else
				{
					netutils::PacketJoinRoom packet(msg, name);
					netutils::Buffer buffer(packet.GetSize());
					packet.Serialize(buffer);

					this->running = this->SendToServer(buffer.data, buffer.Length());

					this->currentRoom = packet.roomName;
					std::cout << std::endl;
					std::cout << std::endl;
					sendEnterMessage = true;
				}
			}
			else
			{
				netutils::PacketSendMessage packet(msg);
				netutils::Buffer buffer(packet.GetSize());
				packet.Serialize(buffer);

				this->running = this->SendToServer(buffer.data, buffer.Length());

				std::cout << std::endl;
				std::cout << std::endl;
				std::cout << "[" << name << "]: " << msg << std::endl;

				sendEnterMessage = true;
			}
		}
		else if (key == 27) // escape key
		{
			message.clear();
			std::cout << "Enter Room Name:" << std::endl;
			roomChange = true;
		}
		else
		{
			message.push_back(key);
			std::string msg(message.begin(), message.end());
			std::cout << "\r" << msg;
		}
	}
}

void Client::AskToLogin()
{
	std::string action;
	while (action != "1" && action != "2") // Keep asking until we get a valid response
	{
		std::cout << "What would you like to do?:" << std::endl;
		std::cout << "1) Login" << std::endl;
		std::cout << "2) Sign Up\n" << std::endl;
		std::getline(std::cin, action); // This can be blocking because at this point we have nothing to read from the server
	}

	std::string email;
	std::cout << "Enter your email: ";
	std::getline(std::cin, email);

	std::string password;
	std::cout << "Enter your password: ";
	std::getline(std::cin, password);

	if (action == "1")
	{
		Login(email, password);
	}
	else if (action == "2")
	{
		Register(email, password);
	}
}

void Client::Login(const std::string& email, const std::string& password)
{
	netutils::PacketLoginRequest loginPacket(0, email, password);
	loginPacket.Serialize(this->buffer);
	this->SendToServer(this->buffer.data, this->buffer.Length());
	this->buffer.Clear();

	std::cout << "Waiting for authentication response..." << std::endl;
	this->waitingForServerResponse = true;
}

void Client::Register(const std::string& email, const std::string& password)
{
	netutils::PacketRegisterRequest registerPacket(0, email, password);
	registerPacket.Serialize(this->buffer);
	this->SendToServer(this->buffer.data, this->buffer.Length());
	this->buffer.Clear();

	std::cout << "Waiting for registration response..." << std::endl;
	this->waitingForServerResponse = true;
}