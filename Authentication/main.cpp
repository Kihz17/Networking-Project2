#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "27778"	

#include "AuthServer.h"

#include <Auth.pb.h>

#include "BCryptUtils.h"

int main()
{  
    char salt[BCRYPT_HASHSIZE];
    BCryptUtils::GenerateSalt(salt);
    std::string saltS(salt);
    std::string hash = BCryptUtils::GenerateHash("password", salt);
    std::cout << "Salt: " << salt << " (" << saltS.length() << ")" << std::endl;
    std::cout << "Hash: " << hash << " (" << hash.length() << ")" << std::endl;
    int result = 0;
    WSADATA wsaData; // Holds winsock data

    // Initialize winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return 1;
    }

    AuthServer server(DEFAULT_PORT);
    if (!server.Initialize())
    {
        printf("Authentication server failed to initialize!\n");
        return 1;
    }

    server.Start();

    WSACleanup();
    return 0;
}