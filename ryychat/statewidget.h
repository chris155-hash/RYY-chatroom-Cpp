#ifndef STATEWIDGET_H
#define STATEWIDGET_H
#include <qwidget>
#include <QLabel>
#include "global.h"


//最左边的状态栏（手写ui），有消息显示红点等
class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void SetState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    ClickLbState GetCurState();
    void ClearState();

    void SetSelected(bool bselected);
    void AddRedPoint();
    void ShowRedPoint(bool show=true);

protected:
    void paintEvent(QPaintEvent* event);

    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:

    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;//红点状态，normal和selected。又细分为各三种，同密码可视化那里。
    QLabel * _red_point;//红点的Qlabel对象

signals:
    void clicked(void);

signals:

public slots:
};

#endif // STATEWIDGET_H
