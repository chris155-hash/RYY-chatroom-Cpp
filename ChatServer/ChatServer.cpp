// ChatServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"
#include "const.h"

using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	auto& cfg = ConfigMgr::Inst();
	auto server_name = cfg["SelfServer"]["Name"];
	try {
		auto pool = AsioIOServicePool::GetInstance();
		//将登录数量设置为0
		RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");

		//启动一个GrpcServer(与其他ChatServer通信作为服务端用，启动一个监听)
		std::string server_address = cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"];
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		//监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		//构建并启动Grpc服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listing on :" << server_address << std::endl;
		//启动一个线程来处理Grpc服务
		std::thread grpc_server_thread([&server](){
			server->Wait();
			});

		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM,SIGBREAK);
		//直接关闭控制台窗口发送的是SIGBREAK（在Windows上）或SIGHUP（在Unix-like系统上）信号，这些信号没有被处理，所以程序无法正常清理。
		//导致不会执行后面的Redis删除命令，在Redis上服务器无法正确下线
		signals.async_wait([&io_context, pool,&server](auto, auto) {
			io_context.stop();
			pool->Stop();
			server->Shutdown();
			});
		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();//io_context::run() 就变身 事件循环工作线程。io_context内部就是 一张 epoll/kqueue/iocp 描述符表 + 一个任务队列 + 一个循环线程池，
        // 你把“异步操作”挂到它身上，它负责 等在系统调用上 → 收到事件 → 把回调扔进队列 → 调用线程池执行。

		RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);//服务器下线前将Redis里logincount表的自己名字删除，表示自己服务器不在线。这很重要，不然下次Redis就可能会返回给客户端不在线的服务器。
		RedisMgr::GetInstance()->Close();
		grpc_server_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}

