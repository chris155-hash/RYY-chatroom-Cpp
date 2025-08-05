#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>
#include "usermgr.h"

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)   //处理收到的数据。从buffer里正确拿出收到的数据（主要是：解决粘包，正确拿出数据包）
{
    QObject::connect(&_socket,&QTcpSocket::connected,[&](){
        qDebug() << "connected to server";
        //成功连接服务器后发送消息--连接成功信号
        emit sig_con_success(true);
    });

    QObject::connect(&_socket,&QTcpSocket::readyRead,[&](){
       //当有数据可读时，读取所有数据.
       //读取所有数据并追加到buffer区
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer,QIODevice::ReadOnly);//把buffer的数据以只读的形式读出来--字节流形式
        stream.setVersion(QDataStream::Qt_5_0);

        forever{
           //先解析头部（TLV的TL）
            if (!_b_recv_pending){
                //先检查是否够4字节了，即id+len
                if (_buffer.size() < static_cast<int>(sizeof(quint16)) * 2){
                    return;//数据都不够处理呢，等待更多数据
                }

                //预读取消息id和消息长度，但是不从buffer里删除
                stream >> _message_id >> _message_len;
                //成功获取T和L，从buffer里删除这4个字节
                _buffer = _buffer.mid(sizeof(quint16) * 2);

                //打印一下读取的数据id和长度
                qDebug() << "Message id:" << _message_id << "Message len:" << _message_len;
            }

            //上一个读取了id和长度的数据包还没处理完
            //buffer剩余长度是否满足消息长度，不满足就退出读取继续等待数据追加到buffer
            if (_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }

            //buffer的数据够_message.len的长度了，读取
            _b_recv_pending = false;
            QByteArray messageBody = _buffer.mid(0,_message_len);//从buffer里移除message.len的数据
            qDebug() << "receive body msg is :" << messageBody;

            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id),_message_len,messageBody);//根据id调用回调
        }
    });

    // 处理错误（适用于Qt 5.15之前的版本）
            QObject::connect(&_socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
                                [&](QTcpSocket::SocketError socketError) {
                   qDebug() << "Error:" << _socket.errorString() ;
                   switch (socketError) {
                       case QTcpSocket::ConnectionRefusedError:
                           qDebug() << "Connection Refused!";
                           emit sig_con_success(false);
                           break;
                       case QTcpSocket::RemoteHostClosedError:
                           qDebug() << "Remote Host Closed Connection!";
                           break;
                       case QTcpSocket::HostNotFoundError:
                           qDebug() << "Host Not Found!";
                           emit sig_con_success(false);
                           break;
                       case QTcpSocket::SocketTimeoutError:
                           qDebug() << "Connection Timeout!";
                           emit sig_con_success(false);
                           break;
                       case QTcpSocket::NetworkError:
                           qDebug() << "Network Error!";
                           break;
                       default:
                           qDebug() << "Other Error!";
                           break;
                   }
             });

            //处理连接断开
            QObject::connect(&_socket,&QTcpSocket::disconnected,[&](){
               qDebug() << "Disconnected from server";
            });
            //连接发送信号用来发送数据
            QObject::connect(this,&TcpMgr::sig_send_data,this,&TcpMgr::slot_send_data);
            //注册消息处理
            initHandlers();
}

TcpMgr::~TcpMgr(){

}

void TcpMgr::initHandlers()
{
    //auto self = shared_from_this();
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "handle id is "<< id ;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
           qDebug() << "Failed to create QJsonDocument.";
           return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj is " << jsonObj ;

        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }

        UserMgr::GetInstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::GetInstance()->SetName(jsonObj["name"].toString());
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());

        emit sig_switch_chatdlg();
    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end()){
        qDebug() << "not found id [" << id << "] to handler";
        return;
    }
    find_iter.value()(id,len,data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)//有人通知TcpMgr干活了
{
    qDebug() << "receive tcp connect signal";
    //尝试连接到服务器
    qDebug() << "Connecting to Server...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(_host,_port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id= reqId;

    //将字符串转换为UTF-8编码的字节数组
    QByteArray dataBytes = data.toUtf8();
    //计算长度(使用网络字节序转换)
    quint16 len = static_cast<quint16>(data.size());
    //创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    //设置数据流使用网络字节序(大端模式)
    out.setByteOrder(QDataStream::BigEndian);

    //写入id 和长度
    out << id << len;

    //添加字符串数据
    block.append(dataBytes);

    // 发送数据
    _socket.write(block);
}
