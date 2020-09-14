#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "login.h"
#include "ui_login.h"
#include "connection.h"
#include "mainwindow.h"
#include <QMessageBox>


#define SETTINGS_VERSION                    "1.8"
#define DEFAULT_SETTINGS_FILENAME           "mysql.ini"
#define DEFAULT_PLUGINS_PATH                "plugins"


Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login),
    settings(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

//    init_mysql();
//    if(!db_mysqlcreateConnection()){//mysql连接数据库
//        return;
//    }


    if(!db_sqllitecreateConnection()){//DB连接数据库
        QMessageBox::information(this, tr("提示"), tr("数据库连接失败"),tr("确定"));
        return;
    }

//    ui->logLabel->hide();
    ui->logLabel->setText("空压机恒压控制系统");
    this->setWindowTitle("空压机恒压控制系统");

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
            dataOper.saveLog(STATION_HOUSE,"","用户登录",u,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            this->hide();
            m = new MainWindow(3,u,p);
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



QSettings *Login::init_mysql()
{
    // Initialize settings
    settings = new QSettings(DEFAULT_SETTINGS_FILENAME, QSettings::IniFormat);
    if (settings->value("SettingsVersion").toString() == SETTINGS_VERSION){
        return settings;
    }

    QFileInfo fi(DEFAULT_SETTINGS_FILENAME);
    if (fi.exists())
    {
        return settings;
    }

    settings->setValue("SettingsVersion", SETTINGS_VERSION);

    settings->setValue("SQL/HostName", "127.0.0.1");
    settings->setValue("SQL/port", 3306);
    settings->setValue("SQL/DataBaseName", "ch");
    settings->setValue("SQL/UserName", "root");
    settings->setValue("SQL/PWD", "123456");

    return settings;
}

bool Login::db_mysqlcreateConnection()
{
    QSqlDatabase db_mysql = QSqlDatabase::addDatabase("QMYSQL");

    db_mysql.setHostName(settings->value("SQL/HostName").toString());
    db_mysql.setPort(settings->value("SQL/port").toInt());
    db_mysql.setDatabaseName(settings->value("SQL/DataBaseName").toString());
    db_mysql.setUserName(settings->value("SQL/UserName").toString());
    db_mysql.setPassword(settings->value("SQL/PWD").toString());

    if (!db_mysql.open()) {
        qDebug() << QStringLiteral("MYSQL数据库链接失败：") << db_mysql.lastError();
        return false;
    }
    return true;
}
