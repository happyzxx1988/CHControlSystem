#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "mainwindow.h"
#include "dataoper.h"
#include <QSettings>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    QSettings* init_mysql();

    bool db_mysqlcreateConnection();

private slots:
    void on_LoginBtn_clicked();

    void on_LoginResetBtn_clicked();

private:
    Ui::Login *ui;
    MainWindow *m;
    DataOper dataOper;
    QSettings* settings;
};

#endif // LOGIN_H
