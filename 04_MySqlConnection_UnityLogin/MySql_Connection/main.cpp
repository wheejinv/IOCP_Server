#include "stdafx.h"

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

int main()
{

	sql::mysql::MySQL_Driver *driver;
	sql::Connection *con;

	driver = sql::mysql::get_mysql_driver_instance();
	con = driver->connect("tcp://wheejin.com:3306", "iocp", "test");

	sql::Statement *stmt;
	sql::ResultSet *res;

	stmt = con->createStatement();

	stmt->execute("set session character_set_connection=euckr;");
	stmt->execute("set session character_set_results=euckr;");
	stmt->execute("set session character_set_client=euckr;");

	stmt->execute("USE IOCP;");
	res = stmt->executeQuery("SELECT * FROM 03_iocp_user;");

	while (res->next()) {
		cout << res->getInt(1) << " " <<
			res->getInt(2) << " " << 
			res->getInt(3)  << " " <<
			res->getString("userName").c_str() << endl;
	}

	delete con;

	return 0;
}