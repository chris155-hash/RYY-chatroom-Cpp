#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include <QRegularExpression>
#include <iostream>
#include <memory>
#include <mutex>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include "QStyle"
/**
 * @brief repolish  用来刷新qss
 */
extern std::function<void(QWidget*)> repolish;

enum ReqId{
    ID_GET_VARIFY_CODE = 1001,//获取验证码 的功能ID
    ID_REG_USER = 1002,//注册用户 的 功能ID
};

enum Modules{
  REGISTERMOD = 0,
};

enum ErrorCodes{
    SUCCESS  = 0,
    ERR_JSON = 1, //json解析失败
    ERR_NETWORK = 2, //网络错误
};

extern QString gate_url_prefix;

#endif // GLOBAL_H
