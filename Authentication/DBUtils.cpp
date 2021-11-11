#include "DBUtils.h"
#include "BCryptUtils.h"

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
		std::stringstream ss;
		ss << "SELECT salt, hashed_password, userId FROM `web_auth` WHERE email = \"" << email << "\";";
		this->m_ResultSet = stmt->executeQuery(ss.str());
		ss.str("");
		if (!this->m_ResultSet->next()) // We have no results
		{
			userId = 0;
			return auth::AuthenticateWebResult_AuthenticateResult::AuthenticateWebResult_AuthenticateResult_INVALID_CREDENTIALS;
		}

		userId = this->m_ResultSet->getInt64("userId");
		std::string salt = this->m_ResultSet->getString("salt");
		std::string hash = this->m_ResultSet->getString("hashed_password");

		std::string hashedPass = salt + hash;
		if (BCryptUtils::VerifyPassword(plainTextPassword, hashedPass))
		{
			// Get the date the account was created
			ss << "SELECT creation_date FROM `user` WHERE userId = " << userId << ";";
			this->m_ResultSet = stmt->executeQuery(ss.str());
			this->m_ResultSet->next();
			creationDate = this->m_ResultSet->getString("creation_date");

			// Update last login to now
			ss.str("");
			std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << "UPDATE `user` SET last_login = " << timestamp << " WHERE userId = " << userId << ";"; 
			stmt->executeUpdate(ss.str());

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
		std::stringstream ss;
		ss << "SELECT * FROM `web_auth` WHERE email = \"" << email << "\";";
		this->m_ResultSet = stmt->executeQuery(ss.str());
		if (this->m_ResultSet->next()) // This account already exists
		{
			return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_ACCOUNT_ALREADY_EXISTS;
		}

		ss.str("");
		std::string salt; // TODO: Generate salt
		std::string hash; // TODO: Generate hash

		// Add new user to user table

		//ss << "INSERT INTO `user` (last_login, creation_date) VALUES (-1, " << date << ");";
		if (!stmt->execute(ss.str())) // We failed yo create new user
		{
			std::cout << "Failed to insert new user into user table!" << std::endl;
			return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR;
		}

		this->m_ResultSet = stmt->executeQuery("SELECT last_insert_id() FROM user"); // This will get the ID of the user we just inserted
		uint64_t userId = this->m_ResultSet->getInt("last_insert_id()");

		// Insert data into web_auth table
		ss.str("");
		ss << "INSERT INTO `web_auth` (email, salt, hashed_password, userId) VALUES(" << email << ", " << salt << ", " << hash << ", " << userId << ");";
		if (!stmt->execute(ss.str()))
		{
			std::cout << "Failed to insert new data into the web_auth table!" << std::endl;
			return auth::CreateAccountWebResult_CreateAccountResult::CreateAccountWebResult_CreateAccountResult_INTERNAL_SERVER_ERROR;
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