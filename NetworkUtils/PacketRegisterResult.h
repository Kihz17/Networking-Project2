#pragma once

#include "Packet.h"
#include <Auth.pb.h>

namespace netutils
{
	class PacketRegisterResult : public IPacket
	{
	public:
		PacketRegisterResult(long requestId, long userId, auth::CreateAccountWebResult_CreateAccountResult result);
		PacketRegisterResult(Buffer& buffer);
		virtual ~PacketRegisterResult() = default;

		virtual void Serialize(Buffer& buffer);
		virtual void Deserialize(Buffer& buffer);
		virtual size_t GetSize();

		long requestId;
		long userId;
		auth::CreateAccountWebResult_CreateAccountResult result;

	private:
		int dataLength;
		std::string data;
	};
}
