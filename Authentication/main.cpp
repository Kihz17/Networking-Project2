#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "27778"	

#include "AuthServer.h"

int main()
{  
    char formattedDate[100];
    time_t* currTime;
    tm* time;
    std::time(currTime);
    time = std::localtime(currTime);

    std::cout << std::strftime(formattedDate, 59, "%d-%Y", time) << std::endl;
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