### 一、Qt客户端按下登录按钮后的事情
- 1.为登录按钮添加槽函数：检查邮箱、密码是否符合格式要求
    -1） 因为这个`登录`按钮是直接用的Qt的QPushButton，所以我们只用写槽函数，不用写`触发信号`和`连接`；
    -2） 这个槽函数先执行两个函数，一个`checkEmailValid()`，一个`checkPasswd()`,相当于在前端先做一次拦截。当然可以不管是否合法，都去MySql查询是否符合正确密码，但是这样不是平白增加MySql的压力嘛，如果高并发的情况下呢。这两个检查函数和注册界面逻辑一致，这里不重复了。
    -3） 发送Post请求，构造Json对象，我们Qt和GateServer之间发送的都是Json序列化的格式，之前提到过。然后使用单例模式取出HttpMgr的对象调用PostReq方法，逻辑层相当于结束了。后面就是HttpMgr底层的处理了，
    ```cpp
     //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
    ```
- 2.上面处理完发送了，下面处理接受逻辑。
    - 1）HttpMgr发送完Post请求后当然会等着接受回包，这些在第一天的`HttpMgr`里都实现了。这边等到服务器以后就会触发`slot_http_finish`槽函数，通知指定模块处理，这里就新增通知上面Post请求里的`ReqId::ID_LOGIN_USER,Modules::LOGINMOD`，也就是登录模块处理。
    - 2）登录模块怎么处理呢？1.构造里连接HttpMgr的`sig_http_finish`和`slot_http_finish`。2.具体处理：首先处理错误情况，是服务器拒绝之类的就显示`网络错误`，是自己的HttpMgr序列化失败，就显示`Json解析错误`等。这些和验证码回来之后的处理逻辑基本一致，处理通用错误；3.然后上面没问题就调用自己的特殊处理，根据ReqId执行不同处理，处理不同的Http对应不同处理（类比服务器端，处理注册的逻辑还是登录的逻辑）。那向`_handlers`这个map里添加Id和处理逻辑。4.这个逻辑也很简单，就是失败就显示`参数错误`,成功就显示`登陆成功`，然后发送Tcp长连接请求（要去和ChatServer建立长连接了）。
### 二、GateServer的处理/user_login的逻辑
- 1.HttpMgr层面的处理自不用多说，不用管。
- 2.LogicSystem增加对`/user_login`的逻辑处理。
    - 1）序列化等通用处理不说了；去Mysql层调用`CheckPwd`函数去Mysql服务器里检查用户邮箱的密码和用户填写的密码是否一致；
    - 2）那就要去`MySqlMgr--MySql_Dao->`增加`CheckPwd`函数，之前安装了`MySqlConnector C++`,这里简单看一下：
    ```cpp
        //准备Sql语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
        pstmt->setString(1,email);
        //执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    ```
    - 3）GateServer作为StatusServer的客户端向StatusServer通过Grpc调用StatusServer的服务，请求返回合适的ChatServer的uid，token。
- 这里讲一下为什么要去StatusServer，因为我们的项目不止一个ChatServer，所以每当由用户来了以后，我们会考虑多个服务器之间的负载均衡，会返回给客户端当下压力最小的那个服务器。这个操作就是由StatusServer来当中间人。
- 3.既然使用Grpc去服务，服务器之间的序列化我们采用的是Protobuf的序列化格式，先修改Proto文件。
    - 1）新增的GetChatServer的服务，和交互的消息格式，添加之后还需要重新编译proto message的cpp文件和grpc文件，来生成修改后的Grpc服务的文件。
    ```
        message GetChatServerReq {
        int32 uid = 1;
        }

        message GetChatServerRsp {
        int32 error = 1;
        string host = 2;
        string port = 3;
        string token = 4;
        }

        service StatusService {
            rpc GetChatServer (GetChatServerReq) returns (GetChatServerRsp) {}
        }
    ```
    - 2）StatusClient类，GateServer作为StatusServer的客户端调用Grpc服务，和VarifyServer的那个VarifyClient类是一个道理。看下它的实现文件就明白了：1.构造函数里用之前写的Grpc的连接池，初始化连接池。2.GateServer调用GetChatServer函数。从连接池取出连接---Rpc服务的stub客户端存根（连接池里Create channel，提供stub供使用）。这里用了go语言的defer思想，变量接受一个函数作为对象，变量的析构函数会调用传入的函数。所以函数return前都会执行一下defer传入的函数，这里就是向连接池里归还取出的连接。
    ```cpp
        GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
        {
            ClientContext context;
            GetChatServerRsp reply;
            GetChatServerReq request;
            request.set_uid(uid);
            auto stub = pool_->getConnection();
            Status status = stub->GetChatServer(&context, request, &reply);
            Defer defer([&stub, this]() {
                pool_->returnConnection(std::move(stub));
                });
            if (status.ok()) {	
                return reply;
            }
            else {
                reply.set_error(ErrorCodes::RPCFailed);
                return reply;
            }
        }

        StatusGrpcClient::StatusGrpcClient()
        {
            auto& gCfgMgr = ConfigMgr::Inst();
            std::string host = gCfgMgr["StatusServer"]["Host"];
            std::string port = gCfgMgr["StatusServer"]["Port"];
            pool_.reset(new StatusConPool(5, host, port));
        }
    ```
### 三、StatusServer的构建
- 目光很自然地转向了StatusServer，大部分代码都服用了GateServer的代码。主从Reactor模型，和Gateserver一样，主线程只负责建立连接，当有新连接到来时，从线程池取出一个线程context，实例化一个HttpConnection类来监听socket，并绑定到取出的线程上，后续异步监听可读可写事件，由该线程执行绑定的HttpConnection的回调函数。
- 主程序里添加
    ```cpp
    StatusServiceImpl service;
    ```
    - 这个StatusServiceImpl就和CServer一样，StatusServer主要做的事。看下他的头文件：
    ```cpp
    public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
		GetChatServerRsp* reply) override;
	Status Login(ServerContext* context, const LoginReq* request,
		LoginRsp* reply) override;
    private:
        void insertToken(int uid, std::string token);
        ChatServer getChatServer();
        std::unordered_map<std::string, ChatServer> _servers;
        std::mutex _server_mtx;
    ```
- 1）这个`_servers`就是一个哈希表，key就是ChatServer的name，value就是ChatServer的地址。这个map在构造函数里通过读取配置文件被读取，`GetChatServer`函数就是Grpc的版本，返回类型Status是grpc定义的；实际上还是去调用private的这个GetChatServer；`GetChatServer`这个是遍历map里的服务器name，再去Redis里根据name查找负载，比较之后最后返回一个负载最小的ChatServer地址和token。
- 2）这里可能理解还不是很深，后续可能会在补充。

    
