#pragma once

#include "Packet.h"

namespace netutils
{
	class PacketRegisterRequest : public IPacket
	{
	public:
		PacketRegisterRequest(long requestId, const std::string& email, const std::string& password);
		virtual ~PacketRegisterRequest() = default;

		virtual void Serialize(Buffer& buffer);
		virtual void Deserialize(Buffer& buffer);
		virtual size_t GetSize();

		long requestId;
		std::string email;
		std::string password;

	private:
		int dataLength;
		std::string data;
	};
}
