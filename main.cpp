#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "mainwindow.h"
#include <QApplication>
#include "login.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font = a.font();
    font.setPointSize(10);//字体大小
    font.setFamily("Microsoft YaHei");//微软雅黑字体
    a.setFont(font);

    Login w;

    w.show();

    return a.exec();
}
