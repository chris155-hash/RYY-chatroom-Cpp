#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "littlesurprisedialog.h"
#include "chatdialog.h"

/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      主窗口 Function
 *
 * @author     ryy任阳阳
 * @date       2025/07/02
 * @history
 *****************************************************************************/
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLogin2();
    void SlotSwitchLittleSurprise();//切换到彩蛋界面的槽函数
    void SlotSwitchLogin3();//彩蛋界面切换回登录界面的槽函数
    void SlotSwitchChat();
private:
    Ui::MainWindow *ui;
    LoginDialog * _login_dlg;
    RegisterDialog * _reg_dlg;
    ResetDialog * _reset_dlg;
    LittleSurpriseDialog * _little_surprise_dlg;
    ChatDialog* _chat_dlg;
};
#endif // MAINWINDOW_H
