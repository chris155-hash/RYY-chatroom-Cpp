#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //加载qss，风格
    QFile qss(":/style/stylesheet.qss");
    if (qss.open(QFile::ReadOnly)){
        qDebug("Open Success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else{
        qDebug("QFile fail");
    }
    //加载客户配置(写在config.ini文件里)。读取文件拿出指定字段，拼接成GateServer的url
    QString fileName = "config.ini";
    QString app_path = QCoreApplication::applicationDirPath();//返回应用程序的运行目录路径
    QString config_path = QDir::toNativeSeparators(app_path + QDir::separator() + fileName);//QDir::separator()分隔符，linux里/，windows里右斜杠
    QSettings settings(config_path,QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;


    MainWindow w;
    w.show();
    return a.exec();
}
