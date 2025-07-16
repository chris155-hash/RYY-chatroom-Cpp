### 一、安装Redis的Windows版本。
- 1.目的
    - 我们的验证码是要设置过期的，可以用redis管理过期的验证码自动删除，key为邮箱，value为验证码，过期时间为3min。这是我们使用Redis的目的。
- 2.安装
    - windows 版本下载地址:
        ```
        https://github.com/tporadowski/redis/releases
        ```
    - 启动redis 服务器 
        ```
        .\redis-server.exe .\redis.windows.conf
        ```
    - 测试能否链接以及以后查看数据,可以下载redis desktop manager
        ```
        redisdesktop.com/
        ```
    - C++ 的redis库有很多种，最常用的有hredis和redis-plus-plus. 我们用redis-plus-plus。
- 3.编译
    - 用Redis编译生成hiredis工程和Win32_Interop工程即可，分别点击生成,生成hiredis.lib和Win32_Interop.lib即可。
    - 在项目属性表中导入文件即可。
- 4.代码测试
        ```cpp
        void TestRedis() {
        //连接redis 需要启动才可以进行连接
        //redis默认监听端口为6387 可以再配置文件中修改
            redisContext* c = redisConnect("127.0.0.1", 6380);
            if (c->err)
            {
                printf("Connect to redisServer faile:%s\n", c->errstr);
                redisFree(c);        return;
            }
            printf("Connect to redisServer Success\n");

            std::string redis_password = "123456";
            redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password);
            if (r->type == REDIS_REPLY_ERROR) {
                printf("Redis认证失败！\n");
            }else {
                printf("Redis认证成功！\n");
                }

            //为redis设置key
            const char* command1 = "set stest1 value1";

            //执行redis命令行
            r = (redisReply*)redisCommand(c, command1);

            //如果返回NULL则说明执行失败
            if (NULL == r)
            {
                printf("Execut command1 failure\n");
                redisFree(c);        return;
            }

            //如果执行失败则释放连接
            if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
            {
                printf("Failed to execute command[%s]\n", command1);
                freeReplyObject(r);
                redisFree(c);        return;
            }

            //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(r);
            printf("Succeed to execute command[%s]\n", command1);

            const char* command2 = "strlen stest1";
            r = (redisReply*)redisCommand(c, command2);

            //如果返回类型不是整形 则释放连接
            if (r->type != REDIS_REPLY_INTEGER)
            {
                printf("Failed to execute command[%s]\n", command2);
                freeReplyObject(r);
                redisFree(c);        return;
            }

            //获取字符串长度
            int length = r->integer;
            freeReplyObject(r);
            printf("The length of 'stest1' is %d.\n", length);
            printf("Succeed to execute command[%s]\n", command2);

            //获取redis键值对信息
            const char* command3 = "get stest1";
            r = (redisReply*)redisCommand(c, command3);
            if (r->type != REDIS_REPLY_STRING)
            {
                printf("Failed to execute command[%s]\n", command3);
                freeReplyObject(r);
                redisFree(c);        return;
            }
            printf("The value of 'stest1' is %s\n", r->str);
            freeReplyObject(r);
            printf("Succeed to execute command[%s]\n", command3);

            const char* command4 = "get stest2";
            r = (redisReply*)redisCommand(c, command4);
            if (r->type != REDIS_REPLY_NIL)
            {
                printf("Failed to execute command[%s]\n", command4);
                freeReplyObject(r);
                redisFree(c);        return;
            }
            freeReplyObject(r);
            printf("Succeed to execute command[%s]\n", command4);

            //释放连接资源
            redisFree(c);
        }
        ```
