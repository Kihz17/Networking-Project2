#include "IPacketHandler.h"

class PacketRegisterResultHandler : public IPackethandler
{
public:
	virtual void HandleOnClient(Client& client, const SOCKET& serverSocket);

private:
	friend class PacketManager;
	PacketRegisterResultHandler() = default;
};