### 一、功能完善
- 1.之前的密码是明文传输的，这样做有两个风险：1）如果传输过程被别人捕获了就会直接把用户信息泄露出去；2）MySql服务器里存储的也是明文密码，如果MySql服务器被黑客爆破之后，就可以直接看到邮箱和密码等等。如果恰好用户的密码和银行卡密码一致，从这里泄露了用户的个人信息要付相应的法律责任。
- 2.实现：自己写了一个简单的异或算法，和自己的密码长度进行异或。虽然简单，但只是象征性的加密一下，也可以采用更复杂的哈希算法等密码学的加密算法，这个不作为本项目的重点，所以这里只是简单的写一下。
    ```cpp
    std::function<QString(QString)> xorString = [](QString input){      //自己写的简单的加密算法，和密码的长度异或一下
        QString result = input;          //input只是可读的，我们复制一份操作
        int length = result.length();
        length = length % 255;//将字符串长度限制在 0 到 254 的范围内，以确保 XOR 操作的结果仍然是一个合法的 Unicode 字符。
        for (int i = 0;i < length;i++){
            result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
        }
        return result;
    };
    ```
    - 写成function是为了让加密算法像变量一样被传递或替换，方便 回调 / 注册 / 传递。
- 服务器端做同样的操作即可恢复出原始的密码。
### 二、注册界面美化
- 1.击获取验证码的按钮以后，按钮变成倒计时。设置10秒，也就是10秒之后才能再次点击获取按钮。增加这样一个小插件。
    - 1）新建一个TimeBtn类，初始化一个定时器，每个1s触发一次，打印计数器数值，如果倒计时为0，停止计时器，重新显示获取按钮。
    - 2）那么这个类的两个私有成员变量：`QTimer  *_timer`;`int _counter`，一个定时器和一个计数器。
    - 3）构造函数：我们让这个按钮是从QPushButton继承来的，那就把基类的构造稍作修改。
        ```cpp
        TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
        ```
        - 基类的构造`QPushButton(parent)`，用于指定当前窗口部件（QWidget 或其子类）的父窗口部件，说明这是个子窗口。初始化计数器数值为10。
        - 构造函数具体：new出一个计时器，connect连接超时触发信号和触发函数。触发函数这里直接用lambda表达式写，不重新写一个函数了，毕竟只在这里一次性用一下。
        - 触发函数：收到触发信号后触发，触发信号每1s触发一次，那他就先看计数器到0没有，到了就重置计数器为10，停止定时器，让按钮处于可点击状态，显示`“获取”`文字。
        - 触发信号：由鼠标点击函数触发，没触发一次就相当于触发一次触发函数。
        - 鼠标点击函数：`void mouseReleaseEvent(QMouseEvent *e) override;`重写了鼠标点击函数，动态多态。如果鼠标点击左键后：先设置按钮为不可点击，显示计数器数值，地洞定时器的触发功能`_timer->start(1000);`表示每1s触发一次timeout信号，从而触发上面的触发函数。`emit clicked();`在触发一次点击信号，让原本点击按钮后会触发的槽函数正常工作。最后，还要再执行一下基类的`QPushButton::mouseReleaseEvent(e);`鼠标点击事件，他原本会做的功能我们不动。
            - 这个是不是有点像hook，先执行了我们自定义的逻辑（子类重写），在里面重新去调用积累的实现。
        ```cpp
        TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
        {
            _timer = new QTimer(this);

            connect(_timer, &QTimer::timeout, [this](){
                _counter--;
                if(_counter <= 0){
                    _timer->stop();
                    _counter = 10;
                    this->setText("获取");
                    this->setEnabled(true);
                    return;
                }
                this->setText(QString::number(_counter));
            });
        }

        TimerBtn::~TimerBtn()
        {
            _timer->stop();
        }

        void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
        {
            if (e->button() == Qt::LeftButton) {
                // 在这里处理鼠标左键释放事件
                qDebug() << "MyButton was released!";
                this->setEnabled(false);
                this->setText(QString::number(_counter));
                _timer->start(1000);
                emit clicked();
            }
            // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
            QPushButton::mouseReleaseEvent(e);
        }
        ```
- 2.错误提示优化
    - 1）用户输入完信息后，鼠标离开该信息行就直接触发检查函数，检查输入的内容是否合法，如果不合法直接弹出错误提示文本。加载注册界面的默认构造里，直接初始化就连接好信号和检测函数。
    - 2）用户点击`确定`按钮时，检查所有信息是否满足要求后在发送Http请求，不满足就直接返回，不会发送Http请求，同时`检查是否合法的函数`会显示`错误信息`，提示用户修改。
    - 3）直接看代码讲吧：
        ```cpp
        ui->err_tip->clear();

        connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
            checkUserValid();
        });
        ```
    - 拿检查用户名的函数举个例子，err_tip是前端界面de错误提示处。Qt的信号和槽机制：连接connect(连接对象，信号，对象，触发函数)，对于用户名这一栏，当`鼠标离开用户名`或按下`enter`这一栏就会触发`editingFinished信号`,信号就会触发相应槽函数，`checkUserValid()`是我们自己写的`检查用户名是否合法的函数`。
    - `checkUserValid()`：检测不合法就打印错误提示，把错误信息添加到`_err_tips`里，等合法以后再从里面删除。
        ```cpp
        bool RegisterDialog::checkUserValid()   //验证用户名是否合法（不为空）
        {
            if (ui->user_edit->text() == ""){
                AddTipErr(TipErr::TIP_USER_ERR,tr("用户名不能为空"));
                return false;
            }

            DelTipErr(TipErr::TIP_USER_ERR);
            return true;
        }
        ```
        - `AddTipErr`接受的两个参数，第一个`TipErr`枚举的一系列错误类型，方便后期运维定位错误，第二个显示到err_tip的错误提示。较为简单，不贴代码了。
        - `QMap<TipErr,QString> _tip_errs;`错误集合用Qt的QMap结构，存储所有的错误，比如信息1和2都错了，用户改了2的错误，这时候还要显示回1的错误。所以这里我做了一些小修改
        - `DelTipErr函数`，新增了还有错误时显示上一个错误。这里我原本写的是_tip_errs.end().value()，运行一会就回崩溃。改成_tip_errs.last()就好了。
            - 补充：end() 返回的是一个“哨兵”迭代器，不能对其进行解引用或调用。 指向的是列表末尾的下一个位置，而不是最后一个元素本身。就比如Std::map的back().second，而不是end().second()。
            ```cpp
            void RegisterDialog::DelTipErr(TipErr te){
                _tip_errs.remove(te);
                if (_tip_errs.empty()){
                    ui->err_tip->clear();
                    return;
                }
                else{
                    showTip(_tip_errs.last(),false);
                }
            }
            ```
- 3.密码可控显示与否：前端确实比后端麻烦太多了，光一个小功能就又要加前端图标，加label，加鼠标事件，信号，槽函数，连接，再根据不同状态不同处理。麻烦得很。
    - 1）加图标，加label。输入密码时希望能通过点击可见还是不可见，显示密码和隐藏密码，这里先添加图片放入资源中，然后在Register.ui中添加两个label，分别命名为pass_visible和confirm_visible, 用来占据位置。
    - 2）一个Label有六种状态，普通状态，普通的悬浮状态，普通的点击状态，选中状态，选中的悬浮状态，选中的点击状态。当Label处于普通状态，被点击后，切换为选中状态，再次点击又切换为普通状态。大的状态分两种：1.普通（默认密码模式，密码不可见）；2.选中模式（密码可见模式）。
    - 3）六种状态对应的图片在qss中链接好，以此实现切换。但实际上就四大类，另外两类置为了空。（因为点击就切换了，所以两个点击的状态其实不需要）
    - 4）过于复杂，直接看代码。总的来说，分两大类：normal和select，然后根据鼠标是否悬浮又细分了_normal和_normal_hover;_select和_select_hover。具体代码实现这里就不展示了。
    ```cpp
    class ClickedLabel:public QLabel
    {
        Q_OBJECT
    public:
        ClickedLabel(QWidget* parent);
        virtual void mousePressEvent(QMouseEvent *ev) override;//切换打的两类normal<->select的显示qss,这里就不处理密码显示与否的切换了，放在后面连接点击信号再出发槽函数那里
        virtual void enterEvent(QEvent* event) override;//进入hover状态的qss
        virtual void leaveEvent(QEvent* event) override;//离开hover，进入普通状态的qss
        void SetState(QString normal="", QString hover="", QString press="",
                    QString select="", QString select_hover="", QString select_press="");//设置qss对应实参的函数，这样方便外面主体调用的时候通过这个接口改参数就能改对应的图片。一开始感觉绕了半天，因为定下这个图片我就不改了，为什么不直接定死。      还是那句话，面向对象，提高复用性！！！！！

        ClickLbState GetCurState();//提供一个接口，获取现在的状态。后面点击触发的槽函数会根据当前状态切换密码显示与否。
    protected:
    private://qss的形参，后面被SetState赋予实参。
        QString _normal;
        QString _normal_hover;
        QString _normal_press;

        QString _selected;
        QString _selected_hover;
        QString _selected_press;

        ClickLbState _curstate;
    signals:
        void clicked(void);//点击信号，上面的函数只实现qss样式切换，这个信号才触发实际功能。（真是严谨啊，模块之间功能分明，这就是严谨的代码！！！！）
    };
    ```
    - 5）然后在RegisterDialog的构造函数中添加label点击的响应函数。
    ```cpp
    //设置浮动显示手形状
    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->confirm_visible->setCursor(Qt::PointingHandCursor);

    ui->pass_visible->SetState("unvisible","unvisible_hover","","visible",
                                "visible_hover","");

    ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                    "visible_hover","");
    //连接点击事件

    connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }else{
                ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }else{
                ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });
    ```
    - 一看既明，不多讲了。前端的就是麻烦，但是逻辑很简单。
### 三、注册见面2--注册成功界面
- 1.ui设计--不多赘述
- 2.initHandlers函数内实现--收到服务器注册回复的请求里---增加界面切换函数调用。
    - 原本只是收到注册成功消息后就提示“注册成功”，现在新增跳转到注册界面2。
    ```cpp
    void RegisterDialog::ChangeTipPage()
    {
        _countdown_timer->stop();
        ui->stackedWidget->setCurrentWidget(ui->page_2);

        // 启动定时器，设置间隔为1000毫秒（1秒）
        _countdown_timer->start(1000);
    }
    ```
- 3.注册界面2会每个1s触发一次timeout信号（上面的定时器信号），提示“还有多少秒返回登录界面“；以及一个直接返回登录界面的按钮。
- 4.返回按钮的触发信号，槽函数，以及连接等等不再多说。至此也算很详细了。