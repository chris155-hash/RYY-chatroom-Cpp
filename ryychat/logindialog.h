#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void initHead();
    void initHttpHandlers();
    Ui::LoginDialog *ui;
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void showTip(QString str,bool b_ok);
    bool checkPwdValid();
    bool enableBtn(bool enabled);
    bool checkEmailValid();

    QMap<TipErr,QString> _tip_errs;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;//跟register那里一样，根据服务器回包的url调用不同处理
    int _uid;
    QString _token;
public slots:
    void slot_forget_pwd();
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
private slots:
    void on_login_btn_clicked();
    void slot_login_mod_finish(ReqId id,QString res,ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
    void slot_login_failed(int err);
};

#endif // LOGINDIALOG_H
