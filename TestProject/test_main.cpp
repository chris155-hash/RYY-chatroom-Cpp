// 改进的多线程连接测试示例
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include "HttpMgr.h"

class ChatServerLoadTester {
private:
    boost::asio::io_context io_context;
    std::vector<std::thread> worker_threads;
    std::queue<int> connection_queue;
    std::mutex queue_mutex;

public:
    void test_concurrent_connections(int total_users) {
        // 创建工作线程池
        const int num_workers = 5; // 使用5个线程
        for (int i = 0; i < num_workers; ++i) {
            worker_threads.emplace_back([this, total_users]() {
                while (true) {
                    int user_id;
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        if (connection_queue.empty()) {
                            break;
                        }
                        user_id = connection_queue.front();
                        connection_queue.pop();
                    }

                    // 处理单个用户的连接
                    connect_and_login(user_id);
                }
                });
        }

        // 填充连接队列
        for (int i = 0; i < total_users; ++i) {
            i = i + 30000;//第一个测试用户的uid=30000
            connection_queue.push(i);
        }

        // 等待所有线程完成
        for (auto& t : worker_threads) {
            t.join();
        }
    }

private:
    void connect_and_login(int user_id) {
        try {
            // 向GateServer发起登录请求，由StatusServer设置Token，返回CHatServer的IP地址
            std::string username = "testuser" + std::to_string(user_id);
            // ... 连接服务器并登录
            std::string server_ip = "0.0.0.0:8090";
        }
        catch (...) {
            // 处理异常
        }
    }
};

int main() {
    ChatServerLoadTester chatservertest;
    chatservertest.test_concurrent_connections(500);
}