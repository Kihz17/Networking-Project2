#pragma once

#include "Packet.h"

namespace netutils
{
	class PacketSendMessage : public IPacket
	{
	public:
		PacketSendMessage(const std::string& message);
		PacketSendMessage(Buffer& buffer);
		virtual ~PacketSendMessage() = default;

		virtual void Serialize(Buffer& buffer);
		virtual void Deserialize(Buffer& buffer);
		virtual size_t GetSize();

		int messageLength;
		std::string message;
	};
}
