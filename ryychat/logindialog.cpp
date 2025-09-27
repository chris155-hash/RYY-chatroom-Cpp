#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>
#include <QPainter>
#include "httpmgr.h"
#include "tcpmgr.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->reg_btn,&QPushButton::clicked,this,&LoginDialog::switchRegister);

    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");

    connect(ui->forget_label,&ClickedLabel::clicked,this,&LoginDialog::slot_forget_pwd);
    initHead();//主页图片headlabel
    initHttpHandlers();
    //连接HttpMgr发来的处理用户登录的回包信号
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_login_mod_finish,this,&LoginDialog::slot_login_mod_finish);//GetInstance().get()把智能指针里的裸指针取出来。有的旧代码只接受裸指针
    ui->err_tip->clear();

    //连接tcp连接请求的信号和槽函数
    connect(this,&LoginDialog::sig_connect_tcp,TcpMgr::GetInstance().get(),&TcpMgr::slot_tcp_connect);
    //连接tcp管理者发来的链接成功信号
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_con_success,this,&LoginDialog::slot_tcp_con_finish);
    //连接tcp管理者发来的登录失败信号
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_login_failed,this,&LoginDialog::slot_login_failed);

}


LoginDialog::~LoginDialog()
{
    qDebug() << "LoginDialog Destruct";
    delete ui;
}

void LoginDialog::initHead()//初始化登录界面的图片
{
    //加载图片
    QPixmap originalPixmap(":/res/head_1.jpg");
    //设置图片自动缩放
    qDebug() << originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
            Qt::KeepAspectRatio,Qt::SmoothTransformation);
    //创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent);//用于透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);//设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0,0,originalPixmap.width(),originalPixmap.height(),10,10);
    painter.setClipPath(path);

    //将原始图片绘制成roundedPixmap上
    painter.drawPixmap(0,0,originalPixmap);

    //设置绘制好的圆角图片到QLabel上
    ui->head_label->setPixmap(roundedPixmap);
}

bool LoginDialog::checkEmailValid()
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

bool LoginDialog::checkPwdValid()//为什么又要判断密码是否合法？当然可以直接去MySql判断是否匹配，但是这不是平白增加MySql的压力嘛！！！
{
    auto pwd = ui->pass_edit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pass length invalid";
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pwd).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符且长度为(6~15)"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

void LoginDialog::AddTipErr(TipErr te,QString tips){
    _tip_errs[te] = tips;
    showTip(tips,false);
}

void LoginDialog::DelTipErr(TipErr te){
    _tip_errs.remove(te);
    if (_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.last(), false);
}

void LoginDialog::showTip(QString str,bool b_ok)
{
    if (b_ok){
        ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state","err");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

bool LoginDialog::enableBtn(bool enabled){
    ui->login_btn->setEnabled(enabled);
    ui->reg_btn->setEnabled(enabled);
    return true;
}

void LoginDialog::initHttpHandlers()
{
    //auto self = shared_from_this(); 这里没有这样是因为构造函数会调用这个函数，说明类都还没构造完成
    _handlers.insert(ReqId::ID_LOGIN_USER,[this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();

        //根据GateServer返回的ChatServer的IP地址和Token信息，向目标聊天服务器，发送信号通知TcpMgr，让它发送长连接请求
        ServerInfo si;//存储聊天服务器信息的struct
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "email is " << email << " uid is " << si.Uid <<" host is "
                << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        emit sig_connect_tcp(si);//通知TcpMgr干活
        //这里为什么不直接发送tcp请求？   发送Tcp的可能是多线程，利用信号和槽的队列机制相当于保证了一个有序性
    });
}

void LoginDialog::slot_forget_pwd()
{
    qDebug() << "slot switch forget pwd";
    emit switchReset();
}

void LoginDialog::on_login_btn_clicked()//void on_pushButton_clicked();Qt的PushButton只要按规定格式就可以省去自己连接、写信号的过程
{
    qDebug() << "login btn clicked";
    if (!checkEmailValid()){
        return;
    }

    if (!checkPwdValid()){
        return;
    }

    enableBtn(false);
    auto email = ui->email_edit->text();
    auto pwd = ui->pass_edit->text();
    //发送请求登录的Http请求
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj,ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        enableBtn(true);
        return;
    }
    //解析JSON 字符串，res 转化成QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()){
        showTip(tr("JSON解析失败"),false);
        enableBtn(true);
        return;
    }
    //JSON解析错误
    if (!jsonDoc.isObject()){
        showTip(tr("JSON解析失败"),false);
        enableBtn(true);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登录失败，err is: %1").arg(err);
    showTip(result,false);
    enableBtn(true);
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    if (bsuccess){
        showTip(tr("聊天服务器连接成功，正在登录..."),true);//这里连接成功是指qt和ChatServer连接上了，可以收发数据了。接下来发送用户登录的tcp请求
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        //把 QJsonObject 变成 QString 格式的漂亮 JSON 文本QJsonDocument
        QJsonDocument doc(jsonObj);
        QByteArray dataBytes = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给ChatServer，请求用户登录
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN,dataBytes);

    }
    else{
        showTip(tr("网络错误"),false);
        enableBtn(true);
    }
}
