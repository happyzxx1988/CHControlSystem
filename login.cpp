#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "login.h"
#include "ui_login.h"
#include "connection.h"
#include "mainwindow.h"
#include <QMessageBox>


Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    if(!db_sqllitecreateConnection()){
        QMessageBox::information(this, tr("提示"), tr("数据库连接失败"),tr("确定"));
        return;
    }

    ui->logLabel->hide();
//    this->setWindowTitle("长虹智能空压站房系统");

}

Login::~Login()
{
    delete ui;
}
//登录
void Login::on_LoginBtn_clicked()
{
    QString u = ui->userName->text();
    QString p = ui->passWord->text();
    if(u.isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("账号不能为空"),tr("确定"));
        ui->userName->setFocus();
        return;
    }
    if(p.isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("密码不能为空"),tr("确定"));
        ui->passWord->setFocus();
        return;
    }
    QByteArray bytePwd = p.toUtf8();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    QString strPwdMd5 = bytePwdMd5.toHex();

    vector<User> users;
    dataOper.getAllUsersInfo(users,u);
    int dataSize = users.size();
    if(dataSize == 0){
        QMessageBox::information(this, tr("提示"), tr("用户名错误！"),tr("确定"));
        ui->userName->clear();
        ui->userName->setFocus();
        return;
    }else{
        if(users[0].password != strPwdMd5 ){
            QMessageBox::information(this, tr("提示"), tr("密码错误！"),tr("确定"));
            ui->passWord->clear();
            ui->passWord->setFocus();
            return;
        }else if(users[0].isUse == 0){
            QMessageBox::information(this, tr("提示"), tr("当前账号未启用！"),tr("确定"));
            ui->userName->clear();
            ui->userName->setFocus();
            return;
        }else{
            dataOper.saveLog("G11","","用户登录",u,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            this->hide();
            m = new MainWindow(u,p);
//            m->setAttribute(Qt::WA_DeleteOnClose);
            m->show();
        }
    }
}
//重置
void Login::on_LoginResetBtn_clicked()
{
    ui->userName->clear();
    ui->passWord->clear();
    ui->userName->setFocus();
}
