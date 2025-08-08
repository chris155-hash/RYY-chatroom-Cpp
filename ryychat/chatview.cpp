#include "chatview.h"
#include <QScrollBar>
#include <QEvent>
#include <QStyleOption>
#include <QPainter>

ChatView::ChatView(QWidget *parent)  : QWidget(parent),isAppended(false)
{
    //设置垂直布局MainLayout，和内部widget等边距为0
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    pMainLayout->setMargin(0);
    //里面在加滚动区域，命名chat_area
    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    pMainLayout->addWidget(m_pScrollArea);
    //滚动区域里添加显示聊天的widget->w,w里设置布局Layout->pVLayout_1
    QWidget *w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);//自动填充背景，配合伸缩因子扩充
    //pVLayout_1里再加一个Qwidget
    QVBoxLayout *pVLayout_1 = new QVBoxLayout();
    pVLayout_1->addWidget(new QWidget(), 100000);//10000是伸缩因子
    w->setLayout(pVLayout_1);
    m_pScrollArea->setWidget(w);

    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//滚动条不显示
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();//滚动条垂直布局
    connect(pVScrollBar, &QScrollBar::rangeChanged,this, &ChatView::onVScrollBarMoved);//连接滚轮滚动范围变化-》滚动条下滑槽函数

    //pHLayout_2和pHLayout_1平级，显示滚动条的水平布局。把垂直ScrollBar放到上一层图像 而不是原来的并排在一个layout里
    QHBoxLayout *pHLayout_2 = new QHBoxLayout();
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
    pHLayout_2->setMargin(0);
    m_pScrollArea->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);//允许m_pScrollArea滚动区域随着控件大小变化而变化
    m_pScrollArea->installEventFilter(this);
    initStyleSheet();
}

void ChatView::appendChatItem(QWidget *item)
{
    QVBoxLayout *v1 = qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());
    v1->insertWidget(v1->count() - 1,item);//插入item。v比如一开始就一个w的layout，count是1，就在0的位置插入新聊天消息widget
    isAppended = true;
}

void ChatView::prependChatItem(QWidget *item)
{

}

void ChatView::insertChatItem(QWidget *before, QWidget *item)
{

}

bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::Enter && o == m_pScrollArea)
    {
        m_pScrollArea->verticalScrollBar()->setHidden(m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }
    else if(e->type() == QEvent::Leave && o == m_pScrollArea)
    {
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);//调用下基类的处理
}

void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatView::onVScrollBarMoved(int min, int max)//滚动条上下移动槽函数（滑块）
{
    if (isAppended){//添加聊天记录item可能会调用多次
        QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        //500ms内可能调用多次
        QTimer::singleShot(500,[this](){
           isAppended = false;
        });
    }
}

void ChatView::initStyleSheet()
{

}
