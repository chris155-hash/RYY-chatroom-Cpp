#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog() override;
    void addChatUserList();
    void ClearLabelState(StateWidget* lb);
protected:
    bool eventFilter(QObject* watched,QEvent* event) override;
    void handleGlobalMousePress(QMouseEvent* event);
private:
    void ShowSearch(bool bsearch);
    void AddLBGroup(StateWidget* lb);
    void SetSelectChatItem(int uid);
    void SetSelectChatPage(int uid);
    void loadMoreChatUser();
    void loadMoreConUser();
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget*> _lb_list;
    QMap<int,QListWidgetItem*> _chat_items_added;//已经添加到Chat聊天列表的 聊天信息条目 集合
    int _cur_chat_uid;//正在聊天用户的uid

private slots:
    void slot_loading_chat_user();
    void slot_loading_con_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
public slots:
    void slot_friend_apply(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);

};

#endif // CHATDIALOG_H
