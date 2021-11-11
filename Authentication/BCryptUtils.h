#pragma once

#include <bcrypt/BCrypt.hpp>

#include <string>

class BCryptUtils
{
public:
	static bool GenerateSalt(char* salt, int workload = 12);

	static std::string GenerateHash(const std::string& password, char* salt);

	static bool VerifyPassword(const std::string& password, const std::string& hash);
};