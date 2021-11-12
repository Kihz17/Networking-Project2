#pragma once

#include "IPacketHandler.h"

class PacketRegisterRequestHandler : public IPacketHandler
{
public:
	PacketRegisterRequestHandler() = default;

	virtual void Handle(AuthServer& server, AuthClient* sender) override;
};