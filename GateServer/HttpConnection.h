#pragma once
#include "const.h"

class HttpConnection: public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicSystem;
	HttpConnection(boost::asio::io_context & ioc);
	void Start();
	tcp::socket& GetSocket() {
		return _socket;
	}
private:
	void PreParseGetParam();//解析Get请求Url的函数
	void CheckDeadline();//java/go等的http都封装好了超时检测。C++这里我们也封装一个，超时就判定为掉线
	void WriteResponse();//应答函数,三次握手，四次挥手等的应答
	void HandleReq();//解析包体内容
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };//先设置8K的缓冲区大小，MTU大概1500字节左右
	http::request<http::dynamic_body> _request;//请求的内容，可以是图片、文本等
	http::response<http::dynamic_body> _response;//回复的
	net::steady_timer deadline_{ //定时器
		_socket.get_executor(),std::chrono::seconds(60)  //超过60秒认为超时
	};
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

