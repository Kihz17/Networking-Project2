#include "PacketLeaveRoom.h"

namespace netutils
{
	PacketLeaveRoom::PacketLeaveRoom(const std::string& roomName, const std::string& name)
		: IPacket(3), roomName(roomName), roomNameLength(roomName.length()), name(name), nameLength(name.length())
	{

	}

	PacketLeaveRoom::PacketLeaveRoom(Buffer& buffer)
		: IPacket(3)
	{
		Deserialize(buffer);
	}

	void PacketLeaveRoom::Serialize(Buffer& buffer)
	{
		IPacket::Serialize(buffer);
		buffer.WriteInt(this->roomNameLength);
		buffer.WriteString(this->roomName);
		buffer.WriteInt(this->nameLength);
		buffer.WriteString(this->name);
	}

	void PacketLeaveRoom::Deserialize(Buffer& buffer)
	{
		this->roomNameLength = buffer.ReadInt();
		this->roomName = buffer.ReadString(roomNameLength);
		this->nameLength = buffer.ReadInt();
		this->name = buffer.ReadString(nameLength);
	}

	size_t PacketLeaveRoom::GetSize()
	{
		return sizeof(PacketHeader) + roomNameLength + nameLength;
	}
}
