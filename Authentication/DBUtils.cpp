#include "DBUtils.h"
#include "CryptoUtils.h"

#include <sstream>

DBUtils::DBUtils(void)
	: m_IsConnected(false)
	, m_IsConnecting(false)
	, m_Connection(0)
	, m_Driver(0)
	, m_ResultSet(0)
{
}

bool DBUtils::IsConnected(void)
{
	return m_IsConnected;
}

auth::AuthenticateWebResult_AuthenticateResult DBUtils::AuthenticateAccount(const std::string& email, const std::string& plainTextPassword, long& userId, std::string& creationDate)
{
	sql::Statement* stmt = this->m_Connection->createStatement();
	try
	{
		// Check if the account exists
		{
			std::stringstream ss;
			ss << "SELECT salt, hashed_password, userId FROM `web_auth` WHERE email = \"" << email << "\";";
			this->m_ResultSet = stmt->executeQuery(ss.str());
			if (!this->m_ResultSet->next()) // We have no results
			{
				userId = 0;
				return auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INVALID_CREDENTIALS;
			}

			userId = this->m_ResultSet->getInt64("userId");
		}
	
		std::string salt = this->m_ResultSet->getString("salt");
		std::string hash = this->m_ResultSet->getString("hashed_password");

		if (CryptoUtils::VerifyHash(salt, plainTextPassword, hash))
		{
			// Get the date the account was created
			{
				std::stringstream ss;
				ss << "SELECT creation_date FROM `user` WHERE id = " << userId << ";";
				this->m_ResultSet = stmt->executeQuery(ss.str());
				this->m_ResultSet->next();
				creationDate = this->m_ResultSet->getString("creation_date");
			}
		
			// Update last login to now
			{
				std::stringstream ss;
				ss << "UPDATE `user` SET last_login = NOW() WHERE id = " << userId << ";";
				stmt->executeUpdate(ss.str());
			}

			return auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_SUCCESS;
		}
		else
		{
			return auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INVALID_CREDENTIALS;
		}
	}
	catch (SQLException e) 
	{
		return auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INTERNAL_SERVER_ERROR;
	}
}

auth::CreateAccountWebResult_CreateAccountResult DBUtils::CreateAccount(const std::string& email, const std::string& password)
{
	sql::Statement* stmt = this->m_Connection->createStatement();
	try
	{
		// Check if the account already exists
		{
			std::stringstream ss;
			ss << "SELECT * FROM `web_auth` WHERE email = \"" << email << "\";";
			this->m_ResultSet = stmt->executeQuery(ss.str());
			if (this->m_ResultSet->next()) // This account already exists
			{
				return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_ACCOUNT_ALREADY_EXISTS;
			}
		}
		
		std::string salt = CryptoUtils::GenerateSalt(email); // Generate salt from email since it is unique (Lukas Gustafson's idea) 
		std::string hash = CryptoUtils::HashPassword(salt, password); 

		// Add new user to user table
		{
			stmt->execute("INSERT INTO `user` (last_login, creation_date) VALUES (0, NOW());");
			uint64_t updateCount = stmt->getUpdateCount();
			if (updateCount == 0) // We failed to create new user
			{
				std::cout << "Failed to insert new user into user table!" << std::endl;
				return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR;
			}
		}
		
		this->m_ResultSet = stmt->executeQuery("SELECT last_insert_id() FROM user;"); // This will get the ID of the user we just inserted
		this->m_ResultSet->next();
		uint64_t userId = this->m_ResultSet->getUInt("last_insert_id()");

		// Insert data into web_auth table
		{
			std::stringstream ss;
			ss << "INSERT INTO `web_auth` (email, salt, hashed_password, userId) VALUES(\"" << email << "\", \"" << salt << "\", \"" << hash << "\", " << userId << ");";
			stmt->execute(ss.str());
			uint64_t updateCount = stmt->getUpdateCount();
			if (updateCount == 0)
			{
				std::cout << "Failed to insert new data into the web_auth table!" << std::endl;
				return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR;
			}
		}
		

		return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_SUCCESS;
	}
	catch (SQLException e)
	{
		return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR;
	}
}

bool DBUtils::Connect(const std::string& hostname, const std::string& username, const std::string& password)
{
	if (this->m_IsConnecting)
		return true;

	this->m_IsConnecting = true;
	try
	{
		this->m_Driver = mysql::get_driver_instance();
		this->m_Connection = this->m_Driver->connect(hostname, username, password);
		this->m_Connection->setSchema("authenticationservice");
	}
	catch (SQLException e)
	{
		printf("Failed to connect to database with error: %s\n", e.what());
		this->m_IsConnecting = false;
		return false;
	}
	this->m_IsConnected = true;
	this->m_IsConnecting = false;

	printf("Successfully connected to database!\n");
	return true;
}

void DBUtils::CloseConnection()
{
	this->m_Connection->close();
}