#pragma once
#include <grpcpp/grpcpp.h>
#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include <atomic>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool {
public:
	RPConPool(size_t poolsize, std::string host, std::string port):
	poolSize_(poolsize),host_(host),port_(port),b_stop_(false){
		for (size_t i = 0;i < poolsize;i++) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());
			connections_.push(VarifyService::NewStub(channel));//queue的push在C++14后提供了移动重载，把临时unique_ptr直接“挪”进去，而不用显式 std::move
		}
	}
	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();//通知线程不用再等了，我要关闭连接池了；如果已经加锁的线程那我要等他执行完再close
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}
	std::unique_ptr<VarifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this]() {
			if (b_stop_) return true;
			return !connections_.empty();  //lambda返回false就会释放锁，然后wait在这里，等待notify这类的通知，然后再重新判断，知道条件为true才会加锁继续执行。
			});
		if (b_stop_) {
			return nullptr;
		}
		auto context = std::move(connections_.front());//connections里的stub是unique_ptr类型，不能拷贝，只能移动。
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) return;  //如果连接池要关闭，那也没必要放回连接池了
		connections_.push(std::move(context));
		cond_.notify_one(); 
	}

private:
	std::atomic<bool> b_stop_;  //连接池是否停止
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VarifyService::Stub>> connections_;  //这里就是连接池本体，里面存的是stub，就是Grpc的信使，提供的就是unique_ptr类型。unique_ptr类型不可拷贝、只可移动；
	std::condition_variable cond_;
	std::mutex mutex_;
};

class VarifyGrpcClient: public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyReq request;
		GetVarifyRsp reply;
		request.set_email(email);
		auto stub = pool_->getConnection();
		Status status = stub->GetVarifyCode(&context, request, &reply);//去看GetVarifyCode，一、三参数是指针，二是引用
		if (status.ok()) {
			pool_->returnConnection(std::move(stub));//只要动stub（uique_ptr）就要move
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			pool_->returnConnection(std::move(stub));
			return reply;
		}
	}
private:
	VarifyGrpcClient();
	
	std::unique_ptr<RPConPool> pool_;
};

