# 今天做 Qt 客户端和服务器的联通

## 一、先让服务器端能实现 POST 请求

两个方面：
1. LogicSystem 层面的处理
2. HttpConnection 上的处理

### 1. 先写 LogicSystem 层面的处理
同 GET 请求，向 `_post_handlers` 里添加一个处理验证码的逻辑（LogicSystem 的构造里调用 `RegPost` 添加事件及回调到 `_post_handlers`）。

处理逻辑：
- 失败就返回错误原因，定义一些状态码标识错误原因：json 解析失败、后端调用失败等等。成功就打印一下收到的邮箱到控制台。

### 2. HttpConnection 的处理
HttpConnection：`HandleReq`
添加对 POST 请求的处理，就是去请求 LogicSystem 层面的处理，接受结果，调用失败就说明这个 url 在 LogicSystem 里面没有对应的处理嘛，上层不知道这个 url 是干什么的。

那 HttpConnection 就返回一个 “Url not found”；成功就不需要这个；然后到返回下状态，服务器名等，调用 `WriteResponse` 给客户端写响应。

## 二、客户端
1. 发送 POST 请求
2. 用 config 写用户配置

### 1. 发送 POST 请求
之前点击获取验证码之后，会先用正则表达式看邮箱格式是否符合，符合之后的逻辑是空的，现在逻辑就变成发送 http 的 POST 请求了。之前这里已经有了 HttpMgr 类已经写了 `HttpPostReq` 函数用来发送 POST 请求，调用一下就可以了，把邮箱传进去。

### 2. 用户的邮箱信息等
我们上一步是直接作为参数传进去的，这以后不同用户登录肯定很麻烦，所以我们写一个配置文件。把用户的信息写进去，我们在写一个读取文件拼接 url 的功能即可。

## 两个主要报错

### 一、遇到 Qt 报错
```cpp
error: static assertion failed: No Q_OBJECT in the class with the signal
 #  define Q_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
```
此错误意味着尝试在没有使用 `Q_OBJECT` 宏的类中使用信号（signal）或槽（slot）。

由于在编写 `HttpMgr` 类时忘记在定义中包含 `Q_OBJECT` 宏，导致排查错误花费了很长时间。原来问题在于上述的“没有 Q_OBJECT 宏的类中包含信号”。我一直以为是 `Q_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)`，以为信号和槽的参数类型不匹配，排查了很久，后来才发现了真正的原因。添加 `Q_OBJECT` 宏之后，又发现更多错误，找不到许多 `HttpMgr` 的成员变量及成员函数，怀疑是构建时没有这个宏，于是重新构建项目，顺利运行。

**反思**：理解错误报告的原因，才能有的放矢，准确排查错误，提高效率。

### 二、对象访问成员函数权限问题

实例化的对象一直提示没有权限调用成员函数，我检查了类定义，它们并没有被设为私有（private）。后来发现是忘记写 `public` 关键字，所以类内部不写权限时，默认为私有（private）。
