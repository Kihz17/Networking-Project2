#pragma once

#include "Packet.h"

namespace netutils
{
	class PacketJoinRoom : public IPacket
	{
	public:
		PacketJoinRoom(const std::string& roomName, const std::string& name);
		virtual ~PacketJoinRoom() = default;

		virtual void Serialize(Buffer & buffer);
		virtual void Deserialize(Buffer & buffer);
		virtual size_t GetSize();

		int roomNameLength;
		std::string roomName;
		int nameLength;
		std::string name;
	};
}
