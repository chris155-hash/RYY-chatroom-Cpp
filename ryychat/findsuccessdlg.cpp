#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include <QDir>
#include "findsuccessdlg.h"
#include "applyfriend.h"

FindSuccessDlg::FindSuccessDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindSuccessDlg),_parent(parent)
{
    ui->setupUi(this);
    //设置对话框标题
    setWindowTitle("添加");
    //dialog类型的对话框有最大化最小化等按钮，隐藏掉
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);//windowFlags窗口现有的标志位集合，与无边框的掩码位运算后无边框标志位为1，表示无边框
    //获取当前应用程序的路径。因为后面搜索到别人的信息、头像要从服务器下载到本地在加载进来。
    QString app_path = QCoreApplication::applicationDirPath();
//    qDebug() << "app_path is :" << app_path;
    QString pix_path = QDir::toNativeSeparators(app_path + QDir::separator() + "static"+QDir::separator()+"head_66.jpg");
//    qDebug() << "pix_path is :" << pix_path;
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_lb->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    ui->add_friend_btn->SetState("normal","hover","press");
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg()
{
    delete ui;
}

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_lb->setText(si->_name);
    _si = si
            ;}

void FindSuccessDlg::on_add_friend_btn_clicked()
{
    //todo... 添加好友界面弹出
    this->hide();
    //弹出加好友界面
    auto applyFriend = new ApplyFriend(_parent);
    SetSearchInfo(_si);
    setModal(true);
    applyFriend->show();
}
