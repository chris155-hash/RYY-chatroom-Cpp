#pragma once
#include "CServer.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

class SqlConnection {    //把sql::Connection封装成了我们的SqlConnection，多加了上一次操作的时间。存储在连接池里方便心跳检测
public:
	SqlConnection(sql::Connection* con,int64_t lasttime) :_con(con),_last_oper_time(lasttime){}  //上一次操作的时间（长时间不操作，Mysql会断开连接）
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
	std::unique_ptr<SqlConnection> getConnection();
	void returnConnection(std::unique_ptr<SqlConnection> con);
	void Close();
	void checkConnection();
	~MySqlPool();

private:
	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_;  //（要连接的Mysql）
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;  //检测线程，检测超过一段时间未操作的连接（心跳机制）
};

struct UserInfo{
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

class MysqlDao {  //DAO操作层：去操作Mysql的中间层？再由MysqlMgr来调用，那为什么不直接让MysalMgr一起做，和Redis那边一样？
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	/*bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& newpwd);
	bool CheckPwd(const std::string& name, const std::string& pwd,UserInfo& userInfo);*/
private:
	std::unique_ptr<MySqlPool> pool_;
};