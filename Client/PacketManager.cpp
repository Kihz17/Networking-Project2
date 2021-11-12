#include "PacketManager.h"
#include "PacketReceiveMessageHandler.h"
#include "PacketJoinRoomHandler.h"
#include "PacketLeaveRoomHandler.h"
#include "PacketLoginResultHandler.h"
#include "PacketRegisterResultHandler.h"

#include <iostream>

PacketManager* PacketManager::instance = NULL;

PacketManager* PacketManager::GetInstance()
{
	if (PacketManager::instance == NULL)
	{
		PacketManager::instance = new PacketManager();
		instance->handlerMap.insert(std::make_pair(1, new PacketReceiveMessageHandler()));
		instance->handlerMap.insert(std::make_pair(2, new PacketJoinRoomHandler()));
		instance->handlerMap.insert(std::make_pair(3, new PacketLeaveRoomHandler()));
		instance->handlerMap.insert(std::make_pair(6, new PacketLoginResultHandler()));
		instance->handlerMap.insert(std::make_pair(7, new PacketRegisterResultHandler()));
	}

	return instance;
}

void PacketManager::HandlePacket(Client& client, const SOCKET& serverSocket, unsigned int packetType)
{
	std::map<unsigned int, IPackethandler*>::iterator it = this->handlerMap.find(packetType);
	if (it != this->handlerMap.end())
	{
		it->second->HandleOnClient(client, serverSocket);
	}
	else
	{
		std::cout << "Packet handler for type " << packetType << " was not found!" << std::endl;
	}
}

void PacketManager::CleanUp()
{
	std::map<unsigned int, IPackethandler*>::iterator it;
	for (it = this->handlerMap.begin(); it != this->handlerMap.end(); it++)
	{
		delete it->second;
	}

	this->handlerMap.clear();
}