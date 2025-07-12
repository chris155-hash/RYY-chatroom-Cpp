#ifndef HTTPMGR_H
#define HTTPMGR_H
#include <singleton.h>
#include <QString>
#include <QObject> //信号和槽
#include <singleton.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QNetworkAccessManager>

//这里就用了CRTP，Singleton<T>,这里又把自身类型做模版参数传递给Singleton又自己继承
class HttpMgr: public QObject,public Singleton<HttpMgr>,public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();//析构写成public：派生类析构-》基类析构-》Singleton的_istance虽然是静态成员变量，但也是成员变量，所以也要析构-》智能指针的析构-》T类型的析构-》HttpMgr的析构，所以要写成public
    void PostHttpReq(QUrl url,QJsonObject json,ReqId req_id,Modules mod);//发送post请求，需要的参数：url，JSON序列（发的是字节流嘛），功能id，模块（因为是异步的，回调不知道什么时候触发，用他们区分）
private:
    friend class Singleton<HttpMgr>; //之所以设为友元函数，Singleton里_istance的时候new T会用到T的构造，这里写成私有那加个friend让他能访问private
    HttpMgr();//单例类自然私有
    QNetworkAccessManager _manager;

private:
    void slot_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);
signals:
    void sig_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);//完成http发送后通知其他模块
    void sig_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
};

#endif // HTTPMGR_H
