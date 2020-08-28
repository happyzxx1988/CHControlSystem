#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "userdialog.h"
#include "ui_userdialog.h"
#include <QMessageBox>
#include <QCryptographicHash>
#include "objectinfo.h"
#include <QDateTime>
/**
 * @brief UserDialog::UserDialog  新增用户，修改用户窗口
 * @param currentUser
 * @param u
 * @param p
 * @param flag
 * @param parent
 */
UserDialog::UserDialog(QString currentUser, QString u, QString p, int flag, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserDialog),
    userName(u),
    pwd(p),
    flag_(flag),
    currentUser_(currentUser)
{
    ui->setupUi(this);
    // 去掉问号，只保留关闭
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    if(flag_ == 2){//修改用户
        ui->userName->setText(this->userName);
        ui->userName->setEnabled(false);//用户名不能修改
        this->setWindowTitle("修改用户");
    }else{
        this->setWindowTitle("新增用户");
    }
}

UserDialog::~UserDialog()
{
    delete ui;
}

//保存新用户   或者    修改旧用户
void UserDialog::on_saveUserBtn_clicked()
{
    QString u = ui->userName->text();
    QString p = ui->pwd->text();
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    bool isChecked = ui->isUse->isChecked();
    int isUse = isChecked ? 1 : 0;

//    QString strrr = QString::fromLocal8Bit(p);
    QByteArray bytePwd = p.toUtf8();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    QString strPwdMd5 = bytePwdMd5.toHex();

    if(u.isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("用户名不能为空"),tr("确定"));
        return;
    }
    if(p.isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("密码不能为空"),tr("确定"));
        return;
    }
    User object_u;
    object_u.name = u;
    object_u.password = strPwdMd5;
    object_u.date = currentDateTime;
    object_u.isUse = isUse;

    if(flag_ == 1){//保存
        if(dataOper.userIsExist(object_u)){
            QMessageBox::information(this, tr("提示"), tr("当前用户已经存在！"),tr("确定"));
            return;
        }
        dataOper.saveUser(object_u);
    }else{//修改
        dataOper.updateUser(object_u);
    }
    emit closeCurrentDialog(flag_);
    this->close();
}
