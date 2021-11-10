#include "PacketLeaveRoomHandler.h"

PacketLeaveRoomHandler::PacketLeaveRoomHandler()
{

}

void PacketLeaveRoomHandler::HandleOnServer(Server& server, Client* sender) 
{
	netutils::PacketLeaveRoom leave(sender->buffer);
	server.LeaveRoom(sender, leave);
}
