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



    //只运行一次
    QSystemSemaphore ss("MyObjectFueRod",1,QSystemSemaphore::Open);
    ss.acquire();// 在临界区操作共享内存   SharedMemory
    QSharedMemory mem("MySystemObjectFueRod");// 全局对象名
    if (!mem.create(1))// 如果全局对象以存在则退出
    {
        QMessageBox::information(0, ("警告"), ("程序已经运行，无需再行启动。"));
        ss.release();// 如果是 Unix 系统，会自动释放。
        return 0;
    }
    ss.release();// 临界区


    Login w;

    w.show();

    return a.exec();
}
