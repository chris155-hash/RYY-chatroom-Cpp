#include "searchlist.h"
#include <QScrollBar>
#include "adduseritem.h"
//#include "invaliditem.h"
//#include "findsuccessdig.h"
#include "tcpmgr.h"
#include "customizeedit.h"
//#include "findfaildlg.h"
#include "loadingdlg.h"
#include "findsuccessdlg.h"
#include "userdata.h"
#include <memory>
#include <QJsonDocument>
#include "findfaildlg.h"
#include <QDebug>

SearchList::SearchList(QWidget *parent):QListWidget(parent),_find_dlg(nullptr),_search_edit(nullptr),_send_pending(false),_loadingDialog(nullptr)
{
    Q_UNUSED(parent);
     this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //添加条目
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}

void SearchList::CloseFindDlg()
{
    if (_find_dlg){
        _find_dlg ->hide();
        _find_dlg = nullptr;
    }
}

void SearchList::SetSearchEdit(QWidget *edit)
{
    _search_edit = edit;
}

void SearchList::waitPending(bool pending)
{
    if (pending){
        _loadingDialog = new LoadingDlg(this);//这里忘记写this，没有父类，导致内存泄漏；走到else里就会访问无效内存。Qt对于程序异常，基本无法调试。
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _send_pending = pending;
    }
    else{
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _send_pending = pending;
    }
}

void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem();
    //qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250,10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp,invalid_item);

    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item,add_user_item);
}



void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); //获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase，然后根据类型确定是chat条目还是contact条目等，一步步到子类
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }

    if(itemType == ListItemType::ADD_USER_TIP_ITEM){
        if (_send_pending){
            return;
        }

        if (!_search_edit){
            return;
        }
        waitPending(true);
        auto search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);//把搜索栏转换成我们自定义的CustomizeEdit类型
        auto uid_str = search_edit->text();//搜索用户时填的信息可能是uid或name，我们这里都叫uid，到服务器在判断是哪种类型处理
        QJsonObject jsonObj;
        jsonObj["uid"] = uid_str;
        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);//compact是把QJsonDocument压缩，节省空间
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,jsonData);

        return;
    }

    //清除弹出框
    CloseFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    waitPending(false);
    if (si == nullptr){   //搜索不到该用户信息，弹出搜索失败界面FindFailDlg
        _find_dlg = std::make_shared<FindFailDlg>(this);//_find_dlg我们定义的是QDialog的基类指针，所以可以接受各种派生类指针
    }else{
        //查到了用户，两种情况，已经是好友；还不是好友
        //已经是好友  todo...

        //还不是好友
        _find_dlg = std::make_shared<FindSuccessDlg>(this);// _find_dlg（基类智能指针）转换为 FindSuccessDlg 类型的智能指针       总结：_find_dlg先初始化在再转换
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);//将 _find_dlg 转换为 FindSuccessDlg 类型,然后才能调用子类的成员函数
    }
    _find_dlg->show();//无论查找成功失败，都show出来。基类调子类的show
}
