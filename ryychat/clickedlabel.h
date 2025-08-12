#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H
#include <QLabel>
#include "global.h"



class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget *parent = nullptr);
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    void SetState(QString normal = "",QString hover = "",QString press = "",
                  QString select = "",QString select_hover = "",QString select_press = "");
    ClickLbState GetCurState();
    bool SetCurState(ClickLbState state);
    void ResetNormalState();

private:
    //总共六种状态：前三种是正常状态，代表闭眼（密码不可见）；又细分了三种：普通、悬浮和点击。后三种是选择状态，代表密码可见。
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;

signals:
    void clicked(QString,ClickLbState);
};

#endif // CLICKEDLABEL_H
