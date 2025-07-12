#pragma once
#include "const.h"
#include <map>

class HttpConnection; //这里是解决： A类引用B，B类引用A，互相引用的问题，头文件里声明一下对方的类，再在cpp文件里引用对方头文件。
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;//把handler封装成函数，作为后面回调函数用。后面写的lambda
//其实就是一种语法糖（简化代码写法，不影响代码功能），这里使用std::function就是lambda的底层原理。
class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {};
	bool HandleGet(std::string,std::shared_ptr<HttpConnection>);
	void RegGet(std::string,HttpHandler handler);
	void RegPost(std::string, HttpHandler handler);
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;//处理get请求的handlers集合map<pair<url,_get_handlers>>,返回url对应的handler
};


