#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include "singleton.h"
#include "userdata.h"
#include <vector>
#include <QJsonArray>


class UserMgr:public QObject,public Singleton<UserMgr>,
        public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~UserMgr();
    void SetToken(QString token);
    int GetUid();
    QString GetName();
    QString GetIcon();
    std::shared_ptr<UserInfo> GetUserInfo();
    std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();
    bool AlreadyApply(int uid);//后面优化可以_applylist改成map管理，不用遍历
    void AddApplyList(std::shared_ptr<ApplyInfo> app);
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void AppendApplyList(QJsonArray array);
    void AppendFriendList(QJsonArray array);
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    std::shared_ptr<FriendInfo> GetFriendById(int uid);
    void AppendFriendChatMsg(int friend_id,std::vector<std::shared_ptr<TextChatData>>);//聊天消息的缓存，切换不同用户的时候随时加载出之前的聊天信息

    std::vector<std::shared_ptr<FriendInfo>> GetChatListPerPage();//获取一页显示多少聊天条目
    bool IsLoadChatFin();//聊天条目是否加载完
    void UpdateChatLoadedCount();//更新聊天条目
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    void UpdateContactLoadedCount();//更新联系人条目
    bool IsLoadConFin();//联系人条目是否加载完
private:
    UserMgr();
    QString _token;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;//可以保证有序性,map无序
    std::shared_ptr<UserInfo> _user_info;
    QMap<int,std::shared_ptr<FriendInfo>> _friend_map;//好友列表
    int _chat_loaded;//聊天条目  起始加载点
    int _contact_loaded;//联系人条目  起始加载点
};

#endif // USERMGR_H
