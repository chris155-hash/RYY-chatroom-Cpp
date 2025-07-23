#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
{
    _timer = new QTimer(this);

    connect(_timer,&QTimer::timeout,[this](){   //timeout就是下面_timer->start(timeout)传进来的触发回调事件频率
       _counter --;
       if (_counter <= 0){
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
    if (e->button() == Qt::LeftButton){    //按下的是鼠标左键
        qDebug() << "MyButton was realesed!" ;
        this->setEnabled(false);//禁用了当前按钮，使其不再响应用户输入。
        this->setText(QString::number(_counter));
        _timer->start(1000);  //这里是指每一秒触发一次
        emit clicked();// 通知其他对象按钮被点击了，不影响原本被点击后的功能
    }
    //调用基类的mouseRealeseEvent以确保正常的时间处理（比如点击效果），有点像hook是不是，先执行了我们的逻辑，又去调用原始的逻辑
    QPushButton::mouseReleaseEvent(e);
}
