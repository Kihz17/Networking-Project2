#pragma once

#include "Buffer.h"

#include <string>

namespace netutils
{

	struct PacketHeader
	{
		int packetType;
	};

	// 0 == Send Message
	// 1 == Receive Message
	// 2 == Join Room 
	// 3 = Leave Room
	// 4 == Login Request
	// 5 == Register Request
	// 6 == Login Result
	// 7 == Register Result
	class IPacket
	{
	public:
		IPacket(int packetType)
		{
			this->header.packetType = packetType;
		}

		virtual ~IPacket() = default;

		virtual void Serialize(Buffer& buffer)
		{
			buffer.WriteInt(this->header.packetType);
		}

		virtual void Deserialize(Buffer& buffer) = 0;

		virtual size_t GetSize() = 0;

	protected:
		PacketHeader header;
	};
}