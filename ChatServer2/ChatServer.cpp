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
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool,&server](auto, auto) {
			io_context.stop();
			pool->Stop();
			server->Shutdown();
			});
		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();

		RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
		RedisMgr::GetInstance()->Close();
		grpc_server_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}

