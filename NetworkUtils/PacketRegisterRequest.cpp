#include "PacketRegisterRequest.h"

#include <Auth.pb.h>

namespace netutils
{
	PacketRegisterRequest::PacketRegisterRequest(long requestId, const std::string& email, const std::string& password)
		: IPacket(5), requestId(requestId), email(email), password(password), dataLength(0)
	{

	}

	void PacketRegisterRequest::Serialize(Buffer& buffer)
	{
		auth::CreateAccountWeb registerRequestProto;
		registerRequestProto.set_requestid(this->requestId);
		registerRequestProto.set_email(this->email);
		registerRequestProto.set_plaintextpassword(this->password);
		if (!registerRequestProto.SerializeToString(&this->data))
		{
			std::cout << "Failed to serialize PacketRegisterRequest protobuf data!" << std::endl;
			return;
		}

		this->dataLength = this->data.length();

		IPacket::Serialize(buffer);
		buffer.WriteInt(this->dataLength);
		buffer.WriteString(this->data);
	}

	void PacketRegisterRequest::Deserialize(Buffer& buffer)
	{
		this->dataLength = buffer.ReadInt();
		this->data = buffer.ReadString(this->dataLength);

		auth::CreateAccountWeb registerRequestProto;
		if (!registerRequestProto.ParseFromString(this->data))
		{
			std::cout << "Failed to deserialzie PacketRegisterRequest data from protobuf!" << std::endl;
			return;
		}

		this->requestId = registerRequestProto.requestid();
		this->email = registerRequestProto.email();
		this->password = registerRequestProto.plaintextpassword();
	}

	size_t PacketRegisterRequest::GetSize()
	{
		return sizeof(PacketHeader) + dataLength;
	}
}
