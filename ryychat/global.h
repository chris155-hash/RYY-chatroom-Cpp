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

extern std::function<QString(QString)> xorString;

enum ReqId{
    ID_GET_VARIFY_CODE = 1001,//获取验证码 的功能ID
    ID_REG_USER = 1002,//注册用户 的 功能ID
    ID_RESET_PWD = 1003, //重置密码 的功能ID
    ID_LOGIN_USER = 1004,  //用户登录
    ID_CHAT_LOGIN = 1005,  //登录聊天服务器
    ID_CHAT_LOGIN_RSP = 1006,  //登录聊天服务器回包
};

enum Modules{
  REGISTERMOD = 0,
  RESETMOD = 1,
  LOGINMOD = 2,

};

enum ErrorCodes{
    SUCCESS  = 0,
    ERR_JSON = 1, //json解析失败
    ERR_NETWORK = 2, //网络错误
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

struct ServerInfo{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6,
};

extern QString gate_url_prefix;

//聊天界面几种模式
enum ChatUIMode{
    SearchMode,//搜索模式
    ChatMode,//聊天模式
    ContactMode,//联系人模式
};

enum ListItemType{
    CHAT_USER_ITEM,//聊天用户条目
    CONTACT_USER_ITEM,//联系人用户条目
    SEARCH_USER_ITEM,//搜索到的用户
    ADD_USER_ITEM,//提示添加用户
    INVALID_ITEM,//不可点击的条目
    GROUP_TIP_ITEM,//分组提示条目
};

#endif // GLOBAL_H
