#include "PacketReceiveMessage.h"

namespace netutils
{
	PacketReceiveMessage::PacketReceiveMessage(const std::string& senderName, const std::string& message)
		: IPacket(1), nameLength(senderName.length()), senderName(senderName), messageLength(message.length()), message(message)
	{

	}

	void PacketReceiveMessage::Serialize(Buffer& buffer)
	{
		IPacket::Serialize(buffer);
		buffer.WriteInt(this->nameLength);
		buffer.WriteString(this->senderName);
		buffer.WriteInt(this->messageLength);
		buffer.WriteString(this->message);
	}

	void PacketReceiveMessage::Deserialize(Buffer& buffer)
	{
		this->nameLength = buffer.ReadInt();
		this->senderName = buffer.ReadString(this->nameLength);
		this->messageLength = buffer.ReadInt();
		this->message = buffer.ReadString(this->messageLength);
	}

	size_t PacketReceiveMessage::GetSize()
	{
		return sizeof(PacketHeader) + this->nameLength + this->messageLength;
	}
}
