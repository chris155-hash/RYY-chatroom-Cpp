#include "listitembase.h"
#include <QStyleOption>
#include <QPainter>

ListItemBase::ListItemBase(QWidget *parent):QWidget(parent)
{

}

void ListItemBase::SetItemType(ListItemType itemType)
{
    _itemType = itemType;
}

ListItemType ListItemBase::GetItemType()
{
    return _itemType;
}

void ListItemBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt; // 1. 创建一个样式选项对象，用于保存当前控件的绘制参数
    opt.initFrom(this);// 2. 用当前控件(this)的状态、矩形、调色板等信息初始化 opt
    QPainter p(this);// 3. 在当前控件上创建 QPainter，准备绘图
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//QWIdget的Start传入 opt 提供几何/调色板，p 提供画笔，this 指定目标控件
}
