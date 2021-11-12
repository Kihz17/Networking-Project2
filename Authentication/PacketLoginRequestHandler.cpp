#include "PacketLoginRequestHandler.h"

#include "DBUtils.h"
#include <PacketLoginRequest.h>
#include <PacketLoginResult.h>

void PacketLoginRequestHandler::Handle(AuthServer& server, AuthClient* sender)
{
	netutils::PacketLoginRequest loginPacket(sender->buffer);

	long userId;
	std::string creationDate;
	auth::AuthenticateWebResult_AuthenticateResult result = server.AuthenticateAccount(loginPacket.email, loginPacket.password, userId, creationDate);

	netutils::PacketLoginResult loginResult(loginPacket.requestId, userId, result, creationDate, loginPacket.email);
	netutils::Buffer buffer(DEFAULT_BUFLEN);
	loginResult.Serialize(buffer);
	sender->Send(buffer);
}