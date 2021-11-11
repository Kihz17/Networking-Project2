#include "Server.h"
#include "PacketManager.h"

#include <PacketLoginRequest.h>
#include <PacketRegisterRequest.h>
#include <PacketLoginResult.h>
#include <PacketRegisterResult.h>

#include <iostream>

unsigned int currentUnauthenticatedClientId = 0; // Represents the Id counter for our current unauthed clients

Server::Server(PCSTR authServerIp, PCSTR authPort, PCSTR port)
    :authServerIp(authServerIp), authServerPort(authPort), authServerSocket(INVALID_SOCKET), port(port), connectionSocket(INVALID_SOCKET), acceptSocket(INVALID_SOCKET)
{

}

Server::~Server()
{
    for (Client* client : clients)
    {
        delete client;
    }

    clients.clear();
}

bool Server::Initialize()
{
    if (!ResolveAuthServer())
    {
        printf("Failed to connect to Authentication Sever...");
        return false;
    }

    printf("Initializing server...\n");

    int result;
    struct addrinfo* info = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints)); // Fills the hints block of memory with 0's
    hints.ai_family = AF_INET;  // Specify address fmaily
    hints.ai_socktype = SOCK_STREAM; // Assign socket type
    hints.ai_protocol = IPPROTO_TCP; // Use TCP for protocol
    hints.ai_flags = AI_PASSIVE;

    // Resolve port & address
    result = getaddrinfo(NULL, this->port, &hints, &info);
    if (result != 0)
    {
        printf("getaddrinfo failed with error: %d\n", result);
        WSACleanup();
        return false;
    }

    this->connectionSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol); // Assign our socket with our address info properties
    if (this->connectionSocket == INVALID_SOCKET)
    { // We failed...
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(info);
        WSACleanup();
        return false;
    }

    // Attempt to bind our connection socket
    result = bind(this->connectionSocket, info->ai_addr, (int)info->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(info);
        closesocket(this->connectionSocket);
        WSACleanup();
        return false;
    }

    freeaddrinfo(info); // Socket was bound, we don't need this anymore.

    // Listen for activing on our connection socket
    result = listen(this->connectionSocket, SOMAXCONN); // Puts the connection socket in a state where it is listening for an incoming connection
    if (result == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(this->connectionSocket);
        WSACleanup();
        return false;
    }

    // Force our connection socket to be non-blocking
    DWORD nonBlock = 1;
    result = ioctlsocket(this->connectionSocket, FIONBIO, &nonBlock);
    if (result == SOCKET_ERROR)
    {
        printf("Failed to make connection socket non-blocking! %d\n", WSAGetLastError());
        closesocket(this->connectionSocket);
        WSACleanup();
        return false;
    }

    return true;
}

void Server::Start()
{
    int result;
    int total;
  
    DWORD flags;
    DWORD bytesReceived;
    DWORD nonBlock = 1;

    FD_SET readSet;

    printf("Server started!\n");
    while (true)
    {
        timeval timeoutValue = { 0 };
        timeoutValue.tv_sec = 2; // 2 second timeout value

        FD_ZERO(&readSet); // Wipe out our read set
        FD_SET(connectionSocket, &readSet); // Add connection socket to the read set (AKA: Keep listening for connections)
        FD_SET(this->authServerSocket, &readSet); // Read from auth server

        // Add all client sockets to the read set
        for (Client* client : clients)
        {
            FD_SET(client->socket, &readSet);
        }

        // Read from unauthenticated clients
        {
            std::map<unsigned int, Client*> ::iterator it = this->unauthenticatedClients.begin();
            while (it != this->unauthenticatedClients.end())
            {
                FD_SET(it->second->socket, &readSet);
                it++;
            }
        }

        // Find the sockets that need updating
        total = select(0, &readSet, NULL, NULL, &timeoutValue);
        if (total == SOCKET_ERROR) {
            printf("select() has failed! %d\n", WSAGetLastError());
            return;
        }


        // Handle incoming connections
        {
            if (FD_ISSET(connectionSocket, &readSet))
            {
                total--;
                this->acceptSocket = accept(connectionSocket, NULL, NULL); // Permits the incoming connection
                if (acceptSocket == INVALID_SOCKET)
                {
                    printf("Failed to accept socket! %d\n", WSAGetLastError());
                    break;
                }

                // Make the newly accept socket non-blocking
                result = ioctlsocket(this->acceptSocket, FIONBIO, &nonBlock);
                if (result == SOCKET_ERROR)
                {
                    printf("Failed to make accepted socket non-blocking! %d\n", WSAGetLastError());
                }
                else
                {
                    Client* client = new Client(currentUnauthenticatedClientId++);
                    client->socket = this->acceptSocket;
                    this->unauthenticatedClients.insert(std::make_pair(client->id, client));
                    printf("New unauthenticated client has connected on socket %d.\n", (int)this->acceptSocket);
                }
            }
        }


        // Handle incoming data from auth server
        {
            if (FD_ISSET(this->authServerSocket, &readSet))
            {
                total--;
                DWORD flags = 0;
                netutils::Buffer buffer(DEFAULT_BUFLEN);
                bytesReceived = recv(this->authServerSocket, buffer.data, buffer.Length(), flags);
                if (bytesReceived == SOCKET_ERROR)
                {
                    printf("Failed to recv from Authentication Server!\n");
                    break;
                }
                else if (bytesReceived == 0) // Client left
                {
                    // remove client from vector array 
                    printf("Authentication Server disconnected!\n");
                    break;
                }

                int packetHeader = buffer.ReadInt();
                // We don't need packet handlers for this since it's only 2 packets
                if (packetHeader == 6) // Login result
                {
                    netutils::PacketLoginResult loginResult(buffer); // Deserialize the data
                    long unauthenticatedClientId = loginResult.requestId;
                    std::map<unsigned int, Client*>::iterator it = this->unauthenticatedClients.find(unauthenticatedClientId);
                    if (it != this->unauthenticatedClients.end()) // Client still exists
                    {
                        Client* client = it->second;

                        if (loginResult.result == auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_SUCCESS) // It was a success! Authenticate the client
                        {
                            this->unauthenticatedClients.erase(it);
                            this->clients.push_back(client); // Add to authenticated client vector
                        }

                        SendToClient(client, buffer.data, buffer.Length()); // Tell the client the result
                    }
                }
                else if (packetHeader == 7) // Register result
                {
                    netutils::PacketRegisterResult registreResult(buffer); // Deserialize the data
                    long unauthenticatedClientId = registreResult.requestId;
                    std::map<unsigned int, Client*>::iterator it = this->unauthenticatedClients.find(unauthenticatedClientId);
                    if (it != this->unauthenticatedClients.end()) // Client still exists
                    {
                        Client* client = it->second;
                        SendToClient(client, buffer.data, buffer.Length()); // Tell the client the result
                    }
                }
            }
        }


        // Handle incoming data from authenticated clients
        {
            for (int i = this->clients.size() - 1; i >= 0; i--)
            {
                Client* client = clients[i];

                // Check if we are in the read set (AKA: Check if socket sent some new data)
                if (FD_ISSET(client->socket, &readSet))
                {
                    total--;
                    DWORD flags = 0;
                    bytesReceived = recv(client->socket, client->buffer.data, client->buffer.Length(), flags); // Recieve the data (THIS IS BLOCKING, which is we we only call it when the socket has new data)
                    if (bytesReceived == SOCKET_ERROR)
                    {
                        printf("recv() has failed!\n");

                        //if (WSAGetLastError() == 10054) {
                            // remove client from vector array 
                        printf("Client disconnected!\n");
                        this->clients[i] = clients[this->clients.size() - 1];
                        this->clients.pop_back();
                        continue;
                        //}

                    }
                    else if (bytesReceived == 0) // Client left
                    {
                        // remove client from vector array 
                        printf("Client disconnected!\n");
                        this->clients[i] = clients[this->clients.size() - 1];
                        this->clients.pop_back();
                        continue;
                    }

                    int packetHeader = client->buffer.ReadInt();
                    PacketManager::GetInstance()->HandlePacket(*this, client, packetHeader);
                    client->buffer.Clear();
                }
            }
        }
       

        // Read from unauthenticated clients
        {
            std::map<unsigned int, Client*>::iterator it;
            std::vector<unsigned int> unauthedClientsToRemove;
            for (it = this->unauthenticatedClients.begin(); it != this->unauthenticatedClients.end(); it++)
            {
                Client* client = it->second;
                if (FD_ISSET(client->socket, &readSet))
                {
                    total--;
                    DWORD flags = 0;
                    bytesReceived = recv(client->socket, client->buffer.data, client->buffer.Length(), flags); // Recieve the data (THIS IS BLOCKING, which is we we only call it when the socket has new data)
                    if (bytesReceived == SOCKET_ERROR)
                    {
                        printf("recv() has failed!\n");
                        printf("Unauthenticated client disconnected!\n");
                        unauthedClientsToRemove.push_back(client->id); // Flag for removal
                        continue;
                    }
                    else if (bytesReceived == 0) // Client left
                    {
                        // remove client from vector array 
                        printf("Client disconnected!\n");
                        unauthedClientsToRemove.push_back(client->id);  // Flag for removal
                        continue;
                    }

                    int packetHeader = client->buffer.ReadInt();
                    // Only look for the 2 auth request packets, we don't want to handle other packets from unauthed clients
                    // Since these are generally small, we don't really need packet handlers for them
                    if (packetHeader == 4) // Login request
                    {
                        netutils::PacketLoginRequest loginPacket(client->buffer); // Deserialize the data
                        loginPacket.requestId = client->id; // Make sure we assign the requestId to the client's unauthed ID
                        netutils::Buffer buffer(DEFAULT_BUFLEN);
                        loginPacket.Serialize(buffer);
                        SendToAuthServer(buffer); // Forward this to the auth server
                    }
                    else if (packetHeader == 5) // Register request
                    {
                        netutils::PacketRegisterRequest registerPacket(client->buffer); // Deserialize the data
                        registerPacket.requestId = client->id; // Make sure we assign the requestId to the client's unauthed ID
                        netutils::Buffer buffer(DEFAULT_BUFLEN);
                        registerPacket.Serialize(buffer);
                        SendToAuthServer(buffer); // Forward this to the auth server
                    }

                    client->buffer.Clear();
                }
            }

            // Remove invalid clients
            for (unsigned int& id : unauthedClientsToRemove)
            {
                std::map<unsigned int, Client*>::iterator it = this->unauthenticatedClients.find(id);
                if (it != this->unauthenticatedClients.end())
                {
                    Client* client = it->second;
                    this->unauthenticatedClients.erase(id);
                    delete client;
                }

            }
        }
    }

    ShutDown();
}

void Server::ShutDown()
{
    printf("Server shutting down...\n");
    closesocket(this->acceptSocket);
    closesocket(this->connectionSocket);
    for (Client* client : clients)
    {
        closesocket(client->socket);
    }
}

void Server::BroadcastMessage(char* dataToSend, int dataLength)
{
    int result;

    for (Client* client : this->clients)
    {
        result = send(client->socket, dataToSend, dataLength, 0);
        if (result == SOCKET_ERROR)
        {
            printf("send() has failed!");
            continue;
        }
    }
}

void Server::SendToClient(Client* client, char* dataToSend, int dataLength)
{
    int result;
    result = send(client->socket, dataToSend, dataLength, 0);
    if (result == SOCKET_ERROR)
    {
        printf("send() has failed!");
    }
}

void Server::BroadcastMessageExcludeClient(Client* exclude, char* dataToSend, int dataLength)
{
    int result;

    for (Client* client : this->clients)
    {
        if (client == exclude)
        {
            continue;
        }

        result = send(client->socket, dataToSend, dataLength, 0);
        if (result == SOCKET_ERROR)
        {
            printf("send() has failed!");
            continue;
        }
    }
}

void Server::BroadcastToRoom(std::string roomName, char* dataToSend, int dataLength) {

       std::map<std::string,std::vector<Client*>>::iterator it =  this->rooms.find(roomName);

       if (it == rooms.end()) {
           printf("Room was not Found %s\n", roomName.c_str());
           return;
       }

       int result;
       for (Client* client : it->second) {

           result = send(client->socket, dataToSend, dataLength, 0);
           if (result == SOCKET_ERROR)
           {
               printf("send() has failed!");
               continue;
           }

       
       }//for loop
       
}

void Server::BroadcastToRoomExcludeClient(std::string roomName, Client* exclude, char* dataToSend, int dataLength)
{
    std::map<std::string, std::vector<Client*>>::iterator it = this->rooms.find(roomName);

    if (it == rooms.end()) {
        printf("Room was not Found %s\n", roomName);
        return;
    }

    int result;
    for (Client* client : it->second) 
    {
        if (client == exclude)
        {
            continue;
        }

        result = send(client->socket, dataToSend, dataLength, 0);
        if (result == SOCKET_ERROR)
        {
            printf("send() has failed!");
            continue;
        }


    }//for loop
}


//std::map<std::string, std::vector<Client*> > rooms;
void Server::JoinRoom(Client* name,  netutils::PacketJoinRoom& packet) 
{
    std::string roomName = packet.roomName;
    name->name = packet.name;
    std::string currentRoom = FindClientRoom(name);
    if (currentRoom != "")
    {
        netutils::PacketLeaveRoom leavePacket(currentRoom, name->name);
        LeaveRoom(name, leavePacket);
    }

    rooms[roomName].push_back(name);
    this->clientToRoomMap.insert(std::make_pair(name, roomName));
    std::cout << name->name << " has joined room " << roomName << std::endl;

    // Pack the packet up again to be sent
    netutils::Buffer buffer(packet.GetSize());
    packet.Serialize(buffer);

    BroadcastToRoom(roomName, buffer.data, buffer.Length());
}

void Server::LeaveRoom(Client* name, netutils::PacketLeaveRoom& packet)
{
    std::string roomName = packet.roomName;
    std::map<std::string, std::vector<Client*>>::iterator it = this->rooms.find(roomName);

    if (it != rooms.end()) 
    {
        std::vector<Client*>::iterator client = std::find(it->second.begin(), it->second.end(), name);
        if (client != it->second.end()) 
        {
            it->second.erase(client);
        }//if inside iterator
   
    }

    this->clientToRoomMap.erase(name);
    std::cout << name->name << " has left room " << roomName << std::endl;

    // Resent data back to clients
    netutils::Buffer buffer(packet.GetSize());
    packet.Serialize(buffer);

    SendToClient(name, buffer.data, buffer.Length());
    BroadcastToRoom(roomName, buffer.data, buffer.Length());
}

std::string Server::FindClientRoom(Client* client)
{
    std::map<Client*, std::string>::iterator it = this->clientToRoomMap.find(client);
    if (it != this->clientToRoomMap.end())
    {
        return it->second;
    }

    return "";
}

bool Server::ResolveAuthServer()
{
    printf("Connecting to Authentication Server...\n");

    struct addrinfo* infoResult = NULL; // Holds the address information of our server
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    int result;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve server
    result = getaddrinfo(this->authServerIp, this->authServerPort, &hints, &infoResult);
    if (result != 0)
    {
        printf("getaddrinfo failed with error: %d\n", result);
        WSACleanup();
        return 1;
    }

    // Try to connect to an address
    for (ptr = infoResult; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        this->authServerSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (this->authServerSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        result = connect(this->authServerSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            closesocket(this->authServerSocket);
            this->authServerSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(infoResult); // Done connecting, free from memory

    if (this->authServerSocket == INVALID_SOCKET)
    {
        printf("Failed to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Force our connection socket to be non-blocking
    DWORD nonBlock = 1;
    result = ioctlsocket(this->authServerSocket, FIONBIO, &nonBlock);
    if (result == SOCKET_ERROR)
    {
        printf("Failed to make connection socket non-blocking! %d\n", WSAGetLastError());
        closesocket(this->authServerSocket);
        WSACleanup();
        return false;
    }

    printf("Connection to Authentication Server was successful!\n");

    return true;
}

void Server::SendToAuthServer(netutils::Buffer& buffer)
{
    int result;
    result = send(this->authServerSocket, buffer.data, buffer.Length(), 0);
    if (result == SOCKET_ERROR)
    {
        printf("Failed to send data to Authentication Server!\n");
    }
}