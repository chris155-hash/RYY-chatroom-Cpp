#include "CServer.h"
#include "const.h"
#include "HttpConnection.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc),   //引用类型的成员变量要在初始化列表初始化
_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc) {

}

void CServer::Start() {
	auto self = shared_from_this();
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try{
			//出错放弃这次链接，继续监听其他链接
			if (ec) {
				self->Start();
				return;
			}
			//创建新连接，并且创建HttpConnection类管理这个链接
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();//socket没有默认构造，所以这里转成右值再移动构造
			//继续监听
			self->Start();
		}catch(std::exception& exp){
			std::cout << "Error is " << exp.what() << std::endl;
			self->Start();
		}
		});
}