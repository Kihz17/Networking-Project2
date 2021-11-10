#pragma once

#include "Packet.h"

namespace netutils
{
	class PacketLoginRequest : public IPacket
	{
	public:
		PacketLoginRequest(long requestId, const std::string& email, const std::string& password);
		PacketLoginRequest(Buffer& buffer);
		virtual ~PacketLoginRequest() = default;

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
