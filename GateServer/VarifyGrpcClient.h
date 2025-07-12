#pragma once
#include <grpcpp/grpcpp.h>
#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;


class VarifyGrpcClient: public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyReq request;
		GetVarifyRsp reply;
		request.set_email(email);

		Status status = stub_->GetVarifyCode(&context, request, &reply);//去看GetVarifyCode，一、三参数是指针，二是引用
		if (status.ok()) {
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}
private:
	VarifyGrpcClient() {
		std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051",
			grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);
	};
	std::unique_ptr<VarifyService::Stub> stub_;//相当于一个信使，用它进行通信
};

