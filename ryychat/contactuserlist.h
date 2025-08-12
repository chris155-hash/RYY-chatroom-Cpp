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
//   void slot_add_auth_friend(std::shared_ptr<AuthInfo>);//收到添加好友申请
//   void slot_auth_rsp(std::shared_ptr<AutnRsp>);//发出的添加好友申请回复
signals:
    void sig_loading_contact_user();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page();
private:
    ConUserItem *_add_friend_item;//已经是好友的联系人条目
    QListWidgetItem *_groupitem;
    bool _loadpending;
};

#endif // CONTACTUSERLIST_H
