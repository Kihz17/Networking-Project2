#include "BCryptUtils.h"

#include <iostream>

bool BCryptUtils::GenerateSalt(char* salt, int workload)
{
	int result = bcrypt_gensalt(workload, salt);
	if (result != 0)
	{
		std::cout << "Failed to generate salt!" << std::endl;
		return false;
	}

	return true;
}

std::string BCryptUtils::GenerateHash(const std::string& password, char* salt)
{
	char hashedPassword[BCRYPT_HASHSIZE];
	std::string saltString(salt);
	int result = bcrypt_hashpw(password.c_str(), salt, hashedPassword);
	if (result != 0)
	{
		std::cout << "Failed to generate hash!" << std::endl;
		return "";
	}

	std::string hash(hashedPassword);
	hash = hash.substr(saltString.length(), BCRYPT_HASHSIZE - 1); // Since our library automatically concats our salt and hash, we have to get the substring starting at the length of salt to jsut get the hash value
	return hash;
}

bool BCryptUtils::VerifyPassword(const std::string& password, const std::string& hash)
{
	return bcrypt_checkpw(password.c_str(), hash.c_str()) == 0;
}

