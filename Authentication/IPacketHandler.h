#pragma once

#include "Buffer.h"
#include "AuthServer.h"

class IPacketHandler
{
public:
	virtual void Handle(AuthServer& server, AuthClient* sender) = 0;
};

