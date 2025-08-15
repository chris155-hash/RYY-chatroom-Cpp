#pragma once
#include "Singleton.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class CSession;//前置声明，解决互引用
class UserMgr: public Singleton<UserMgr>
{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	std::shared_ptr<CSession> GetSession(int uid);//通过uid查找绑定的Session
	void SetUserSession(int uid, std::shared_ptr<CSession> session);//绑定Session和uid
	void RmvUserSession(int uid, std::string session_id);//移除绑定的客户端uid和服务端的Session
private:
	UserMgr();
	std::mutex _session_mtx;
	std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_session;
};

