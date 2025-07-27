#include "HttpConnection.h"
#include "LogicSystem.h"
HttpConnection::HttpConnection(boost::asio::io_context& ioc): _socket(ioc){

}
void HttpConnection::Start() {
	auto self = shared_from_this();
	http:async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transferred) {
	try {
		if (ec) {  //friend bool operator==( const error_code & code, const error_condition & condition )这是源码，重载了==运算符，可以转换为bool类型
			std::cout << "http read err is" << ec.what() << std::endl;
			return;
		}
		boost::ignore_unused(bytes_transferred);//告诉编译器忽略 bytes_transferred 这个可能未使用的变量。
		self->HandleReq();
		self->CheckDeadline();
	}
	catch (std::exception& exp) {
		std::cout << "exception is " << exp.what() << std::endl;
	}
	});
}

//为了解析url请求增加的进制转换.十进制《--》十六进制
unsigned char ToHex(unsigned char x) {
	return x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x) {
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)//Url编码方式
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//判断是否仅有数字和字母构成
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //为空字符
			strTemp += "+";
		else
		{
			//其他字符需要提前加%并且高四位和低四位分别转为16进制.汉字分成高四位和低四位，转换成16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);//先取出高四位
			strTemp += ToHex((unsigned char)str[i] & 0x0F);//& 0x0F 可以取低四位
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//还原+为空
		if (str[i] == '+') strTemp += ' ';
		//遇到%将后面的两个字符从16进制转为char再拼接
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpConnection::PreParseGetParam() {
	//提取Url   get_test?key1=value1&key2=value2
	auto url = _request.target();
	//查找查询字符串的开始位置（即‘？’的位置）
	auto query_pos = url.find('?');
	if (query_pos == std::string::npos) {
		return;
	}//处理问号前的url
	_get_url = url.substr(0, query_pos);
	std::string query_string = url.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eqpos = pair.find('=');
		if (eqpos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eqpos));
			value = UrlDecode(pair.substr(eqpos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}//以&为分界线处理每一段key和value
	//最后一段参数对key-value没有&分界线，单独处理
	if (!query_string.empty()) {
		size_t eqps = query_string.find('=');
		if (eqps != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eqps));
			value = UrlDecode(query_string.substr(eqps + 1));
			_get_params[key] = value;
		}
	}
}

void HttpConnection::HandleReq() {    //底层处理，LogicSyetem是上层逻辑层处理请求。这里不管怎么样都要解析并确定回包的头部内容。
	//设置版本
	_response.version(_request.version());//回包版本号
	_response.keep_alive(false);//http不维持长连接
	if (_request.method() == http::verb::get) {//处理http的GET请求
		PreParseGetParam();
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
		if (!success) { //处理失败
			_response.result(http::status::not_found);//http状态
			_response.set(http::field::content_type, "text/plain");//内容类型
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		_response.result(http::status::ok);
		_response.set(http::field::sec_websocket_version, "GateServer");//可以回一下是哪个服务器回的
		WriteResponse();
		return;
	}

	if (_request.method() == http::verb::post) {//处理http的PosT请求
		PreParseGetParam();
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
		if (!success) { //处理失败
			_response.result(http::status::not_found);//http状态
			_response.set(http::field::content_type, "text/plain");//内容类型
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		_response.result(http::status::ok);
		_response.set(http::field::sec_websocket_version, "GateServer");//可以回一下是哪个服务器回的
		WriteResponse();
		return;
	}
}


void HttpConnection::WriteResponse() {
	auto self = shared_from_this();
	_response.content_length(_response.body().size());//http把包体长度放进包头，解决粘包
	http:async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) {
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);
		self->deadline_.cancel();//触发了回调就取消定时器
	});
}

void HttpConnection::CheckDeadline() {
	auto self = shared_from_this();
	deadline_.async_wait([self](beast::error_code ec) {
		if (!ec) {
			self->_socket.close();//超时强制关闭这个socket
		}
		});
}