#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H
#include <QFrame>
#include "global.h"
#include <QHBoxLayout>

//聊天气泡的基类，后面气泡有文本、图片、文件等都在这个基类上扩展
class BubbleFrame:public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);
    void setMargin(int margin);
    //inline int margin(){return margin;}
    void setWidget(QWidget *w);//文本还是图片等。。
protected:
    void paintEvent(QPaintEvent *e);//绘制特定气泡样式
private:
    QHBoxLayout *m_pHLayout;
    ChatRole m_role;
    int m_margin;
};

#endif // BUBBLEFRAME_H
