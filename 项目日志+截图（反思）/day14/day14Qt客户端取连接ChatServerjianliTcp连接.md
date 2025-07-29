### 一、向ChatServer发送Tcp连接所需的TcpMgr类
- 上一节讲到了按下登录按钮以后去Gateserver发送Http请求，他去查看密码是否正确，去StatusServer查看合适的ChatServer，并把合适的ChatServer发来的uid和token返回给Qt客户端，然后Qt客户端就要去向目标ChatServer建立长连接了。
- 那么这种长连接我们选择Tcp协议，我们依旧使用Qt提供的QNetWorker模块。我们向HttpMgr一样来封装一个`TcpMgr类`，来`管理Tcp连接`。通过它来充当底层的通用处理，发送、接受处理、解决粘包问题等。之前的HttpMgr为什么不需要处理粘包问题呢？
    - 1.HTTP消息格式定义了明确的消息边界；2.而且Qt的QNetworkAccessManager和QNetworkReply类已经处理了HTTP协议层面的所有细节
- 1.首先看一下头文件，其实和HttpMgr差不多；增加了对于粘包的处理：也就是对于接受缓冲区的处理来处理Tcp的Tlv结构数据。
    ```cpp
    class TcpMgr:public QObject, public Singleton<TcpMgr>,
        public std::enable_shared_from_this<TcpMgr>
        {
            Q_OBJECT
        public:
            TcpMgr();
        private:
            QTcpSocket _socket;
            QString _host;
            uint16_t _port;
            QByteArray _buffer;
            bool _b_recv_pending;
            quint16 _message_id;
            quint16 _message_len;
        public slots:
            void slot_tcp_connect(ServerInfo);
            void slot_send_data(ReqId reqId, QString data);
        signals:
            void sig_con_success(bool bsuccess);
            void sig_send_data(ReqId reqId, QString data);
        };
    ```
    - 1）依旧使用单例模式，方便任何地方都能调用。