#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/filesystem.hpp>//用于读取文件
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <hiredis.h>
#include <cassert>



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json解析错误
	RPCFailed = 1002,   //RPC请求错误
	VarifyExpired = 1003,  //验证码过期
	VarifyCodeErr = 1004,  //验证码错误
	UserExist = 1005,    //用户已存在
	PasswdErr = 1006,    //密码错误
	EmailNotMatch = 1007,    //邮箱不匹配
	PasswdUpFailed = 1008,   //更新密码失败
	PasswdInvalid = 1009,    //密码更新失败
	RPCGetFailed = 1010,     //获取RPC请求失败
};

//Defer类
class Defer {
public:
	//接受一个lambda函数表达式或者函数指针
	Defer(std::function<void()> func) : func_(func) {}

	//析构函数中执行传入的函数
	~Defer() {
		func_();
	}
private:
	std::function<void()> func_;
};

#define CODEPREFIX "code_"    //redis那里存储的邮箱和验证码的邮箱格式是：_code + 邮箱