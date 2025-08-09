#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include <functional>
#include "global.h"
#include "singleton.h"
#include <QObject>
#include "userdata.h"


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
    void sig_user_search(std::shared_ptr<SearchInfo>);
};



#endif // TCPMGR_H
