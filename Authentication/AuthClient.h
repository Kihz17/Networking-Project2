#pragma once

#include <string>
#include <winsock2.h>

#include "Buffer.h"

#define DEFAULT_BUFLEN 512	

class AuthClient
{
public:
	AuthClient() : socket(NULL), buffer(DEFAULT_BUFLEN) {}

	void Send(netutils::Buffer& buffer)
	{
		int result = send(this->socket, buffer.data, buffer.Length(), 0);
		if (result == SOCKET_ERROR)
		{
			printf("Failed to send data to Auth Client!\n");
		}
	}

	void Send(char* data, int length)
	{
		int result = send(this->socket, data, length, 0);
		if (result == SOCKET_ERROR)
		{
			printf("Failed to send data to Auth Client!\n");
		}
	}

	SOCKET socket;
	netutils::Buffer buffer;
};