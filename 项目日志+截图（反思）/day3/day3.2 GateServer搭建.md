# GateServer 网关服务器设计与实现

## 概述
网关服务器主要应答客户端基本的连接请求，包括根据服务器负载情况选择合适服务器给客户端登录、注册、获取验证服务等，接收HTTP请求并应答。

## 一、Cserver类
创建一个TCP服务器，监听指定端口并接受客户端连接，相当于执行事件轮询监听链接的类。负责接收`port`，监听事件到来。

- 使用`boost::asio::ip::tcp::socket`来绑定上下文`io_context`和`socket`，异步接收新连接。
- `tcp::acceptor`是Boost.Asio中用于监听和接受传入连接请求的核心组件。它支持异步操作，使得服务器可以高效地处理多个连接，而不会阻塞主线程。
- `io_context`是事件循环（event loop）的核心，负责处理网络事件、定时器事件以及其他异步操作。Windows一般用I/O完成端口（I/O Completion Ports, IOCP）来实现高效的I/O操作，Linux里一般用的是epoll、poll、select等。负责事件循环，监听哪些socket集合有I/O事件、定时事件等发生就交由上层处理。
- 编写一个`Start()`函数，监听客户端的连接建立请求即可，在它内部创建一个HttpConnection类的智能指针进行http管理。将`_socket`内部数据转移给HttpConnection管理，`_socket`继续用来接受新的连接。

## 二、HttpConnection类（重点）
这个类实际上是底层的监听到可读、可写、定时事件（CheckDeadline）以后交由上层处理就完事了。相当于一个中间人，向上逻辑层及时转达客户端的需求（HandleReq），并进行翻译等；对下把逻辑层的处理结果等返回给对方（WriteResponse）。

1. 四个成员变量：
   - `_buffer` 用来接受数据
   - `_request` 用来解析请求
   - `_response` 用来回应客户端
   - `_deadline` 用来做定时器判断请求是否超时

2. 三个成员函数：
   - `void CheckDeadline();` HTTP 处理请求发送的数据包不能超时。检测超时的函数。
   - `void WriteResponse();` 异步写入 HTTP 响应，然后关闭连接。
   - `void HandleReq();` 设置版本号，短连接，根据post还是get做不同处理。调用HandleReq去交由LogicSystem处理，接受返回结果。然后返回状态码，调用WriteResponse写响应。

## 三、LogicSystem类（重点）
逻辑层处理，根据get还是post请求等调用不同函数处理。依旧采用单例模式。

1. 两个成员变量，两个map，一个处理get请求，一个处理post请求。key就是路由，value就是对应的处理回调函数。
   ```cpp
   std::map<std::string, HttpHandler> _post_handlers;
   std::map<std::string, HttpHandler> _get_handlers;
   ```
   这里的Httpdler就是一个封装,std::function
    ```cpp
    typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
    ```
  其实就隐含了一个信息lambda这种语法糖就是使用的std::function做底层（不准确），后面你RegGet还是RegPost等等往这两个map里添加回到的时候是直接用lambda的。
- 语法糖：让代码更简洁、更易读，但不会增加语言的功能。
### 2. 成员函数

- `bool HandleGet(std::string path, std::shared_ptr<HttpConnection> con)`
  - `HandleGet` 函数，顾名思义，根据 key 返回相应的回调函数成功否，当然查到了就只要执行一下对应的回调，不然这一切图啥是吧。
  - 剩下的是一堆可能的 handler 函数，先写一个 `get/test` 的。那就要在 `LogicSystem` 的类构造函数里先注册。
  - `void RegGet(std::string, HttpHandler handler)`，插入 `get/test` 请求及要触发的回调到对应的 `get_handlers`。上面注册就是用这个函数注册插入的。

## 3. 构造函数
- 向两个（get 和 post）handlers 里添加 url 及相应的回调。

## 四、URL 解析
- get 请求带参数的情况我们要实现参数解析，我们可以自己实现简单的 url 解析函数。
- `http://localhost:8080/get_test?key1=value1&key2=value2`
  - 上面是一种举例，我们处理 URL 请求可能需要：
    1. 将十进制的 char 转为 16 进制
    2. 从 16 进制转为十进制的 char
    3. 汉字会根据字节数用 % 划分，然后高四位和低四位分别转换为 16 进制。>>4 取高四位，&0x0F 取低四位。
  - 然后实现 URL 编码和解码函数加到 `HttpConnection` 里即可，以及两个成员变量。`get_url` 和 `get_param` 来接受读取的 URL 和参数。

### 为什么用 16 进制？
- 十六进制编码是一种通用的编码方式，可以将任何字符转换为一个固定的格式。
  - 跨平台和跨语言的兼容性。
  - URL 编码可以防止注入攻击。

### Qt 信号和槽机制的 C++ 实现

今天面试问到 Qt 的信号和槽机制，能不能用 C++ 实现类似的功能？
- 当时先想到管道，后来面试官说不用这么复杂。其实这里很像上面的 `_handler` 的逻辑，包括当时写这里的时候我就感觉很像，后来去搜信号和槽机制时候 AI 回答元数据，元对象啥的我也没看懂就没继续思考下去。
- 现在想想就可以参考这里实现一个 `map<>`, key 就对应一个 singal 类，value 就是 slot 类或者直接回调函数，connection 就像是往 map 里添加一组对应关系。但是面试的时候又忘记这里怎么做的了，想了半天也没想到，还是太菜了。