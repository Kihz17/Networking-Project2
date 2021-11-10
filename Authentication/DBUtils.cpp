#include "DBUtils.h"
#include "BCryptUtils.h"

#include <sstream>

// Web auth columns
uint32_t waIdColumnNumber = 1;
uint32_t waEmailColumnNumber = 2;
uint32_t waSaltColumnNumber = 3;
uint32_t waHashColumnNumber = 4;
uint32_t waUserIdColumnNumber = 5;

// user columns
uint32_t uId = 1;
uint32_t uLastLogin = 2;
uint32_t uCreationDate = 3;

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

WebResult DBUtils::AuthenticateAccount(const std::string& email, const std::string& plainTextPassword)
{
	sql::Statement* stmt = this->m_Connection->createStatement();
	try
	{
		std::stringstream ss;
		ss << "SELECT userId FROM `web_auth` WHERE email = \"" << email << "\";";
		this->m_ResultSet = stmt->executeQuery(ss.str());
		ss.str("");
		this->m_ResultSet->next();

		std::string salt = this->m_ResultSet->getString(waSaltColumnNumber);
		std::string hash = this->m_ResultSet->getString(waHashColumnNumber);
		int64_t userId = this->m_ResultSet->getInt64(waUserIdColumnNumber);

		std::string hashedPass = salt + hash;
		if (BCryptUtils::VerifyPassword(plainTextPassword, hashedPass))
		{
			printf("Login success!");

			ss << "SELECT creation_date FROM `user` WHERE userId = " << userId << ";";
			this->m_ResultSet = stmt->executeQuery(ss.str());
			this->m_ResultSet->next();
			std::string creationDate = this->m_ResultSet->getString(uCreationDate);
			return WebResult::SUCCESS;
			// TOOD: Return this info to the server
		}
		else
		{
			printf("Invalid credentials!");
			return WebResult::INVALID_CREDENTIALS;
		}
	}
	catch (SQLException e)
	{
		printf("Account does not exist!\n");
		return WebResult::INVALID_CREDENTIALS;
	}
}

WebResult DBUtils::CreateAccount(const std::string& email, const std::string& password)
{
	sql::Statement* stmt = this->m_Connection->createStatement();
	try
	{
		this->m_ResultSet = stmt->executeQuery("SELECT * FROM `web_auth` WHERE email ;");
	}
	catch (SQLException e)
	{
		printf("Failed to retrieved web_auth data!\n");
		return WebResult::INTERNAL_SERVER_ERROR;
	}

	while (this->m_ResultSet->next())
	{
		int32_t id = this->m_ResultSet->getInt(sql::SQLString("id"));
		printf("id: %d\n", id);
		int32_t id_bycolumn = this->m_ResultSet->getInt(1);
		printf("id_bycolumn(1): %d\n", id_bycolumn);

		SQLString email = this->m_ResultSet->getString("email");
		printf("email: %s\n", email.c_str());
	}


	printf("Successfully retrieved web_auth data!\n");
	return WebResult::SUCCESS;
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