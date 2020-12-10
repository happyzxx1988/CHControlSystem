#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "mainwindow.h"
#include "dataoper.h"

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private slots:
    void on_LoginBtn_clicked();

    void on_LoginResetBtn_clicked();

private:
    Ui::Login *ui;
    MainWindow *m;
    DataOper dataOper;
};

#endif // LOGIN_H
