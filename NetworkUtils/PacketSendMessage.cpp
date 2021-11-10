#include "PacketSendMessage.h"

namespace netutils
{
	PacketSendMessage::PacketSendMessage(const std::string& message)
		: IPacket(0), messageLength(message.length()), message(message)
	{

	}

	PacketSendMessage::PacketSendMessage(Buffer& buffer)
		: IPacket(0)
	{
		Deserialize(buffer);
	}

	void PacketSendMessage::Serialize(Buffer& buffer)
	{
		IPacket::Serialize(buffer);
		buffer.WriteInt(this->messageLength);
		buffer.WriteString(this->message);
	}

	void PacketSendMessage::Deserialize(Buffer& buffer)
	{
		this->messageLength = buffer.ReadInt();
		this->message = buffer.ReadString(this->messageLength);
	}

	size_t PacketSendMessage::GetSize()
	{
		return sizeof(PacketHeader) + this->messageLength;
	}
}
