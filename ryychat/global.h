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
    ID_SEARCH_USER_REQ = 1007, //用户搜索请求
    ID_SEARCH_USER_RSP = 1008, //搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,  //添加好友申请
    ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复(来自ChatServer的回应)
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ  = 1017,  //文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,  //文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
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
    ADD_USER_TIP_ITEM,//提示添加用户
    INVALID_ITEM,//不可点击的条目
    GROUP_TIP_ITEM,//分组提示条目
    APPLY_FRIEND_ITEM,//添加好友条目
};

enum ChatRole{
    Self,
    Other,
};

struct MsgInfo{
    QString msgFlag;//消息类型：文本、图片、文件
    QString content;//表示文件和图片的url，文本内容
    QPixmap pixmap;//文件和图片的缩略图
};

const std::vector<QString> heads = {
    ":/res/head_1.jpg",
    ":/res/head_2.jpg",
    ":/res/head_3.jpg",
    ":/res/head_4.jpg",
    ":/res/head_5.jpg"
};

const std::vector<QString> names = {
    "任阳阳",
    "徐纯纯",
    "毛奕",
    "高良平",
    "钱子莲",
    "毛传峰",
    "C++",
    "python",
    "java",
};

const std::vector<QString>  strs ={"hello world !",
                             "nice to meet u",
                             "New year，new life",
                            "You have to love yourself",
                            "My love is written in the wind ever since the whole world is you",
                            };

const int MIN_APPLY_LABEL_ED_LEN = 40;//申请好友标签输入框最低长度
const QString add_prefix = "添加标签";
const int tip_offset = 5;//标签偏移量

#endif // GLOBAL_H
