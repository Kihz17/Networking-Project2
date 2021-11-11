#include "PacketManager.h"
#include "PacketLoginRequestHandler.h"

#include <iostream>

PacketManager* PacketManager::instance = NULL;

PacketManager* PacketManager::GetInstance()
{
	if (PacketManager::instance == NULL)
	{
		PacketManager::instance = new PacketManager();
		instance->handlerMap.insert(std::make_pair(4, new PacketLoginRequestHandler()));
		/*	instance->handlerMap.insert(std::make_pair(5, new PacketRegisterRequestHandler()));
		instance->handlerMap.insert(std::make_pair(6, new PacketLoginResultHandler()));
		instance->handlerMap.insert(std::make_pair(7, new PacketRegisterResultHandler()));*/
	}

	return instance;
}

void PacketManager::HandlePacket(AuthServer& server, AuthClient* client, unsigned int packetType)
{
	std::map<unsigned int, IPacketHandler*>::iterator it = this->handlerMap.find(packetType);
	if (it != this->handlerMap.end())
	{
		it->second->Handle(server, client);
	}
	else
	{
		std::cout << "Packet handler for type " << packetType << " was not found!" << std::endl;
	}
}

void PacketManager::CleanUp()
{
	std::map<unsigned int, IPacketHandler*>::iterator it;
	for (it = this->handlerMap.begin(); it != this->handlerMap.end(); it++)
	{
		delete it->second;
	}

	this->handlerMap.clear();
}