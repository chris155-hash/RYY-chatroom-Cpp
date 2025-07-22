#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [] (QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input){      //自己写的简单的加密算法，和密码的长度异或一下
    QString result = input;          //input只是可读的，我们复制一份操作
    int length = result.length();
    length = length % 255;
    for (int i = 0;i < length;i++){
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
    }
    return result;
};
