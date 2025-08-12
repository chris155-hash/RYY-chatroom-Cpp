#include "clickedoncelabel.h"

ClickedOnceLabel::ClickedOnceLabel(QWidget *parent): QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        emit clicked(this->text());
        return;
    }
    //我们自定义的ClickedOnceLabel只处理鼠标左键点击事件，其余的还是按照基类的处理
    QLabel::mouseReleaseEvent(event);
    //QLabel::mousePressEvent(event;
}
