#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include "userdata.h"
#include <memory>
#include <QJsonArray>
#include <unordered_map>
#include "applyfrienditem.h"

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();
    void AddNewApply(std::shared_ptr<AddFriendApply> apply);
protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::ApplyFriendPage *ui;
    void loadApplyList();
    std::unordered_map<int,ApplyFriendItem*> _unauth_items;//待验证的请求条目
public slots:
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);//处理别人发类似的好友申请
signals:
    void sig_show_search(bool);
};

#endif // APPLYFRIENDPAGE_H
