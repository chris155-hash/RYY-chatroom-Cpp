### 一、Qt客户端对于注册功能还要做的事：
- 1.按下确认按钮以后，将注册ui中确定按钮改为sure_btn，并为其添加click槽函数。
    - 这个槽函数要做的：
        - 1）检查用户名、密码、邮箱、验证码等都不能为空。
        - 2）密码和确认密码要一致。
        - 3）都没问题了，就给GateServer发HttpPost请求，请求的url是`gate_url_prefix+"/user_register"`,要和GateServer那边LogicSystem注册的Post处理的Reqid一致，他才能知道怎么处理对吧。
    - 当然GateServer做完相应处理之后会回给客户端一些回复，成功就前端显示`“注册成功”`，失败就显示原因，目前只显示了参数错误。邮箱或者用户名重复的报错显示在服务器，后面应该回加到这边。
### 二、GateServer对于注册功能还要做的事：
- 1.LogicSystem里注册user_register这个Reqid的处理事件到PostHandler里，在LogicSystem的构造里就插入进去。
    - 1）这些基本都一样，把收到的信息序列化出来，src_root是院士JSON数据，reader：用于解析 JSON 字符串为 Json::Value 结构；root是最后回给客户端的JSON对象。
     ```cpp
    RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
    auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    connection->_response.set(http::field::content_type, "text/json");
    Json::Value root;
    Json::Reader reader;
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
        std::cout << "Failed to parse JSON data!" << std::endl;
        root["error"] = ErrorCodes::Error_Json;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    }
    ```
    - 2）重点要做的事：读取接受的信息
    ```cpp
    //读取客户端发来的name，email、pwd和confirm等
    auto email = src_root["email"].asString();
    auto name = src_root["user"].asString();
    auto pwd = src_root["passwd"].asString();
    auto confirm = src_root["confirm"].asString();
    ```
    - 3）重点要做的事：`if (pwd != confirm)`作相应处理。为什么这里要再检查一次，Qt那里已经检查过一次了对吧？个人理解：以防万一，毕竟这么重要的事情，我后面会存到MySql数据库里，不能全依赖客户端那边，万一它崩了呢。
    - 4） 重点要做的事：先查找Redis里存储的email对应的验证码是否存在，如果该邮箱根本没有验证码，那这验证码要不是用户乱写的，要不是过期了对吧，那我们给客户端返回一个验证码过期或错误。
    - 5） 重点要做的事：查到邮箱的正确验证码之后和收到的用户填写的验证码作比较，不一样这就返回验证码错误。
    - 6)  重点要做的事：去看MySql里调用RegUser，根据返回结果，要么注册成功，将新用户信息添加进MySql数据库里，要么用户已存在，返回`用户已存在`的错误给前端。
    ```cpp
    int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
    if (uid == 0 || uid == -1) {
        std::cout << "user or email existed" << std::endl;
        root["error"] = ErrorCodes::UserExist;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    }
    ```
    这里MysqlMgr后面马上讲。
### 三、MySql
- 1.安装MySql
- 2.修改mysql密码；在本机启动mysql服务；点击服务后可查看当前计算机启动的所有服务，找到mysql，然后右键点击设为启动，同时也可设置其为自动启动和手动启动。
- 3.配置环境变量：为了方便使用mysql命令，可以将mysql目录配置在环境变量里。
- 4.为了方便测试，使用navicat等桌面工具测试连接。以后增删改查也方便。
- 5.安装Mysql Connector C++；尽管Mysql提供了访问数据库的接口，但是都是基于C风格的，为了便于面向对象设计，我们使用Mysql Connector C++ 这个库来访问mysql。
- 6.封装一个MySql连接池。做到这里和之前真是大同小异了，初始化连接池里的连接数量，提供必要的接口，Get来连接池取出连接，returnConnection归还连接，Close关闭池子，checkConnection函数（见后文）。
    - 但是有一点点区别,参见代码，连接池里的连接被我们多加了一个int_64_t的时间戳，用于记录上一次通过该连接操作MySql的时间，长时间未通过该连接操作就断开。毕竟池子的连接数量是有限的，不能让人一直占着嘛。
    - 为什么用int64_t，不是int？
        - int64_t：是固定大小的整数类型，确保在所有平台上都是 64 位，适合跨平台开发。
        - int64_t：用于需要大整数范围或精确控制内存大小的场景，例如：处理大文件偏移量（如文件系统）；高精度计时（如时间戳）；唯一 ID（如数据库主键、UUID 生成）；网络协议定义的数据结构。
    ```cpp
        class SqlConnection {    //把sql::Connection封装成了我们的SqlConnection，多加了上一次操作的时间。存储在连接池里方便心跳检测
    public:
        SqlConnection(sql::Connection* con,int64_t lasttime) :_con(con),_last_oper_time(lasttime){}  //上一次操作的时间（长时间不操作，Mysql会断开连接）
        std::unique_ptr<sql::Connection> _con;
        int64_t _last_oper_time;
    };
    ```
    - 还有一个要注意的点，超时既然要删除连接，那这需要一个线程去操作，这个我们把它写成后台线程，调用`checkConnection`来定时检查每个连接上一次操作的时间到现在是否超过了时长。
    ```cpp
        std::thread _check_thread;  //检测线程，检测超过一段时间未操作的连接
    ```
    - 在MySqlPool的构造里初始化连接池，并启动后台线程来监控超时，把这个线程detach出去，独立运行。
        ```cpp
                _check_thread = std::thread([this]() {
            while (!b_stop_) {
                checkConnection();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
            });
        _check_thread.detach();//检测线程分离出去，后台运行。由系统负责回收
        ```
- 7.封装MysQl的DAO操作层。这里其实不是特别明白，DAO操作层：去操作Mysql的中间层？再由MysqlMgr来调用，那为什么不直接和MysalMgr放一起？是为了把功能分开，减少模块之间的耦合吗，MVC架构？暂时放在这里，后面如果弄懂了再回来。
    - 1）他首先要做的事，实例化ConfigMgr，读取配置文件里的MySql服务器的地址，用户名和密码。
    - 2）其次就是提供一个向MySql数据库注册用户的接口，去调用MySql的注册用户的存储过程，然后接受MySql的返回结果，并处理异常。`int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)`，接受注册用户的用户名，邮箱和密码。传给MySql服务器，MySql的reg_user也会做相应处理。
- 8.MySql服务器那边，新建存储过程reg_user。
    - 1) 一旦插入过程遇到什么错误，直接回滚事务，保证事务的一致性。检查用户或者邮箱是否存在，存在就返回-1，都不存在说明正常，进行插入，更新user表。
    ```
        CREATE DEFINER=`root`@`%` PROCEDURE `reg_user`(
        IN `new_name` VARCHAR(255), 
        IN `new_email` VARCHAR(255), 
        IN `new_pwd` VARCHAR(255), 
        OUT `result` INT)
    BEGIN
        -- 如果在执行过程中遇到任何错误，则回滚事务
        DECLARE EXIT HANDLER FOR SQLEXCEPTION
        BEGIN
            -- 回滚事务
            ROLLBACK;
            -- 设置返回值为-1，表示错误
            SET result = -1;
        END;
        
        -- 开始事务
        START TRANSACTION;

        -- 检查用户名是否已存在
        IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
            SET result = 0; -- 用户名已存在
            COMMIT;
        ELSE
            -- 用户名不存在，检查email是否已存在
            IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
                SET result = 0; -- email已存在
                COMMIT;
            ELSE
                -- email也不存在，更新user_id表
                UPDATE `user_id` SET `id` = `id` + 1;
                
                -- 获取更新后的id
                SELECT `id` INTO @new_id FROM `user_id`;
                
                -- 在user表中插入新记录
                INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (@new_id, new_name, new_email, new_pwd);
                -- 设置result为新插入的uid
                SET result = @new_id; -- 插入成功，返回新的uid
                COMMIT;
            
            END IF;
        END IF;
        
    END
    ```
- 9.建立MySqlMgr类，功能和RedisMgr类似，这里就只是去调用MySql_DAO的reg_user操作。



