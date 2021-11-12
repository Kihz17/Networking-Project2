#include "PacketRegisterResultHandler.h"

#include <PacketRegisterResult.h>

#include <iostream>
#include <string>

void PacketRegisterResultHandler::HandleOnClient(Client& client, const SOCKET& serverSocket)
{
	netutils::PacketRegisterResult registerResult(client.buffer);

	if (registerResult.result == auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_SUCCESS)
	{
		std::cout << "Account created successfully!" << std::endl;
		client.waitingForServerResponse = false;
	}
	else if (registerResult.result == auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INVALID_PASSWORD)
	{
		std::cout << "Invalid password. Please ensure that your password is atleast 8 characters long!" << std::endl;
		client.waitingForServerResponse = false;
	}
	else if (registerResult.result == auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR)
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