### 一、前端界面的Qt客户端设计
- 1.`忘记密码`按钮
    - 1）鼠标悬浮到上面的时候像之前`密码是否可是选项`一样，触发悬浮效果。
    - 2）点击之后触发`点击信号`，相应触发`忘记密码`的槽函数。
    ```cpp
    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    ```
    - 3）这个slot_forget_pwd的槽函数发出`切换到重置密码界面`的信号给MainWindow，然后MainWindow触发`切换到重置密码`界面的槽函数。这样做的原因还是`面向对象`，如果上一步直接发出切换界面为`忘记密码`，就是`面向过程`编程。`切换页面是MainWindow的事情`，各个模块的分工要明确，才能写出好的代码。
    ```cpp
    void MainWindow::SlotSwitchReset()
    {
        //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
        _reset_dlg = new ResetDialog(this);
        _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
        setCentralWidget(_reset_dlg);

    _login_dlg->hide();
        _reset_dlg->show();
        //注册返回登录信号和槽函数
        connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
    }
    ```
- 2.ResetDialog类
    - 这个重置密码的类做的事情和注册界面差不多，1.检查用户输入信息正确与否；2.发送验证码功能需求--发出`get_varifycode`的Post请求，处理回包；3.发送重置密码功能需求--发出`reset_pwd`的Post请求。
    - 1）检查用户信息的代码直接复用注册界面的即可；新增下发出`reset_pwd`的Post请求的代码。发出json对象：
    ```cpp
    //发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pwd_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/reset_pwd"),
                                        json_obj, ReqId::ID_RESET_PWD,Modules::RESETMOD);
    ```
    - 老样子，发出Post请求的时候使用我们的HttpMgr来封装，四个参数1.`Post请求url`，后面服务器也是根据这个url才知道怎么处理这个Post请求；2.json_obj对象，发送的信息主体相当于，重置密码需要的信息；3+4.这次功能的请求Id和对应的相应模块，之前说过`方便服务器回包的时候通知客户端的谁来处理`。
- 3.发送讲完了，讲一下服务器接收回包发生的事。
    - 1）上面调用了`PostHttpReq`,他会处理接受的响应，无论是否成功都发出`sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod)`信号，然后这边相应模块就会触发槽函数处理。
    - 2）那这里`sig_http_finish`和`slot_reset_mod_finis`在哪里连接的呢？当然是在ResetDialog的构造里。他一开始就要处理好自己的业务逻辑（连接Http信号的发送和接受）。至于页面切换的按钮（只是触发切换页面的信号--后面就是MainWindow的事了）和切换页面的槽函数（MainWindow关心）其实是MainWindow的任务，分工要明确。
    ```cpp
    //连接reset相关信号和注册处理回调
    initHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish, this,
            &ResetDialog::slot_reset_mod_finish);
    ```
    - 这里`initHandlers`里注册回包的处理逻辑，失败就打印`“参数错误”`，成功就打印`“重置密码成功”`。
    - 谁去调用这个`initHandlers`的处理逻辑呢？`slot_reset_mod_finish`，也就是服务器回包后的响应函数。
    ```cpp
        void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
    {
        if(err != ErrorCodes::SUCCESS){
            showTip(tr("网络请求错误"),false);
            return;
        }
        // 解析 JSON 字符串,res需转化为QByteArray
        QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
        //json解析错误
        if(jsonDoc.isNull()){
            showTip(tr("json解析错误"),false);
            return;
        }
        //json解析错误
        if(!jsonDoc.isObject()){
            showTip(tr("json解析错误"),false);
            return;
        }
        //调用对应的逻辑,根据id回调。
        _handlers[id](jsonDoc.object());
        return;
    }
    ```
    - 这里可以看到，其实错误情况基本处理完了，能走到`_handlers`的处理基本没错误了，`一般不会打印“参数错误”`。处理错误情况，报错，然后调用对应的逻辑,`根据id回调_handlers`。这里就很像`服务器`的`HttpConnection（slot_reset_mod_finis）`和`LogicSystem（_handlers）`其实，只是客户端吗，没细分一个LogicSystem类，因为`客户端根据不同模块做不同处理，每个模块都相当于一个LogicSystem`。
### 二、服务器添加处理reser_pwd的Post请求处理
- 1.HttpConnection的处理不用管，下层处理都一样的，处理成功以后去调用LogicSystem的id对应的回调。在LogicSystem里添加reser_pwd对应的回调即可。
- 2.`RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {})`,这个回调要做的事
    - 1）json对象解析翻译回string之类的；
    - 2）去看redis里用户邮箱的验证码是否匹配；
    - 3）去看MySql里用户邮箱和用户名是否匹配，你不能乱改别人的密码对吧；
    - 4）都没问题就去MySql里重置密码。
- 3.上面设计MySql操作的，都是用之前封装好的`MySqlMgr`来调用，`MySqlMgr`作为服务层，调用`MySqlDao`操作层去`操作MySql库`，会从`MySqlPool`取连接,再在MySql服务器上增删改查。
- 4.看一下MySql_Dao的`UpdatePwd`操作举个例子吧。这种`简单的增删改查功能也不需要在MySql服务器上封装额外函数`，直接调用即可。
    ```cpp
    bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
        auto con = pool_->getConnection();
        try {
            if (con == nullptr) {
                pool_->returnConnection(std::move(con));
                return false;
            }

            // 准备查询语句
            std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

            // 绑定参数
            pstmt->setString(2, name);
            pstmt->setString(1, newpwd);

            // 执行更新
            int updateCount = pstmt->executeUpdate();

            std::cout << "Updated rows: " << updateCount << std::endl;
            pool_->returnConnection(std::move(con));
            return true;
        }
        catch (sql::SQLException& e) {
            pool_->returnConnection(std::move(con));
            std::cerr << "SQLException: " << e.what();
            std::cerr << " (MySQL error code: " << e.getErrorCode();
            std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
            return false;
        }
    }
    ```
### 三、补充
-1. 之前写过信号和槽函数的C++实现思路，模仿_handler,由Id去调回调。现在随着学习深入，发现不是这么一回事。
    ```cpp
    bool QObject::connect(const QObject *sender, PointerToMemberFunction signal,
                      const QObject *receiver, PointerToMemberFunction slot,
                      Qt::ConnectionType type = Qt::AutoConnection);
    ```
    - 观察connect这个函数，负责连接信号和槽函数，信号已出发就会执行相应的槽函数。但是槽函数有的是需要输入参数的，比如上面的`ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)`，那它需要的三个参数谁给的呢，`信号`给的，去看下信号的定义`void sig_reset_mod_finish(ReqId id,QString res,ErrorCodes err)`。
    - 那如果用id去找回调的话，一是要遍历找到对应的回调；二是怎么传参？所以之前说的方法可能不太适合实现信号和槽函数。具体怎么做能更好地实现信号和槽函数，随着学习的深入，后续补充。
- 2.准备自己写一个彩蛋界面。
    - 自己写的过程发现一个问题，按钮类的只用写槽函数，没去写信号和连接，比如之前`注册界面`、`登录界面`的`确认`那妞，`返回`按钮等。
        - 在 Qt 中，如果你是通过 Qt Designer（.ui 文件） 添加的按钮，并且使用 setupUi() 加载界面，那么你可以不需要手写 connect()，只要按照以下格式定义槽函数即可：
        ```cpp
        private slots:
        void on_<objectName>_<signalName>(<parameters>);
        ```
    Qt 的元对象系统会在运行时自动帮你完成 connect()和click信号。
