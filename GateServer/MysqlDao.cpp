#include "MysqlDao.h"
#include "ConfigMgr.h"

MySqlPool::MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
	: url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false) {
	try {
		for (int i = 0; i < poolSize_; ++i) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* con = driver->connect(url_, user_, pass_);
			con->setSchema(schema);
			//获取当前时间戳
			auto currentTime = std::chrono::system_clock::now().time_since_epoch();
			//将时间戳转换为秒
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
			pool_.push(std::make_unique<SqlConnection>(con,timestamp));
		}
		_check_thread = std::thread([this]() {
			while (!b_stop_) {
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
			});
		_check_thread.detach();//检测线程分离出去，后台运行。由系统负责回收
	}
	catch (sql::SQLException& e) {
		// 处理异常
		std::cout << "mysql pool init failed" << std::endl;
	}
}

void MySqlPool::checkConnection() {
	std::lock_guard<std::mutex> guard(mutex_);
	int poolsize = pool_.size();
	//获取当前时间戳
	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	//将时间戳转换为秒
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	for (int i = 0;i < poolsize;i++) {     //只能遍历，queue没有迭代器，只能pop出来判断完再push回去
		auto con = std::move(pool_.front());
		pool_.pop();
		Defer defer([this, &con]() {  //这里仿照了go语言的Defer思想。取出来最后还要插回到queue，写在前面。类似RAII，}结束前调一次它
			//接受一个lambda或函数指针，function包装一下。Defer的析构函数里调用一下该函数。局部变量每次循环完自然要析构一次，那就等于调用了他。
			pool_.push(std::move(con));
			});
		if (timestamp - con->_last_oper_time < 5) {  //超过5秒（测试用，后面可以改成300s或者600s）
			continue;
		}
		try {
			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			stmt->executeQuery("SELECT 1");
			con->_last_oper_time = timestamp;
			//std::cout << "execute timer alive query , cur is " << timestamp << std::endl;
		}
		catch (sql::SQLException& e) {
			std::cout << "Error keeping connection alive: " << e.what() << std::endl;
			// 重新创建连接并替换旧的连接
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* newcon = driver->connect(url_, user_, pass_);
			newcon->setSchema(schema_);
			con->_con.reset(newcon);
			con->_last_oper_time = timestamp;
		}
	}
};

std::unique_ptr<SqlConnection> MySqlPool::getConnection() {
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this] {
		if (b_stop_) {
			return true;
		}
		return !pool_.empty(); });
	if (b_stop_) {
		return nullptr;
	}
	std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
	pool_.pop();
	return con;
}

void MySqlPool::returnConnection(std::unique_ptr<SqlConnection> con) {
	std::unique_lock<std::mutex> lock(mutex_);
	if (b_stop_) {
		return;
	}
	pool_.push(std::move(con));
	cond_.notify_one();
}

void MySqlPool::Close() {
	b_stop_ = true;
	cond_.notify_all();
}

MySqlPool::~MySqlPool() {
	std::unique_lock<std::mutex> lock(mutex_);
	while (!pool_.empty()) {
		pool_.pop();
	}
}

MysqlDao::MysqlDao()
{
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];  //记得之前写ConfigMgr时候的一级Key和二级Key嘛
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));  //pool是unique_ptr，参数要么在初始化列表里，要么reset重置裸指针再交由独占指针管理
}

MysqlDao::~MysqlDao()
{
	pool_->Close();//和Redis那里类似，这个close会先调用池子的close
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)     //从连接取连接，调用MySql的reg_user方法，接受返回结果@result
{
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;//返回false，不太对int类型，应该是-1？
		}
		//准备调用存储过程。创建一个预编译的 SQL 语句，调用存储过程 reg_user
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		//设置输入参数
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		//由于PrepareStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		 //执行存储过程。执行预编译的 SQL 语句，即调用存储过程 reg_user。
		stmt->execute();
		 //如果存储过程设置了会话变量或有其他方式获取输出参数的值，可以在这里执行SELECT查询来获取他们
		//例如：如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));  //executeQuery 方法用于执行查询并返回结果集。
		if (res->next()) {  //查询有结果就返回查询结果
			int result = res->getInt("result");
			std::cout << "Result is :" << result << std::endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		//不然就是没有查询结果，返回-1表示注册失败
		pool_->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();     //e.what() 返回异常的详细描述字符串。
		std::cerr << " (MySQL error code: " << e.getErrorCode();    //e.getErrorCode() 返回 MySQL 数据库的错误码，用于定位具体的数据库问题。
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;    //e.getSQLState() 返回符合 ANSI SQL 标准的错误状态码.
		return -1;  //表示注册失败
	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		//准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));
		//设置输入参数
		pstmt->setString(1, name);
		//执行查询--查询用户name对应的真正邮箱给res
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		//遍历结果集
		while (res->next()) {
			std::cout << "Check Email:" << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				pool_->returnConnection(std::move(con));
				return false;
			}
			pool_->returnConnection(std::move(con));
			return true;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();     //e.what() 返回异常的详细描述字符串。
		std::cerr << " (MySQL error code: " << e.getErrorCode();    //e.getErrorCode() 返回 MySQL 数据库的错误码，用于定位具体的数据库问题。
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;    //e.getSQLState() 返回符合 ANSI SQL 标准的错误状态码.
		return false;  //表示注册失败
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		//准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));
		//设置输入参数
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		//执行数据库里的用户对应密码更新
		int updateCount = pstmt->executeUpdate();
		std::cout << "Update rows:" << updateCount << std::endl;
		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();     //e.what() 返回异常的详细描述字符串。
		std::cerr << " (MySQL error code: " << e.getErrorCode();    //e.getErrorCode() 返回 MySQL 数据库的错误码，用于定位具体的数据库问题。
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;    //e.getSQLState() 返回符合 ANSI SQL 标准的错误状态码.
		return false;  //表示注册失败
	}
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});
	try {
		//准备Sql语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));//prepareStatement预编译
		pstmt->setString(1,email);

		//执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		//遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			//输出查询到的密码
			std::cout << "Password is :" << origin_pwd << std::endl;
			break;
		}
		if (pwd != origin_pwd) {
			return false;
		}

		userInfo.name = res->getString("name");
		userInfo.email = email;
		userInfo.uid = res->getInt("uid");
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();     //e.what() 返回异常的详细描述字符串。
		std::cerr << " (MySQL error code: " << e.getErrorCode();    //e.getErrorCode() 返回 MySQL 数据库的错误码，用于定位具体的数据库问题。
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;    //e.getSQLState() 返回符合 ANSI SQL 标准的错误状态码.
		return false;  //表示登录失败
	}
}

