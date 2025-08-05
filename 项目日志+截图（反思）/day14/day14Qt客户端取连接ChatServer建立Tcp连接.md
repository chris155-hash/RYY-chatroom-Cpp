### 一、向ChatServer发送Tcp连接所需的TcpMgr类
- 上一节讲到了按下登录按钮以后去Gateserver发送Http请求，他去查看密码是否正确，去StatusServer查看合适的ChatServer，并把合适的ChatServer发来的uid和token返回给Qt客户端，然后Qt客户端就要去向目标ChatServer建立长连接了。
- 那么这种长连接我们选择Tcp协议，我们依旧使用Qt提供的QNetWorker模块。我们向HttpMgr一样来封装一个`TcpMgr类`，来`管理Tcp连接`。通过它来充当底层的通用处理，发送、接受处理、解决粘包问题等。之前的HttpMgr为什么不需要处理粘包问题呢？
    - 1.HTTP消息格式定义了明确的消息边界；2.而且Qt的QNetworkAccessManager和QNetworkReply类已经处理了HTTP协议层面的所有细节
- 1.首先看一下头文件，其实和HttpMgr差不多；增加了对于粘包的处理：也就是对于接受缓冲区的处理---来处理Tcp的`TLV`结构数据。
    ```cpp
    class TcpMgr: public QObject,public Singleton<TcpMgr>,
                public std::enable_shared_from_this<TcpMgr>
        {
            Q_OBJECT
        public:
            ~TcpMgr();
        private:
            friend class Singleton<TcpMgr>;
            TcpMgr();
            void initHandlers();
            void handleMsg(ReqId id,int len,QByteArray data);//根据id去调用Qmap里对应的回调
            QTcpSocket _socket;
            QString _host;
            uint16_t _port;//后面connectToHost连接服务器的函数第二个参数只接受uint16_t
            QByteArray _buffer;//取出的字节流缓冲区
            bool _b_recv_pending;//标识buffer里数据是否读完了:    buffer里接收数据的头部是否已经提取出来了
            quint16 _message_id;
            quint16 _message_len;//Tlv结构解决粘包，两个字节标识id，两个字节数据长度，然后是数据
            QMap<ReqId,std::function<void(ReqId id,int len,QByteArray data)>> _handlers;
        public slots:
            void slot_tcp_connect(ServerInfo);
            void slot_send_data(ReqId reqid,QString data);
        signals:
            void sig_con_success(bool bsuccess);
            void sig_send_data(ReqId reqid,QString data);
            void sig_switch_chatdlg();
            void sig_login_failed(int);
        };
    ```
    - 1）依旧使用单例模式，方便任何地方都能调用。
    - 2）要使用Qt的信号和槽机制、网络模块QTcpSocket功能等，所以继承下QObject；
    - 3）私有成员变量：目标ChatServer的host、port和socket即文件描述符；接收数据处理数据的缓冲区buffer，一个bool变量标志缓冲区里是否已经读取出下一个消息的长度（buffer里接收数据的头部是否已经提取出来了）；消息的TLV的T和L，也就是消息的id和长度字段。
    - 4）槽函数：第一个slot_tcp_connect，这个就是上面有人来通知槽函数干活了，要向目标服务器建立Tcp连接了，所以会接受信号函数传来的ServerInfo，也就是要建立连接目的服务器的host、port和socket。第二个槽函数slot_send_data是有人要用这个Tcp连接向已经建立连接的服务器去发送信息的槽函数，接收要发送信息的id和String数据，然后计算数据长度也就是Len，构成TLV结构嘛！然后把消息装换成UTF-8的编码格式QByteArray类型，设置下网络字节序的大小端模式，这里和服务器约定好都是用大端模式。最好填入QByteArray（TlV），发送出去_socket.write(block)。
    - 5）信号函数：发送数据的信号、连接成功的信号；登陆成功以后建立了和ChatServer的连接以后切换到聊天界面的信号、登录失败的信号。
    - 6）构造函数：1.触发连接ChatServer成功的信号，方便那个通知他干活的人（比如logindialog知道我连接成功了）；2.连接Qt的TcpSocket的READ信号，读取数据到buffer区；3.错误处理，根据SocketError返回错误提示；4.连接必要的信号；比如连接断开的信号、发送数据的信号和发送数据的槽函数；5.最重要的和HttpMgr里一样的initHandlers初始化相应的url对应的处理槽函数，比如登录功能接收服务器的回包来表示登陆成功，ID_CHAT_LOGIN_RSP对应的处理，将将QByteArray转换为QJsonDocument，触发切换到聊天界面的信号，然后Mainwiddow就会切换界面为聊天界面。
        ```cpp
        QObject::connect(&_socket,&QTcpSocket::readyRead,[&](){
            读取数据处理粘包的回调函数作为槽函数
        });
       //当有数据可读时，读取所有数据.
       //读取所有数据并追加到buffer区
       ```
    - 7）重中之重：构造函数里读取数据处理粘包：包都是按照TLV结构来的，所以先读取TL字段，也就是id + len字段共4字节。够了之后就提取这四个字段，从_buffer里删除，然后根据读取的len字段，配合bool变量_b_recv_pending来判断接下来是专心处理data还是下一个包。_b_recv_pending为true就是黄钻新处理data，_buffer里的长度足够len的大小后就读取len长度的数据，从_buffer里移除；_b_recv_pending为false就是TL还未读取就要等着先读取T和L在处理data。最后根据读取的T也就是Id来调用inithandlers里的回调相应处理。