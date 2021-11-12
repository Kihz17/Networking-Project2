#include "CryptoUtils.h"

#include <cryptopp/cryptlib.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace CryptoPP;

std::string CryptoUtils::GenerateSalt(const std::string& input)
{
    // Hash our input
    byte derived[SHA256::DIGESTSIZE];
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
    byte unused = 0;
    pbkdf.DeriveKey(derived, sizeof(derived), unused, (const byte*) input.c_str(), input.length(), (const byte*) "", 0, 1024, 0.0f);

    // Convert that hash into a hex string
    std::string result;
    HexEncoder encoder(new StringSink(result));
    encoder.Put(derived, sizeof(derived));
    encoder.MessageEnd();

    return result;
}

std::string CryptoUtils::HashPassword(const std::string& salt, const std::string& password)
{
    // Hash our input
    byte derived[SHA256::DIGESTSIZE];
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
    byte unused = 0;
    pbkdf.DeriveKey(derived, sizeof(derived), unused, (const byte*)password.c_str(), password.length(), (const byte*) salt.c_str(), salt.length(), 1024, 0.0f);

    // Convert that hash into a hex string
    std::string result;
    HexEncoder encoder(new StringSink(result));
    encoder.Put(derived, sizeof(derived));
    encoder.MessageEnd();

    return result;
}

bool CryptoUtils::VerifyHash(const std::string& salt, const std::string& password, const std::string& hash)
{
    std::string hashToVerify = HashPassword(salt, password);
    return hashToVerify == hash;
}