#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"

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
	//注册post请求
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
			std::cout << "Failed to parsr Json data" << std::endl;
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

