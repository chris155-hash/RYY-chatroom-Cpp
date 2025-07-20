#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);//设置密码模式，即密码不可见
    ui->err_tip->setProperty("state","normal");
    repolish(ui->err_tip);
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,this,&RegisterDialog::slot_reg_mod_finish);
    initHttpHandler();
 }

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.)(\w+)+)");
    bool match = regex.match(email).hasMatch();
    if (match){
     //发送http验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_varifycode"),
                                            json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);

    }else{
        showTip("邮箱地址不正确",false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }
    //解析JSON 字符串，res 转化成QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()){
        showTip(tr("JSON解析失败"),false);
        return;
    }
    //JSON解析错误
    if (!jsonDoc.isObject()){
        showTip(tr("JSON解析失败"),false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void RegisterDialog::initHttpHandler()
{
   //通过不同的RequestId调用不同的处理函数。存在_handlers的QMap里。RegisterDialog默认构造时初始化InitHttpHandler
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](const QJsonObject& jsonObj){
       int error = jsonObj["error"].toInt();
       if (error != ErrorCodes::SUCCESS){
           showTip(tr("参数错误"),false);
           return;
       }
       auto email = jsonObj["email"].toString();
       showTip(tr("验证码已发送至邮箱，请注意查收！"),true);
       qDebug() << "email is:" << email ;
    });
    //注册 用户注册的回包逻辑
    _handlers.insert(ReqId::ID_REG_USER,[this](QJsonObject jsonObj){
       int error = jsonObj["error"].toInt();
       if (error != ErrorCodes::SUCCESS){
           showTip(tr("参数错误"),false);
           return ;
       }
       auto email = jsonObj["email"].toString();
       showTip(tr("用户注册成功"),true);
       qDebug() << "user uid is " << jsonObj["uid"].toString();
    });
}

void RegisterDialog::showTip(QString str,bool b_ok)
{
    if (b_ok){
        ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state","err");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::on_sure_btn_clicked()
{
    if (ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"),false);
        return;
    }
    if (ui->email_edit->text() == ""){
        showTip(tr("邮箱不能为空"),false);
        return;
    }
    if (ui->pass_edit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }

    QRegularExpression re("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{6,13}$");
    if (!re.match(ui->pass_edit->text()).hasMatch()){
        showTip(tr("密码至少应该包含字母和数字，至少六位，至多13位"), false);
        return;
    }

    if (ui->confirm_edit->text() == ""){
        showTip(tr("确认密码不能为空"), false);
        return;
    }

    if (ui->pass_edit->text() != ui->confirm_edit->text()){
        showTip(tr("密码和验证密码不匹配"),false);
        return;
    }

    if (ui->varify_edit->text() == ""){
        showTip(tr("验证码不能为空"),false);
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = ui->pass_edit->text();
    json_obj["confirm"] = ui->confirm_edit->text();
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj,ReqId::ID_REG_USER,Modules::REGISTERMOD);
}
