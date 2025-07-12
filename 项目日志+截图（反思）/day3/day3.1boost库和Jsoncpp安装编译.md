# 网关服务器设计与实现

## 功能概述
网关服务器主要应答客户端基本的连接请求，包括根据服务器负载情况选择合适服务器给客户端登录、注册、获取验证服务等，接收 HTTP 请求并应答。

## 一、Boost 库进行扩充

Boost 是一个开源的 C++ 库集合，旨在为 C++ 标准库提供扩展和补充。Boost 库由一群经验丰富的开发者维护，其中许多库的设计和实现质量都非常高，甚至有些库已经被纳入 C++ 标准库。Boost 库广泛应用于各种项目中，从简单的工具到复杂的系统开发。

### 安装与编译

这里安装好 `boost_1_83_0` 以后一直无法编译，我是 VS Studio 2022 版本。编译分为两步，可以用 mingw 的 gcc 编译，也可以用 vs 的 clang 系列编译。由于后面我项目使用 VS Studio 写的，所以一直尝试 clang 编译。

1. `./booststrap.bat` 编译 `b2.exe` 工具
2. `.\b2.exe install --toolset=msvc-14.2 --build-type=complete --prefix="C:\boost_1_83_0\boost_MSVC" link=static runtime-link=shared threading=multi debug release`

这里一直不成功，相关环境全部没有检测到。试了很多教程的方法都没有用。后来终于在 Github 上找到了答案，一是环境变量，这个是大部分教程解决的问题，在此不做赘述。

2. Boost 的问题不是我们的问题。只有在特定的版本才会出现问题。Boost 对 cl 14.3 及以上版本缺乏支持，需要修改编译脚本才行。

再此附上链接，以供参考：
- [Windows + MinGW/MSVC 安装Boost程序库环境 踩坑指南](https://www.cnblogs.com/aquawius/p/17903200.html)
- [Why can't I build boost in my computer? · Issue #931 · boostorg/boost](https://github.com/boostorg/boost/issues/931)

## 二、JsonCpp

JsonCpp 是一个开源的 C++ 库，用于解析、生成和操作 JSON 数据。它提供了简单易用的 API，使得在 C++ 程序中处理 JSON 数据变得方便和高效。

我们使用它来进行序列化和反序列化。当然也可以使用 Protocol Buffers、XML 等。其中 Protocol Buffers 是二进制格式，体积小、解析速度快，适合对性能有要求的场景。

我们这里之所以用 Json 格式是因为它的可读性比较好，这样一个全栈项目，前后端的通信我们可以看到，排查问题等更方便。

编译安装没遇到什么问题，在此不做赘述。