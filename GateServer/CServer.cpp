#include "CServer.h"
#include "const.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc),   //引用类型的成员变量要在初始化列表初始化
_acceptor(ioc,tcp::endpoint(tcp::v4(),port)){

}

void CServer::Start() {
	auto self = shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);//新链接用线程池里的，传进去线程池里的线程（上下文）绑定新的socket
	_acceptor.async_accept(new_con->GetSocket(), [self,new_con](beast::error_code ec) {
		try{
			//出错放弃这次链接，继续监听其他链接
			if (ec) {
				self->Start();
				return;
			}
			//创建新连接，并且创建HttpConnection类管理这个链接
			new_con->Start();//新连接已经有了，直接启动即可
			//继续监听
			self->Start();
		}catch(std::exception& exp){
			std::cout << "Error is " << exp.what() << std::endl;
			self->Start();
		}
		});
}