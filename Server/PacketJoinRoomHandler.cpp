#include "PacketJoinRoomHandler.h"

#include <PacketJoinRoom.h>

PacketJoinRoomHandler::PacketJoinRoomHandler()
{

}

void PacketJoinRoomHandler::HandleOnServer(Server& server, Client* sender) 
{
	netutils::PacketJoinRoom joinPacket(sender->buffer);
	server.JoinRoom(sender, joinPacket);
}


