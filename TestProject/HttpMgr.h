#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "const.h"

class HttpConnection
{
public:
    explicit HttpConnection(net::io_context& ioc)
        : resolver_(ioc), stream_(ioc) {
    }

    // 建立 TCP 连接（可复用）
    void connect(const std::string& host, unsigned short port)
    {
        if (connected_) return;                       // 已连接就跳过
        auto const results = resolver_.resolve(host, std::to_string(port));
        stream_.connect(results);
        host_ = host;
        connected_ = true;
    }
    ////发送请求登录的Http请求
    //QJsonObject json_obj;
    //json_obj["email"] = email;
    //json_obj["passwd"] = xorString(pwd);
    //HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
    //    json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);

    // 同步 POST
    http::response<http::dynamic_body>
        post(const std::string& target,
            const std::string& body,
            const std::string& contentType = "application/x-www-form-urlencoded")
    {
        http::request<http::string_body> req{ http::verb::post, target, 11 };
        req.set(http::field::host, host_);
        req.set(http::field::content_type, contentType);
        req.body() = body;
        req.prepare_payload();             // 自动计算 Content-Length

        http::write(stream_, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream_, buffer, res);
        return res;
    }

    ~HttpConnection()
    {
        try {
            beast::error_code ec;
            stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
        }
        catch (...) {}
    }

    // 禁止拷贝
    HttpConnection(const HttpConnection&) = delete;
    HttpConnection& operator=(const HttpConnection&) = delete;

private:
    tcp::resolver        resolver_;
    beast::tcp_stream    stream_;
    std::string          host_;
    bool                 connected_ = false;
};

#endif // HTTP_CONNECTION_H