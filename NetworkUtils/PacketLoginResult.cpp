#include "PacketLoginResult.h"

namespace netutils
{
	PacketLoginResult::PacketLoginResult(long requestId, long userId, auth::AuthenticateWebResult_AuthenticateResult result, const std::string& creationDate, const std::string& email)
		: IPacket(6), requestId(requestId), userId(userId), result(result), creationDate(creationDate), email(email)
	{

	}

	PacketLoginResult::PacketLoginResult(Buffer& buffer)
		: IPacket(6)
	{
		Deserialize(buffer);
	}

	void PacketLoginResult::Serialize(Buffer& buffer)
	{
		auth::AuthenticateWebResult loginResultProto;
		loginResultProto.set_requestid(this->requestId);
		loginResultProto.set_userid(this->userId);
		loginResultProto.set_reason(this->result);
		loginResultProto.set_creationdate(this->creationDate);
		loginResultProto.set_email(this->email);
		if (!loginResultProto.SerializeToString(&this->data))
		{
			std::cout << "Failed to serialize PacketLoginResult protobuf data!" << std::endl;
			return;
		}

		this->dataLength = this->data.length();

		IPacket::Serialize(buffer);
		buffer.WriteInt(this->dataLength);
		buffer.WriteString(this->data);
	}

	void PacketLoginResult::Deserialize(Buffer& buffer)
	{
		this->dataLength = buffer.ReadInt();
		this->data = buffer.ReadString(this->dataLength);

		auth::AuthenticateWebResult loginResultProto;
		if (!loginResultProto.ParseFromString(this->data))
		{
			std::cout << "Failed to deserialzie PacketLoginResult data from protobuf!" << std::endl;
			return;
		}

		this->requestId = loginResultProto.requestid();
		this->userId = loginResultProto.userid();
		this->result = loginResultProto.reason();
		this->creationDate = loginResultProto.creationdate();
		this->email = loginResultProto.email();
	}

	size_t PacketLoginResult::GetSize()
	{
		return sizeof(PacketHeader) + dataLength;
	}
}
