### 一、线程池(多个线程供新连接使用来建立HttpConnection，绑定监听——socket和线程上下文io_context)。
- 1.这里就直接复用一部网络编程里的线程池了，导入即可。日志里我就直接粘贴坐着代码了，线程池的学习资料很多，再次不多做解释了。
    ```cpp
    #include "AsioIOServicePool.h"
    #include <iostream>
    using namespace std;
    AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioServices(size),
    _works(size), _nextIOService(0){
        for (std::size_t i = 0; i < size; ++i) {
            _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
        }

        //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
        for (std::size_t i = 0; i < _ioServices.size(); ++i) {
            _threads.emplace_back([this, i]() {
                _ioServices[i].run();
                });
        }
    }

    AsioIOServicePool::~AsioIOServicePool() {
        Stop();
        std::cout << "AsioIOServicePool destruct" << endl;
    }

    boost::asio::io_context& AsioIOServicePool::GetIOService() {
        auto& service = _ioServices[_nextIOService++];
        if (_nextIOService == _ioServices.size()) {
            _nextIOService = 0;
        }
        return service;
    }

    void AsioIOServicePool::Stop(){
        //因为仅仅执行work.reset并不能让iocontext从run的状态中退出
        //当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
        for (auto& work : _works) {
            //把服务先停止
            work->get_io_context().stop();
            work.reset();
        }

        for (auto& t : _threads) {
            t.join();
        }
    }
    ```
### 二、连接池（多个Stub-Grpc的信使Channel，供GateServer来取Stub去调用VerifyServer的Rpc服务）。
```cpp
class RPConPool {
public:
	RPConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {
			
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
				grpc::InsecureChannelCredentials());

			connections_.push(VarifyService::NewStub(channel));
		}
	}

	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	std::unique_ptr<VarifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		//如果停止则直接返回空指针
		if (b_stop_) {
			return  nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(std::move(context));
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
```