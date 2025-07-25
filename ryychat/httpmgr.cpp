#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr()
{
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)  //QT发送http请求的逻辑和写法
{
    QByteArray data = QJsonDocument(json).toJson();//序列化。不对数据做加密，压缩等处理。数据透传
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");//设置http请求的header信息
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));//发送信息长度，用QByteArray等定义好的规则，大端小端等
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request,data);//客户端向服务器发送POST请求，并接受回包
    QObject::connect(reply,&QNetworkReply::finished,[self,reply,req_id,mod] (){  //这里捕获self而不是this，是因为怕这个HttpMgr已经被清理掉
        //处理错误情况
        if (reply->error() != QNetworkReply::NoError){
            qDebug() << reply->errorString();
            //发送信号通知完成
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);
            reply->deleteLater();
            return;
        }
        //无错误
        QString res = reply->readAll();
        //发送信号通知完成
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD){
        //发送信号通知指定模块http的响应结束了
        emit sig_reg_mod_finish(id,res,err);
    }
    if (mod == Modules::RESETMOD){
        //发送信号通知指定模块http的响应结束了
        emit sig_reset_mod_finish(id,res,err);
    }
}
