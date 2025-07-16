### 一、封装redis操作类(这里其实是分两步的，第一步不使用连接池，单连接，context和connect都是直接指定的；第二部封装了Redis连接池从连接池取连接用。后面代码分析都是第二步的，Redis的构造等都会有相应修改)
- 目的:hredis提供的操作不易使用，我们手动封装redis操作类，来简化操作流程。
- 1.封装一个RedisMgr类来管理Redis操作
    - 这里直接贴出头文件，对照着来看
    ```cpp
        class RedisMgr: public Singleton<RedisMgr>, 
        public std::enable_shared_from_this<RedisMgr>
    {
        friend class Singleton<RedisMgr>;
    public:
        ~RedisMgr();
        bool Get(const std::string &key, std::string& value);
        bool Set(const std::string &key, const std::string &value);
        bool Auth(const std::string &password);
        bool LPush(const std::string &key, const std::string &value);
        bool LPop(const std::string &key, std::string& value);
        bool RPush(const std::string& key, const std::string& value);
        bool RPop(const std::string& key, std::string& value);
        bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);
        bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
        std::string HGet(const std::string &key, const std::string &hkey);
        bool Del(const std::string &key);
        bool ExistsKey(const std::string &key);
        void Close();
    private:
        RedisMgr();

        redisContext* _connect;
        redisReply* _reply;
    };
    ```
    - 1）首先还是让它成为单例模式，节省内存资源，保证数据一致性。那构造函数设为私有，只依靠提供的单例全局访问点来访问。析构公有，因为智能指针析构可能会用到。
    ```cpp
        RedisMgr::RedisMgr()
    {
        auto& gCfgMgr = ConfigMgr::Inst();
        auto host = gCfgMgr["Redis"]["Host"];
        auto port = gCfgMgr["Redis"]["Port"];
        auto pwd = gCfgMgr["Redis"]["Passwd"];
        _con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
    }
    ```
    构造函数里我们直接让他读取configMgr这个我们之前封装好的读取配置得类，读取Redis服务器的IP+Port、密码等初始化连接池，里面启动五个连接。
    析构简单，就不贴了，调用RedisMgr的Close，这个Close就是调用连接池的close就行了；它的close第二节再讲。
    - 2）私有成员变量：调用的Redis连接的上下文redisContext*、接受的Redis的回应redisReply*
    - 3) 一系列操作Redis的函数，Auth验证身份，Get获取Key对应的value，Set设置Key对应的value。这种适用于简单的字符串键值对。但RedisRedis不仅仅是一个简单的键值存储系统，它还支持更复杂的数据结构，如列表（List）、集合（Set）、有序集合（Sorted Set）和哈希表（Hash）。LPUSH、RPUSH等命令就是用于操作这些复杂数据结构的。
    - 4）有了LPush、LPop、RPush、RPop等就可以实现复杂的结构，LPUSH和RPop就可以组合成栈等等。
    - 5）HSet、HSet和HGet等可以操作二级map，一级key是key，比如website，二级key是hkey，比如bilibili，然后是value，比如我的主页。删除Del操作、检测key是否存在等。Hset提供了两个版本：第一个版本使用 redisCommand 格化发送 HSET 命令。第二个版本使用 redisCommandArgv 手动构造参数数组发送命令，支持二进制安全的 hvalue。--第一个版本依赖字符串的结束符来判断参数的结束位置，第二个版本Redis 会严格按照提供的长度来处理数据，不会依赖字符串的结束符 \0。
    - 6）拿一个来看一下
        ```cpp
            //以下的SET、GET等的if-else处理错误情况并释放资源等，看公司要求处理。也可以自己写一下RAII，析构释放。
        bool RedisMgr::Set(const std::string& key, const std::string& value) {//执行redis命令行
            auto connect = _con_pool->getConnection();
            if (connect == nullptr) {
                return false;
            }
            auto _reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
            if (NULL == _reply)  //如果返回NULL则说明执行失败
            {
                std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
                freeReplyObject(_reply);
                return false;
            }

            //如果执行失败则释放连接
            if (!(_reply->type == REDIS_REPLY_STATUS && (strcmp(_reply->str, "OK") == 0 || strcmp(_reply->str, "ok") == 0)))
            {
                std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
                freeReplyObject(_reply);
                return false;
            }

            //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(_reply);
            std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
            return true;
        }
        ```
    这个Set做的事情：1.从连接池中取连接，失败直接返回false；取到连接后去调用真正的Redis的Set接受返回的响应。失败则释放资源，打印错误；成功就打印成功信息，释放资源。释放资源的操作也可以都放到析构中去，RAII思想。但个人建议Redis操作不要这样做。
    - 7）总结：这些操作
        - 1.增加了错误处理，及时报错方便我们定位到错误，增加了代码的健壮性。   
        - 2.使用我们建立的的Redis连接池，从连接池取连接使用，没有连接则挂起等待。一来保证线程安全，二来节省开销。
        - 3.调用原始的Redis相应接口，相当于屏蔽了hredis的繁琐细节。这正是封装这一面向对象思想的生动体现。
### 二、封装redis连接池
- 1.头文件拿来看看
    ```cpp
        class RedisConPool {
    public:
        RedisConPool(size_t poolSize, const char* host, int port, const char* pwd);
        ~RedisConPool();
        redisContext* getConnection();
        void returnConnection(redisContext* context);
        void Close();
    private:
        std::atomic<bool> b_stop_;
        size_t poolSize_;
        const char* host_;
        int port_;
        std::queue<redisContext*> connections_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };
    ```
    - 这里其实和之前的线程池。连接池没什么区别。初始化连接池，加锁取连接，归还链接，关闭连接池等。注意解锁保证线程安全，条件变量通知来取连接等即可。
    - 构造函数就是初始化连接池的过程，接收参数：连接池的连接数量、Redis服务器的地址，密码。初始化的时候就要进行密码验证。
    - close函数把b_stop_变量置为true，通知所有在等待的线程不用等了。
    - 析构函数把连接池 connections 的元素都弹出释放。
### 三、遇到的小错误
- 报错在文件夹里，写的时候这一句
    ```cpp
    std::unique_ptr<RedisConPool> _con_pool;
    ```
    是RedisMgr的私有成员变量，调用RedisConPool初始化连接池。同时用独占指针来进行管理，这样安全些，毕竟是单例，一时间只让一个线程来持有操作权。
    - 报错：类 "std::unique_ptr<<错误类型>, std::default_delete<<错误类型>>>" 不存在默认构造函数
    后面不知为何，先编译运行后报错七八十个。仔细一看都是由连接池未成功初始化导致，后面RedisMgr的操作中的context和connection都报错未初始化。
    - 错因：先写的RedisMgr在写的RedisConPool，所以RedisMgr这里编译器不知道RedisConPool是什么类型。后修改两个类的顺序，顺利通过。