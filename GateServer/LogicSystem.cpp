#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url,handler));
}
void LogicSystem::RegPost(std::string url, HttpHandler handler) {
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem() {
	//注册get请求
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req" << std::endl;
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << "key is " << elem.first << ",";
			beast::ostream(connection->_response.body()) << "param" << i << "value is " << elem.second << std::endl;
		}
	});
	//注册post请求（获取验证码的、用户注册的···等等）
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receiver body is :" << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;//返回给对方的Json结构
		Json::Reader reader;//json的处理，做反序列化的
		Json::Value src_root;//解析的来源json
		bool parse_success = reader.parse(body_str, src_root);//把接受到的字节流转成json
		if (!parse_success) {
			std::cout << "Failed to parsr Json data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();//序列化以后的Json串
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;//还是返回true，只有重大情况才返回false，一般会断开连接
		}
		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse Json data" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();//序列化以后的Json串
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//没问题就增加逻辑：发送给验证服务器，此时GateServer相当于客户端，Varify服务器是服务端
		auto email = src_root["email"].asString();
		GetVarifyRsp rsp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);
		std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});

	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//读取客户端发来的name，email、pwd和confirm等
		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();
		//虽然前端校验了密码和确认密码，这里服务器还是在确认一次比较好。更安全，这种事不能只交给客户端。
		if (pwd != confirm) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//先查找Redis里存储的email对应的验证码是否存在
		std::string varify_code;  //去Redis里根据邮箱查找键--验证码
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!b_get_varify) {         //获取失败，验证码过期了，
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//另一种验证码错误：查到的验证码varify_code与用户提交的src_root["varifycode"]不一致
		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		
		//查找Mysql看用户是否存在
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
		if (uid == 0 || uid == -1) {
			std::cout << "user or email existed" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//暂时先都返回客户端
		root["error"] = 0;
		root["email"] = src_root["email"];
		root["user"] = src_root["user"].asString();
		root["passwd"] = src_root["passwd"].asString();
		root["confirm"] = src_root["confirm"].asString();
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
}


bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}
	_get_handlers[path](con);//注意这里找到了对应的回调就执行然后返回true，调用成功
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}
	_post_handlers[path](con);
	return true;
}

