#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// 使用 round-robin轮询 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/); //可选2倍CPU核心数
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;   //监工，因为iocontext在运行时如果没有任何需要监听的可读或可写时间就会退出，所以需要一个work对象来保持它的运行状态
	std::vector<std::thread> _threads;
	std::size_t _nextIOService;
};

