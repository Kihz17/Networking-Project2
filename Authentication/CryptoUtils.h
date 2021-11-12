#pragma once

#include <string>

class CryptoUtils
{
public:
	static std::string GenerateSalt(const std::string& input);

	static std::string HashPassword(const std::string& salt, const std::string& password);

	static bool VerifyHash(const std::string& salt, const std::string& password, const std::string& hash);
};