#pragma once

#include <mysql/jdbc.h>

#include <string>

using namespace sql;

enum WebResult
{
	SUCCESS,
	INVALID_CREDENTIALS,
	ACCOUNT_ALREADY_EXISTS,
	INVALID_PASSWORD,
	INTERNAL_SERVER_ERROR
};

class DBUtils
{
public:
	DBUtils();

	bool Connect(const std::string& hostname, const std::string& username, const std::string& password);
	bool IsConnected();

	// SELECT = sql::Statement::executeQuery()
	// UPDATE = and sql::Statement::executeUpdate()
	// INSERT = sql::Statement::execute()

	WebResult AuthenticateAccount(const std::string& email, const std::string& plainTextPassword);

	WebResult CreateAccount(const std::string& email, const std::string& plainTextPassword);

	void CloseConnection();
private:
	mysql::MySQL_Driver* m_Driver;
	Connection* m_Connection;
	ResultSet* m_ResultSet;
	bool m_IsConnected;
	bool m_IsConnecting;
};