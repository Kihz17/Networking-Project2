#include "PacketSendMessageHandler.h"

#include <PacketSendMessage.h>
#include <PacketReceiveMessage.h>

PacketSendMessageHandler::PacketSendMessageHandler()
{

}

void PacketSendMessageHandler::HandleOnServer(Server& server, Client* sender)
{
	netutils::PacketSendMessage sendMessagePacket(sender->buffer);

	netutils::PacketReceiveMessage receivePacket(sender->name, sendMessagePacket.message);
	netutils::Buffer buffer(receivePacket.GetSize());
	receivePacket.Serialize(buffer);
	server.BroadcastToRoomExcludeClient(server.FindClientRoom(sender), sender, buffer.data, buffer.Length()); // Broadcast message to all clients except for the sender
}


