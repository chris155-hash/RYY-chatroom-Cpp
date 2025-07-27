# 日志明天再补，明天有场面试，先准备面试去

### 一、安装了 GRPC，负责服务器之间的通信。
安装好之后用 `cmake` 编译一下，grpc 编译还需要 Nasm，Go 和 Perl。
NASM 是用于编写底层汇编代码的工具，Go 是一种高效的编程语言，适用于构建高性能的服务器端应用程序，而 Perl 是一种强大的脚本语言。我先暂时这么浅显的理解。
gRPC 是一种允许客户端应用程序直接调用另一台机器上的服务器应用程序中的方法，就像调用本地对象一样，从而让创建分布式应用和服务变得更加容易。
这是我下载的对应的版本：
- `cmak` v3.31.6 官网可以下（不是最新版）
- grpc v1.34.0 (用的 git)
- go v1.24.0 官网下的（最新的）
- perl v5.16.2
- nasm v2.16.0 官网下的（不是最新版）

### 二、编写服务器之间通信的 protobuf 格式文件
之前客户端和 GateServer 之间的通信序列化格式我们选用的是 Json 嘛，主要因为可读，这里服务器之间通信用 RPC，RPC 的序列化格式是 Protobuf，效率更高嘛，相比 JSON 或 XML 等文本格式，Protobuf 在序列化和反序列化时速度更快，占用的带宽更少。

1. 创建 `message.proto`
Protobuf 格式，定义了服务接口和交互的消息格式，以及服务Service，后续grpc编译以后会生成客户端、服务器存根Stub等。我们还是先实现验证码操作，那就定义一下 VarifyService 的服务。
    ```protobuf
    service VarifyService {
    rpc GetVarifyCode (GetVarifyReq) returns (GetVarifyRsp) {}
    }
    ```
定义了一个名为 `GetVarifyCode` 的远程过程调用（RPC）方法。他要做的事：接收一个 `GetVarifyReq` 类型的请求消息，并返回一个 `GetVarifyRsp` 类型的响应消息。
   ```
   service VarifyService {
      rpc GetVarifyCode (GetVarifyReq) returns (GetVarifyRsp) {}
   }
   ```
随后就相应的定义一下这两种消息格式：`GetVarifyReq` 格式就填一个 `string` 类型的 `email` 就行，`GetVarifyRsp` 就定义三个字段，`int` 类型 `error` = 1 表示错误与否；`string` 类型返回验证码；`string` 类型返回 `email`。

为什么会返回 `email` 呢，觉得有些多余？
1. 调试和服务器日志记录；确认客户端的邮箱；比如：如果用户报告未收到验证码，返回的 `email` 字段可以帮助确认验证码是否发送到了正确的邮箱。
2. 客户端不需要额外存储请求的邮箱地址，而是可以直接从响应中获取。

之后 `proc.exe` 生成 `proto` 的 grpc 的头文件和源文件，生成用于序列化和反序列化的 `pb` 文件。这些是 grpc 需要的吧。
`C:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe --cpp_out=. "message.proto"`
`C:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe  -I="." --grpc_out="." --plugin=protoc-gen-grpc="C:\cppsoft\grpc\visualpro\Debug\grpc_cpp_plugin.exe" "message.proto" `

### 三、 调用 RPC 方法发送请求接受响应。
   - `VerifyGrpcClient` 单例类
        - 上面只是定义了交互的信息格式和接口，现在来使用它。写一个单例类 `VerifyGrpcClient`，RPC 的客户端也就是,现在是我们的 `GateServer` 服务器，服务提供方是 `Varify` 服务器。
        - 这个类要做的事，创建 `channel`（RPC 要求的），绑定本地的端口 `50051`（`127.0.0.1:50051` 本地 IP+端口，`GateServer` 地址），不设置安全证书。
     ```cpp
     grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials())
     ```
   - `VarifyService::NewStub(channel)` 创建了一个服务存根（Stub），用于调用服务端的方法。
   - 一个 `GetVarifyCode` 函数（相当于 Rpc 的调用方--`GateServer` 要做的）：保存上下文，构造请求，调用 stub 里的 `GetVarifyCode` 函数（上面提到的 rpc 提供的），接受响应。
   - 根据响应结果处理，失败就返回“RPC 调用失败”，成功就直接返回响应。

### 四、 `GateServer` 收到 POST 请求获取验证码的逻辑
   - 之前这里就只是打印一下 Qt 客户端发来的 email 地址，现在要加上调用 RPC 服务发送邮箱获取验证码了。

### 五、 添加 `GateServer` 的配置文件
   - 和之前 QT 端的工作一样，添加 `config.ini` 文件。来接受其他服务器的信息。因为 `GateServer` 是个中间人，要和很多其他服务器打交道。先这样写：

        ```cpp
        [GateServer]
        Port = 8080
        [VarifyServer]
        Port = 50051
        ```
   - 添加ConfigMgr类用来读取和管理配置，就是读取文件，按照key和value匹配字段。
   - map<string,SectionInfo>；SectionInfo就是map<string,string>这种结构SectionInfo的map的key就是port，value就是对应的值。
        - 因为后面一个服务器可能不同的端口提供不同的服务，那一个服务器就对应不止一个port-8080这种了。



### 番外

#### 昨天面试问到为什么 Qt 等程序可以在多个不同的系统上跨平台使用？或者说写一个跨平台项目应该考虑什么，怎么保证？

1. **抽象层设计**
   - Qt 通过在不同操作系统之间提供一个抽象层，隐藏了底层操作系统的细节。开发者通过 Qt 的 API 编写代码，而 Qt 框架则负责将这些调用转换为特定操作系统所需的实现。

2. **统一的 API**
   - Qt 提供了一套统一的 API，这些 API 在所有支持的平台上都是一致的。这意味着开发者可以使用相同的代码在不同平台上运行，而无需为每个平台单独编写代码。

3. **平台无关的代码生成**
   - Qt 的构建系统（如 qmake 或 CMake）会根据目标平台生成相应的构建文件。这些构建文件会包含特定平台的编译器选项和链接器选项，确保代码可以在目标平台上正确编译和链接。

#### 那我自己感觉就是：
1. 他有自己的抽象层实现具体逻辑，qt封装好了抽象层到底层的转换；
2. 不管 Windows 还是 Linux 或 MacOS 他用到都是 qt 提供的 Api，比如 QNetworkAccessManager 类用于处理网络请求，所以编写的时候不用考虑不同平台的差异；
3. Cmake 构建项目的时候，来根据不同的平台生成相应的构建文件，在链接阶段实现大一统。

### 想写跨平台的项目：
1. 不用某一个平台特有的 Api；
2. 可以用支持各种平台的库，比如上面安装的 grpc、boost、nasm 之类的都是有 windows、linux、macOS 等不同版本的；
3. 用 Cmake 支持跨平台来构建项目适应不同平台。

