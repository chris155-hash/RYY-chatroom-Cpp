#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [] (QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input){      //自己写的简单的加密算法，和密码的长度异或一下
    QString result = input;          //input只是可读的，我们复制一份操作
    int length = result.length();
    length = length % 255;  //将字符串长度限制在 0 到 254 的范围内，以确保 XOR 操作的结果仍然是一个合法的 Unicode 字符。
    for (int i = 0;i < length;i++){
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
    }
    return result;
};
