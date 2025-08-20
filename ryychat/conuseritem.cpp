#include "conuseritem.h"
#include "ui_conuseritem.h"

ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CONTACT_USER_ITEM);
    ui->red_point->raise();//红点设置到最上层，图层最上层，->不会被覆盖
    ShowRedPoint(false);
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const
{
    return QSize(250,70);//返回自定义的尺寸
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)//去看UserInfo的构造，可以接受一个<AuthInfo>类型的智能指针，把智能指针执行的内容拷贝下来给UserInfo初始化
{
    _info = std::make_shared<UserInfo>(auth_info);
    //加载图片
    QPixmap pixmap(_info->_icon);
    //图片等比例缩放;图片里的内容允许缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp)
{
    _info = std::make_shared<UserInfo>(auth_rsp);
    //加载图片
    QPixmap pixmap(_info->_icon);
    //图片等比例缩放;图片里的内容允许缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
    _info = std::make_shared<UserInfo>(uid,name,name,icon,0);//UserInfo构造至少五个参数
    //加载图片
    QPixmap pixmap(_info->_icon);
    //图片等比例缩放;图片里的内容允许缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::ShowRedPoint(bool show)
{
    if (show){
        ui->red_point->show();
        return;
    }
    else{
        ui->red_point->hide();
        return;
    }
}


