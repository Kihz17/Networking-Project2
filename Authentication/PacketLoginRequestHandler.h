#pragma once

#include "IPacketHandler.h"

class PacketLoginRequestHandler : public IPacketHandler
{
public:
	PacketLoginRequestHandler() = default;

	virtual void Handle(AuthServer& server, AuthClient* sender) override;
};