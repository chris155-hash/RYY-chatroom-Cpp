### 一、VarifyServer增加redis.js
- 之前的Redis我们测试的时候是直接让GateServer连接的Redis服务器进行的测试，其实就是没有调用Grpc服务的，目的就是实验Redis的工作正常，连接池工作正常嘛。
- 现在他们都是正常的，我们就进行下一步：中间加入之前的VarifyServer，GateServer和Redis之间通过它去桥接。可以看一下实验截图。
    ```
    - 流程如下：客户端-》GateSerfer-》GrpcServer 
    GrpcServer    1. -》RedisServer-》存储验证码
    GrpcServer    2. -》Sendmail-》发送验证码
    ```
- 1.首先VarifyServer使用Node.js(javascript)和json写的。先要给他加上Redis管理，让他充当Redis的客户端。
- 2.这个Redis.js要做的事：
    - 1）创建Redis客户端实例，连接Redis的ip+port，验证密码，正确连接到Redis服务器。  说白了，我们又是在做同样的事，拿人家大佬提供的Redis来用，把人家的接口封装一下。感觉基本就是这些工作。毕竟Redis、Mysql也好还是beast这些也好，工具拿来用就好了。这种使用库、轮子来加速开发的过程其实就是很好的面向对象思想的体现。封装继承多态其实最重要的就是封装。之前反问面试官，怎么分模块确定每个模块的功能和调用关系？
        - 他的回答是：在工程中学习，确定好每个模块的功能，这个模块应该暴露哪些接口，隐藏哪些接口，先把这些考虑清楚了。然后看一些设计模式的书，多做项目自然就掌握了。
    - 2）连接错误时，及时抛出错误，并退出。
    - 3）能够操作Redis，Redis毕竟相当于一个库，帮我们缓存用户邮箱及验证码。肯定要能操作它，不然有什么用。操作那就是Set、Get、Query、Quit。
        - Get：根据key值返回对应的value。这里的key我们设置为_code+邮箱，也就是接了一个固定前缀，虽然现在redis只存了验证码，后面可能会存更多东西，加个前缀来区分肯定有好处。返回为空或返回错误码相应处理。
        - Set：向Redis里添加key，value和expiretime。添加邮箱，验证码和过期时间。Redis只是管理Redis的中间接口，实际操作还是在VarifyServer。
        ```cpp
        async function SetRedisExpire(key,value, exptime)
        ```
        - Query、quit较简单，可自行参考源码。
### 二、VarifyServer端的Server.js要增加对Redis的调用
- 1.之前这个VarifyServer做的事：生成uuid，发送uuid作为验证码。现在我们先让他去Redis服务器查询该邮箱是否在Redis里有对应的验证码，有就直接返回；没有就生成uuid，把前四位作为验证码，和邮箱、过期时间插入到Redis库里。
- 2.调用`emailModule.SendMail`模块发送邮件。
- 3.关键代码如下：里面的redis_module就是第一步封装的操作Redis的部分。
    ```cpp
     let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);
        console.log("query_res is ", query_res)
        if(query_res == null){

        }
        let uniqueId = query_res;
        if(query_res ==null){
            uniqueId = uuidv4();
            if (uniqueId.length > 4) {
                uniqueId = uniqueId.substring(0, 4);
            } 
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,600)
            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
                });
                return;
            }
        }
    ```