#include "PacketRegisterResult.h"

namespace netutils
{
	PacketRegisterResult::PacketRegisterResult(long requestId, long userId, auth::CreateAccountWebResult_CreateAccountResult result)
		: IPacket(7), requestId(requestId), userId(userId), result(result)
	{

	}

	PacketRegisterResult::PacketRegisterResult(Buffer& buffer)
		: IPacket(7)
	{
		Deserialize(buffer);
	}

	void PacketRegisterResult::Serialize(Buffer& buffer)
	{
		auth::CreateAccountWebResult createAccountResultProto;
		createAccountResultProto.set_requestid(this->requestId);
		createAccountResultProto.set_userid(this->userId);
		createAccountResultProto.set_reason(this->result);
		if (!createAccountResultProto.SerializeToString(&this->data))
		{
			std::cout << "Failed to serialize PacketRegisterResult protobuf data!" << std::endl;
			return;
		}

		this->dataLength = this->data.length();

		IPacket::Serialize(buffer);
		buffer.WriteInt(this->dataLength);
		buffer.WriteString(this->data);
	}

	void PacketRegisterResult::Deserialize(Buffer& buffer)
	{
		this->dataLength = buffer.ReadInt();
		this->data = buffer.ReadString(this->dataLength);

		auth::CreateAccountWebResult createAccountResultProto;
		if (!createAccountResultProto.ParseFromString(this->data))
		{
			std::cout << "Failed to deserialzie PacketRegisterResult data from protobuf!" << std::endl;
			return;
		}

		this->requestId = createAccountResultProto.requestid();
		this->userId = createAccountResultProto.userid();
		this->result = createAccountResultProto.reason();
	}

	size_t PacketRegisterResult::GetSize()
	{
		return sizeof(PacketHeader) + dataLength;
	}
}
