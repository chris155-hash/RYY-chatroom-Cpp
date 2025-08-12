#include "clickedlabel.h"
#include <QMouseEvent>


ClickedLabel::ClickedLabel(QWidget *parent):QLabel (parent),_curstate(ClickLbState::Normal)
{
    this->setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        if (_curstate == ClickLbState::Normal){
            qDebug() << "clicked ,change to selected hover:" << _selected_hover;
            _curstate = ClickLbState::Selected;
            setProperty("state",_selected_press);
            repolish(this);
            update();  //最好手动刷新一下，我们自己写的按钮事件
        }
        else{
            qDebug() << "clicked ,change to normal hover:" << _selected_hover;
            _curstate = ClickLbState::Normal;
            setProperty("state",_normal_press);
            repolish(this);
            update();
        }
        return;
    }
    //调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        if (_curstate == ClickLbState::Normal){
            qDebug() << "clicked ,change to selected hover:" << _selected_hover;
            setProperty("state",_normal_hover);
            repolish(this);
            update();  //最好手动刷新一下，我们自己写的按钮事件
        }
        else{
            qDebug() << "clicked ,change to normal hover:" << _selected_hover;
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(),_curstate);
        return;
    }
    //调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

void ClickedLabel::enterEvent(QEvent *event)
{
    //这里处理鼠标悬停进入的逻辑
    if (_curstate == ClickLbState::Normal){
        qDebug() << "enter ,change to normal hover:" << _normal_hover;
        setProperty("state",_normal_hover);  //加载qss里的不同图片
        repolish(this);
        update();
    }
    else{
        qDebug() << "enter ,change to selected hover:" << _selected_hover;
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }
    QLabel::enterEvent(event);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    //处理鼠标离开悬浮的逻辑
    if (_curstate == ClickLbState::Normal){
        qDebug() << "leave ,change to normal:" << _normal;
        setProperty("state",_normal);
        repolish(this);
        update();
    }
    else{
        qDebug() << "leave ,change to selected:" << _selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    //后面去找对应qss的实参
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

ClickLbState ClickedLabel::GetCurState()
{
    return _curstate;
}

bool ClickedLabel::SetCurState(ClickLbState state)
{
    this->_curstate = state;
    if (_curstate == ClickLbState::Normal){
        setProperty("state",_normal);
        repolish(this);
    }
    else if (_curstate == ClickLbState::Selected){
        setProperty("state",_selected);
        repolish(this);
    }
    return true;
}

void ClickedLabel::ResetNormalState()
{
    _curstate = ClickLbState::Normal;
    setProperty("state",_normal);
    repolish(this);
}
