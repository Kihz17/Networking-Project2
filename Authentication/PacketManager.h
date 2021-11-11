#pragma once

#include "IPacketHandler.h"

#include <map>

class PacketManager
{
public:
	static PacketManager* GetInstance();

	void HandlePacket(AuthServer& server, AuthClient* client, unsigned int packetType);

	void CleanUp();
private:
	static PacketManager* instance;
	std::map<unsigned int, IPacketHandler*> handlerMap;
};