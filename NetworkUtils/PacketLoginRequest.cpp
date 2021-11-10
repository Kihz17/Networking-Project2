#include "PacketLoginRequest.h"

#include <Auth.pb.h>

namespace netutils
{
	PacketLoginRequest::PacketLoginRequest(long requestId, const std::string& email, const std::string& password)
		: IPacket(4), requestId(requestId), email(email), password(password), dataLength(0)
	{

	}

	void PacketLoginRequest::Serialize(Buffer& buffer)
	{
		auth::AuthenticateWeb loginRequestProto;
		loginRequestProto.set_requestid(this->requestId);
		loginRequestProto.set_email(this->email);
		loginRequestProto.set_plaintextpassword(this->password);
		if (!loginRequestProto.SerializeToString(&this->data))
		{
			std::cout << "Failed to serialize PacketLoginRequest protobuf data!" << std::endl;
			return;
		}

		this->dataLength = this->data.length();

		IPacket::Serialize(buffer);
		buffer.WriteInt(this->dataLength);
		buffer.WriteString(this->data);
	}

	void PacketLoginRequest::Deserialize(Buffer& buffer)
	{
		this->dataLength = buffer.ReadInt();
		this->data = buffer.ReadString(this->dataLength);

		auth::AuthenticateWeb loginRequestProto;
		if (!loginRequestProto.ParseFromString(this->data))
		{
			std::cout << "Failed to deserialzie PacketLoginRequest data from protobuf!" << std::endl;
			return;
		}

		this->requestId = loginRequestProto.requestid();
		this->email = loginRequestProto.email();
		this->password = loginRequestProto.plaintextpassword();
	}

	size_t PacketLoginRequest::GetSize()
	{
		return sizeof(PacketHeader) + dataLength;
	}
}
