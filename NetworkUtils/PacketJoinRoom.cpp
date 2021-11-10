#include "PacketJoinRoom.h"

namespace netutils
{
	PacketJoinRoom::PacketJoinRoom(const std::string& roomName, const std::string& name)
		: IPacket(2), roomName(roomName), roomNameLength(roomName.length()), name(name), nameLength(name.length())
	{

	}

	void PacketJoinRoom::Serialize(Buffer& buffer)
	{
		IPacket::Serialize(buffer);
		buffer.WriteInt(this->roomNameLength);
		buffer.WriteString(this->roomName);
		buffer.WriteInt(this->nameLength);
		buffer.WriteString(this->name);
	}
	
	void PacketJoinRoom::Deserialize(Buffer& buffer)
	{
		this->roomNameLength = buffer.ReadInt();
		this->roomName = buffer.ReadString(roomNameLength);
		this->nameLength = buffer.ReadInt();
		this->name = buffer.ReadString(nameLength);
	}

	size_t PacketJoinRoom::GetSize()
	{
		return sizeof(PacketHeader) + roomNameLength + nameLength;
	}
}
