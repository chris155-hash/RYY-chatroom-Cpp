#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"


VarifyGrpcClient::VarifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VarifyServer"]["Host"];
	std::string port = gCfgMgr["VarifyServer"]["Port"];
	pool_.reset(new RPConPool(5, host, port));//unique_ptr的reset方法用于:释放当前管理的对象，并将智能指针指向一个新的对象 p。
}