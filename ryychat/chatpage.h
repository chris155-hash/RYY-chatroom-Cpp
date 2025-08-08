#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
protected:
    void paintEvent(QPaintEvent *event);//ChatPage是继承自QWidget，他没有设置背景颜色，加载图片喷绘等。想实现样式更新，需要重写paintEvent
private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;
};

#endif // CHATPAGE_H
