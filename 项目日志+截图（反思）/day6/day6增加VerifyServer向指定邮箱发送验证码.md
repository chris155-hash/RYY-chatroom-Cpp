### 一、安装 Nodejs，实现用 JavaScript 写服务器端程序。
认证服务要给邮箱发送验证码，所以用nodejs较为合适，nodejs是一门IO效率很高而且生态完善的语言，用到发送邮件的库也方便。
- 1.官网下载https://nodejs.org/en，一路安装
- 2.安装`grpc-js`包，也可以安装grpc，grpc是C++版本，grpc-js是js版本，C++版本停止维护了。所以用`grpc-js`版本。
- 3.安装`proto-loader`用来动态解析proto文件。
    ```
    npm install @grpc/proto-loader
    ````
- 4.安装email处理的库
    ```
    npm install nodemailer
    ````
- 5.新建一个`proto.js`用来解析`proto`文件,加载`proto`文件路径，读取文件，处理`protobuf`格式的字段。
- 6.在写代码发送邮件之前，我们先去邮箱开启smtp服务。我用的163邮箱，在邮箱设置中查找smtp服务器地址，需要开启smtp服务。这个是固定的，不需要修改。
网易163邮箱的 `SMTP 服务器`地址为: smtp.163.com发送邮件，建议使用授权码(有的邮箱叫 独立密码)，确保邮箱密码的安全性。授权码在邮箱设置中进行设置。如果开启了授权码，发送邮件的时候，必须使用授权码。
- 7.创建一个`config.json`文件，实现参数可配置，所以要读取配置。
- 8.新建`config.js`用来读取配置
- 9.封装发邮件的模块，新建一个`email.js`文件，实现发邮件函数
    - 根据返回的结果出发相应回调，返回结果resolve是正常发送后的回调，reject是异常情况后的回调。
    ```cpp
        /**
    * 发送邮件的函数
    * @param {*} mailOptions_ 发送邮件的参数
    * @returns 
    */
    function SendMail(mailOptions_){
        return new Promise(function(resolve, reject){
            transport.sendMail(mailOptions_, function(error, info){
                if (error) {
                    console.log(error);
                    reject(error);
                } else {
                    console.log('邮件已成功发送：' + info.response);
                    resolve(info.response)
                }
            });
        })
    
    }

    module.exports.SendMail = SendMail
    ```
    这里`Sendmail`是一个异步函数，也就是他不会等事件完成他就会返回回来，那你其实就不知道他返回的时候邮箱是发送成功了还是失败了啊。那我们就用`Promise`封装一下，与C++11的`std::future`有关。
    - Promise 负责“写”（生产值或异常）
    - Future  负责“读”（消费结果）
    - 主线程通过 `future.get()` 取得结果；如尚未就绪会阻塞等待。后面我们调用了`await`来等待Promise抛出结果。
- 10.新建`server.js`，用来启动`grpc server`。

### 二、提升GateServer的并发性。
- 1.`GateServer`存在的问题：
    - 目前`GateServer`的接受以及处理连接在一个`GateServer的main线程ioc`上，那么这种单线程的并发能力就很差了。`HttpConnection`后面可能跑在多个线程上，那都去调用`LogicSystem`的服务，然后触发`HttpConnection`的回调会有线程安全问题。包括`LogicSystem`再去掉Grpc服务也是一样面临线程安全问题。
        - 解决思路：创建一个`连接池`管理stub（Grpc的信使，之前提到过），初始化N个联接到Grpc服务，每个线程取自己的连接后再还回来。
        - 创建一个（`线程池`），分别处理接受连接和处理连接。实现一个多线程的服务端。  添加`ASIO IOContext Pool`结构，让多个iocontext跑在不同的线程中
- 2.改进之后（增加一个线程池+一个连接池）：
    - `线程池`：像一个`多线程多Recator`模型，主线程负责建立连接，每次从线程池取出线程建立HttpConnection监听事件。即主Recator+从Recator。
    - `连接池`：存放多个`stub`，每次LogicSystem要调用RPC服务去取出一个stub来调服务，用完再还回去。
    - `池思想`：这种建立池子来提升`并发`的思想和手段非常常见和使用。就是一个典型的生产者消费者模型，要注意`加锁`保证线程安全，`条件变量`进行线程间通信等。后面要做的MySql、Redis也都会用到类似的技术。