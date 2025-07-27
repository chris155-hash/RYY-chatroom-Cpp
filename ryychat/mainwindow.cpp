#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "resetdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);//登录对话框的标题栏和边框全部去掉
    setCentralWidget(_login_dlg);
//    _login_dlg->show();

    //创建和注册消息链接
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);//切换到注册界面的信号，切换注册界面的槽函数。连接起来，信号触发->执行槽函数
    //链接登录界面忘记密码的信号和槽函数
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    _reg_dlg = new RegisterDialog(this);
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

    //连接注册界面返回登录信号
    connect(_reg_dlg,&RegisterDialog::sigSwitchLogin,this,&MainWindow::SlotSwitchLogin);  //返回登录界面的信号和槽函数

    //链接注册界面的触发彩蛋信号  和  MainWindow的彩蛋槽函数
    connect(_reg_dlg,&RegisterDialog::sigSwitchLittleSurprise,this,&MainWindow::SlotSwitchLittleSurprise);

    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

void MainWindow::SlotSwitchLogin()
{
    //旧的登录界面已经析构掉了，新建一个并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog();
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reg_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);//连接登录界面的注册信号和槽函数
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);//连接登录界面的忘记密码信号和槽函数
}

void MainWindow::SlotSwitchReset(){
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

    setCentralWidget(_reset_dlg);
    _login_dlg->hide();
    _reset_dlg->show();
    connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::SlotSwitchLogin2);
}

void MainWindow::SlotSwitchLogin2()
{
    _login_dlg = new LoginDialog();
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);//连接登录界面的注册信号和槽函数
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);//连接登录界面的忘记密码信号和槽函数
}

void MainWindow::SlotSwitchLittleSurprise()
{
    _little_surprise_dlg = new LittleSurpriseDialog(this);
    _little_surprise_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_little_surprise_dlg);

    _reg_dlg->hide();
    _little_surprise_dlg->show();
    //连接注册界面  返回登录界面的信号
    connect(_little_surprise_dlg,&LittleSurpriseDialog::sigSwitchLogin,this,&MainWindow::SlotSwitchLogin3);  //返回登录界面的信号和槽函数

}

void MainWindow::SlotSwitchLogin3()
{
    _login_dlg = new LoginDialog();
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _little_surprise_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);//连接登录界面的注册信号和槽函数
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);//连接登录界面的忘记密码信号和槽函数
}

