#pragma once

#include <mysql/jdbc.h>

#include <Auth.pb.h>

#include <string>

using namespace sql;

class DBUtils
{
public:
	DBUtils();

	bool Connect(const std::string& hostname, const std::string& username, const std::string& password);
	bool IsConnected();

	// SELECT = sql::Statement::executeQuery()
	// UPDATE = and sql::Statement::executeUpdate()
	// INSERT = sql::Statement::execute()

	auth::AuthenticateWebResult_AuthenticateResult AuthenticateAccount(const std::string& email, const std::string& plainTextPassword, long& userId, std::string& creationDate);

	auth::CreateAccountWebResult_CreateAccountResult CreateAccount(const std::string& email, const std::string& plainTextPassword);

	void CloseConnection();
private:
	mysql::MySQL_Driver* m_Driver;
	Connection* m_Connection;
	ResultSet* m_ResultSet;
	bool m_IsConnected;
	bool m_IsConnecting;
};