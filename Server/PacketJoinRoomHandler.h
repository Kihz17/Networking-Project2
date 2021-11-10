#pragma once
#include "PacketHandler.h"


class PacketJoinRoomHandler : public IPackethandler
{
public:
	virtual void HandleOnServer(Server& server, Client* sender);

private:
	friend class PacketManager;
	PacketJoinRoomHandler();
};


