#include "AuthServer.h"
#include <Buffer.h>
#include "PacketManager.h"

AuthServer::AuthServer(PCSTR port)
{
    this->port = port;
    this->connectionSocket = INVALID_SOCKET; // This socket is used to listen for incoming connections
    this->acceptSocket = INVALID_SOCKET;
}

AuthServer::~AuthServer()
{

}

bool AuthServer::Initialize()
{
    printf("Initializing Authentication Server...\n");

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

    return database.Connect("127.0.0.1", "root", "TSOPTSOP77!!");
}

void AuthServer::Start()
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

        // Add all client sockets to the read set
        for (AuthClient* client : clients)
        {
            FD_SET(client->socket, &readSet);
        }

        // Find the sockets that need updating
        total = select(0, &readSet, NULL, NULL, &timeoutValue);
        if (total == SOCKET_ERROR) {
            printf("select() has failed! %d\n", WSAGetLastError());
            return;
        }

        // Handle incoming connections
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
                AuthClient* client = new AuthClient();
                client->socket = this->acceptSocket;
                this->clients.push_back(client);
                printf("New client has connected on socket %d.\n", (int)this->acceptSocket);
            }
        }

        for (int i = this->clients.size() - 1; i >= 0; i--)
        {
            AuthClient* client = clients[i];

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

    ShutDown();
}

void AuthServer::ShutDown()
{
    printf("Server shutting down...\n");
    closesocket(this->acceptSocket);
    closesocket(this->connectionSocket);
    this->database.CloseConnection();
}

auth::AuthenticateWebResult_AuthenticateResult AuthServer::AuthenticateAccount(const std::string& email, const std::string& plainTextPassword, long& userId, std::string& creationDate)
{
    return this->database.AuthenticateAccount(email, plainTextPassword, userId, creationDate);
}

auth::CreateAccountWebResult_CreateAccountResult AuthServer::CreateAccount(const std::string& email, const std::string& plainTextPassword, long& userId)
{
    return this->database.CreateAccount(email, plainTextPassword, userId);
}