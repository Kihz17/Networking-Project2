#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

#include "AuthClient.h"
#include "DBUtils.h"

class AuthServer
{
public:
	AuthServer(PCSTR port);
	~AuthServer();

	bool Initialize();

	void Start();

	auth::AuthenticateWebResult_AuthenticateResult AuthenticateAccount(const std::string& email, const std::string& plainTextPassword, long& userId, std::string& creationDate);

	auth::CreateAccountWebResult_CreateAccountResult CreateAccount(const std::string& email, const std::string& plainTextPassword);

private:
	void ShutDown();

	PCSTR port;
	SOCKET connectionSocket; // This socket is used to listen for incoming connections
	SOCKET acceptSocket; // Will hold a new connection

	DBUtils database;

	std::vector<AuthClient*> clients; // Holds our connected clients
};