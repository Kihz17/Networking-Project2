#include "PacketLoginResultHandler.h"

#include <PacketLoginResult.h>

#include <iostream>
#include <string>

void PacketLoginResultHandler::HandleOnClient(Client& client, const SOCKET& serverSocket)
{
	netutils::PacketLoginResult loginResult(client.buffer);

	if (loginResult.result == auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_SUCCESS)
	{
		std::cout << "Authenticated successfully!" << std::endl;
		std::cout << "Your account was created on " << loginResult.creationDate << "." << std::endl;
		client.authenticated = true; // We have authenticated successfully!
		client.waitingForServerResponse = false;
	}
	else if (loginResult.result == auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INVALID_CREDENTIALS)
	{
		std::cout << "Invalid Credentials, please try again." << std::endl;
		client.waitingForServerResponse = false;
	}
	else if (loginResult.result == auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INTERNAL_SERVER_ERROR)
	{
		std::cout << "Internal server error, please try again." << std::endl;
		client.waitingForServerResponse = false;
	}
	else
	{
		std::cout << "Unknown login response." << std::endl;
		client.waitingForServerResponse = false;
	}
}