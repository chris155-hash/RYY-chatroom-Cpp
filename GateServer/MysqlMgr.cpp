#include "MysqlMgr.h"

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	return _dao.RegUser(name, email, pwd);
}

MysqlMgr::MysqlMgr() {
}

MysqlMgr::~MysqlMgr() {

}