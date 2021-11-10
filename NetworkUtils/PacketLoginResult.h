#pragma once

#include "Packet.h"

#include <Auth.pb.h>

namespace netutils
{
	class PacketLoginResult : public IPacket
	{
	public:
		PacketLoginResult(long requestId, long userId, auth::AuthenticateWebResult_AuthenticateResult result, const std::string& creationDate);
		PacketLoginResult(Buffer& buffer);
		virtual ~PacketLoginResult() = default;

		virtual void Serialize(Buffer& buffer);
		virtual void Deserialize(Buffer& buffer);
		virtual size_t GetSize();

		long requestId;
		long userId;
		auth::AuthenticateWebResult_AuthenticateResult result;
		std::string creationDate;

	private:
		int dataLength;
		std::string data;
	};
}
