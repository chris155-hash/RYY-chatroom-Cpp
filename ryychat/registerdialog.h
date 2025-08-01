#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void on_sure_btn_clicked();

    void on_return_btn_clicked();
    void on_cancel_btn_clicked();

private:
    void initHttpHandler();
    void showTip(QString str,bool b_ok);

    void changeTipPage();
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVarifyValid();
    bool check_little_surprise();//触发彩蛋函数
    QMap<TipErr,QString> _tip_errs;    //_tip_errs是一个map，存储所有的错误，密码错误，验证码错误啊等等

    Ui::RegisterDialog *ui;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;//通过不同的请求id，调用不同的函数方法；函数传参是QJsonObject

    QTimer *_countdown_timer;
    int _countdown;

signals:
    void sigSwitchLogin();
    void sigSwitchLittleSurprise();//触发彩蛋界面的信号
};

#endif // REGISTERDIALOG_H
