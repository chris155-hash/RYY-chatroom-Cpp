#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include <QMouseEvent>
#include "chatuserwid.h"
#include "loadingdlg.h"

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog),
    _mode(ChatUIMode::ChatMode),_state(ChatUIMode::ChatMode),_b_loading(false)
{
    ui->setupUi(this);
    ui->add_btn->SetState("normal","hover","press");
    ui->search_edit->SetMaxLength(15);

    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction,QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    // 创建一个清除动作并设置图标
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    // 初始时不显示清除图标
    // 将清除动作添加到LineEdit的末尾位置
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

    // 当需要显示清除图标时，更改为实际的清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }

    });

    connect(clearAction,&QAction::triggered,[this,clearAction](){
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        ui->search_edit->clearFocus();
        //清除按钮被按下则不显示搜索框
        ShowSearch(false);
    });

    ShowSearch(false);
    connect(ui->chat_user_list,&ChatUserList::sig_loading_chat_user,this,&ChatDialog::slot_loading_chat_user);
    //test
    addChatUserList();

    //左边状态栏加载
    QPixmap pixmap(":/res/head_1.jpg");
    QPixmap scaledPixmap = pixmap.scaled( ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    ui->side_chat_lb->setProperty("state","normal");
    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_contact_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);
    connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);
    //连接搜索框输入变化
    connect(ui->search_edit,&QLineEdit::textChanged,this,&ChatDialog::slot_text_changed);

    this->installEventFilter(this);//安装事件过滤器

    //设置聊天Label默认是选中状态
    ui->side_chat_lb->SetSelected(true);
    //为search_list设置search_edit
    ui->search_list->SetSearchEdit(ui->search_edit);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}


void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for (auto& ele : _lb_list){
        if (ele == lb){
            continue;
        }
        ele->ClearState();
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);//QEvent是事件基类，QMouseEvent是其中一种派生类。还有定时器类、窗口类等事件。基类指针转成子类指针,提前做了类型检查，static_cast
        handleGlobalMousePress(mouseEvent);
    }

    return QDialog::eventFilter(watched,event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    //实现点击位置的判断和处理逻辑
    //如果不是搜索模式，那搜索列表就没显示，直接返回
    if (_mode != ChatUIMode::SearchMode){
        return;
    }
    //将鼠标点击的位置装换位相对于搜索列表的相对坐标.鼠标点击范围在搜索框内直接返回不处理，否则不显示搜索列表
    QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
    if (!ui->search_list->rect().contains(posInSearchList)){     //rect()返回一个 QRect 对象，表示该控件自身的内部矩形区域，
        ui->search_edit->clear();
        ShowSearch(false);
    }
}

void ChatDialog::ShowSearch(bool bsearch)
{
    if(bsearch){
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->con_user_list->hide();
        ui->search_list->hide();
        ui->chat_user_list->show();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}

void ChatDialog::slot_loading_chat_user()
{
    if(_b_loading){
        return;
    }

    _b_loading = true;
    LoadingDlg *loadingDialog = new LoadingDlg(this);
    loadingDialog->setModal(true);//将 loadingDialog 设置为“模态窗口”,阻塞用户与其他窗口的交互，直到该窗口被关闭.
    loadingDialog->show();
    qDebug() << "add new data to list.....";
    addChatUserList();
    // 加载完成后关闭对话框
    loadingDialog->deleteLater();

    _b_loading = false;
}

void ChatDialog::slot_side_chat()
{
    qDebug() << "receive side_chat_label clicked";
    ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact()
{
    qDebug() << "receive side_contact_label clicked";
    ClearLabelState(ui->side_contact_lb);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_text_changed(const QString &str)
{
    //qDebug() << "oreceive slot text changed str is " << str;
    if (!str.isEmpty()){
        ShowSearch(true);
    }
}
