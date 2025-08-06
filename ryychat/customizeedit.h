#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H
#include <QLineEdit>
#include <QDebug>

class CustomizeEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget *parent = nullptr);
    void SetMaxLength(int maxlen);
protected:
    void focusOutEvent(QFocusEvent *event) override;//执行失去焦点时的处理逻辑
private:
    void limitTextLength(QString text);

    int _max_len;
signals:
    void sig_focus_out();
};

#endif // CUSTOMIZEEDIT_H
