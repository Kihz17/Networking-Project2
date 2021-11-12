#include "PacketRegisterRequestHandler.h"

#include "DBUtils.h"
#include <PacketRegisterRequest.h>
#include <PacketRegisterResult.h>

void PacketRegisterRequestHandler::Handle(AuthServer& server, AuthClient* sender)
{
	netutils::PacketRegisterRequest registerPacket(sender->buffer);

	long userId; 
	auth::CreateAccountWebResult_CreateAccountResult result = server.CreateAccount(registerPacket.email, registerPacket.password, userId);

	netutils::PacketRegisterResult registerResult(registerPacket.requestId, userId, result);
	netutils::Buffer buffer(DEFAULT_BUFLEN);
	registerResult.Serialize(buffer);
	sender->Send(buffer);
}