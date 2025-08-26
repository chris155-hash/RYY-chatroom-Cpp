#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H
#include <QListWidget>
#include <QEvent>
#include <QScrollArea>
#include <QWheelEvent>
#include <memory>
#include "userdata.h"
#include <QDebug>


class ConUserItem;

class ContactUserList:public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget *parent = nullptr);
    void ShowRedPoint(bool bshow =true);
protected:
    bool eventFilter(QObject *watched,QEvent *event) override;
private:
    void addContactUserList();//列表加载
public slots:
    void slot_item_clicked(QListWidgetItem *item);
   void slot_add_auth_friend(std::shared_ptr<AuthInfo>);//发出的好友申请被对面同意了 显示到联系人列表
   void slot_auth_rsp(std::shared_ptr<AuthRsp>);//同意对面发来的好友申请 的服务器回包
signals:
    void sig_loading_contact_user();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page(std::shared_ptr<UserInfo> user_info);
private:
    ConUserItem *_add_friend_item;//已经是好友的联系人条目
    QListWidgetItem *_groupitem;
    bool _load_pending;

    int x = 100;//ryy test,模拟联系人的uid.
};

#endif // CONTACTUSERLIST_H
