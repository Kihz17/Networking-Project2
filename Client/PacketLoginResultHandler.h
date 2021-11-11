#include "IPacketHandler.h"

class PacketLoginResultHandler : public IPackethandler
{
public:
	virtual void HandleOnClient(Client& client, const SOCKET& serverSocket);

private:
	friend class PacketManager;
	PacketLoginResultHandler() = default;
};