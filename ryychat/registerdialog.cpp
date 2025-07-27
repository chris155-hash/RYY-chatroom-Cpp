#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),_countdown(5)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);//设置密码模式，即密码不可见
    ui->err_tip->setProperty("state","normal");
    repolish(ui->err_tip);
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,
            this,&RegisterDialog::slot_reg_mod_finish);
    initHttpHandler();

    ui->err_tip->clear();
    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->email_edit,&QLineEdit::editingFinished,this,[this](){
        checkEmailValid();
    });
    connect(ui->pass_edit,&QLineEdit::editingFinished,this,[this](){
        checkPassValid();
    });
    connect(ui->confirm_edit,&QLineEdit::editingFinished,this,[this](){
        checkConfirmValid();
    });
    connect(ui->varify_edit, &QLineEdit::editingFinished, this, [this](){
            checkVarifyValid();
    });


    //传入六种状态
    ui->pass_visible->SetState("unvisible","unvisible_hover","","visible",
                                "visible_hover","");      //传入qss实际对应的实参

    ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                    "visible_hover","");

    //点击密码是否可视之后触发槽函数。切换密码的显示模式。
    connect(ui->pass_visible,&ClickedLabel::clicked,this,[this](){
       auto state= ui->pass_visible->GetCurState() ;
       if (state == ClickLbState::Normal){
           ui->pass_edit->setEchoMode(QLineEdit::Password);
       }else{
           ui->pass_edit->setEchoMode(QLineEdit::Normal);
       }
       qDebug() << "Label was clicked !";
    });
    connect(ui->confirm_visible,&ClickedLabel::clicked,this,[this](){
       auto state= ui->confirm_visible->GetCurState() ;
       if (state == ClickLbState::Normal){
           ui->confirm_edit->setEchoMode(QLineEdit::Password);
       }else{
           ui->confirm_edit->setEchoMode(QLineEdit::Normal);
       }
       qDebug() << "Label was clicked !";
    });

    //创建定时器
    _countdown_timer = new QTimer(this);
    //连接信号和槽
    connect(_countdown_timer,&QTimer::timeout,[this](){
        if (_countdown == 0){
            _countdown_timer->stop();
            emit sigSwitchLogin();  //发送切换回登录界面的信号
            return;
        }
        _countdown --;
        auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        ui->tip_lb->setText(str);
    });

 }

void RegisterDialog::changeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    //启动定时器,每隔一秒去触发一次定时器的回调
    _countdown_timer->start(1000);
}

RegisterDialog::~RegisterDialog()
{
    qDebug() <<"RegisterDialog Destruct";
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
       changeTipPage();
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


void RegisterDialog::AddTipErr(TipErr te,QString tips){
    _tip_errs[te] = tips;
    showTip(tips,false);
}

void RegisterDialog::DelTipErr(TipErr te){
    _tip_errs.remove(te);
    if (_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.last(), false);
}

bool RegisterDialog::checkUserValid()   //验证用户名是否合法（不为空）
{
    if (ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR,tr("用户名不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()   //验证邮箱地址的正则表达式
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if (!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR,tr("邮箱地址不正确"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();

    if(pass.length() < 6 || pass.length()>15){     //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求 ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkVarifyValid()   //验证用户名是否合法（不为空）
{
    if (ui->varify_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR,tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}

bool RegisterDialog::check_little_surprise()
{
    auto pass = ui->pass_edit->text();
    if (pass == "520xcc") return true;
    return false;
}

bool RegisterDialog::checkConfirmValid(){
    if (ui->pass_edit->text() != ui->confirm_edit->text()){
        AddTipErr(TipErr::TIP_PWD_CONFIRM,tr("密码和确认密码不一致"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_CONFIRM);
    return true;
}


void RegisterDialog::on_sure_btn_clicked()
{
    //彩蛋界面
    bool caidan = check_little_surprise();
    if (caidan){
        qDebug() << "恭喜你触发了彩蛋界面!";
        emit sigSwitchLittleSurprise();//触发切换到彩蛋界面的信号
        return;
    }

    bool valid = checkUserValid();
    if(!valid){
        return;
    }

    valid = checkEmailValid();
    if(!valid){
        return;
    }

    valid = checkPassValid();
    if(!valid){
        return;
    }
    valid = checkConfirmValid();
    if(!valid){
        return;
    }

    valid = checkVarifyValid();
    if(!valid){
        return;
    }


    //发送Http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pass_edit->text());
    json_obj["confirm"] = xorString(ui->confirm_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj,ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer ->stop();
    emit sigSwitchLogin();//触发切换到登录界面的信号
}

void RegisterDialog::on_cancel_btn_clicked()
{
    _countdown_timer ->stop();
    emit sigSwitchLogin();//触发切换到登录界面的信号
}
