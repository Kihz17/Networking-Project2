#pragma once

#include <string>
#include <winsock2.h>

#include "Buffer.h"

#define DEFAULT_BUFLEN 512	

class Client
{
public:
    Client(unsigned int id) : id(id), authenticated(false), socket(NULL), buffer(DEFAULT_BUFLEN) {}

    unsigned int id; // Represents the client's userId (will be given a random value when they are unauthenticated)
    bool authenticated; // Represents whether this client is authenticated or not

    std::string name;
    SOCKET socket;
    netutils::Buffer buffer;
};