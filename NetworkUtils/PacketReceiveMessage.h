#pragma once

#include "Packet.h"

namespace netutils
{
	class PacketReceiveMessage : public IPacket
	{
	public:
		PacketReceiveMessage(const std::string& senderName, const std::string& message);
		virtual ~PacketReceiveMessage() = default;

		virtual void Serialize(Buffer& buffer);
		virtual void Deserialize(Buffer& buffer);
		virtual size_t GetSize();

		int nameLength;
		std::string senderName;
		int messageLength;
		std::string message;
	};
}
