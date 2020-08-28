#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtSql>
#include <QDebug>


static bool db_sqllitecreateConnection()
{

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("ch.db");
    if (!database.open()){
        qDebug() << "Error: Failed to connect database." << database.lastError();
        return false;
    } else {
        qDebug() << "Succeed to connect QSQLITE database." ;
        return true;
    }
}

//创建数据库表
static bool createTable()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //建立数据库连接
    QSqlQuery query(db);
    bool success = query.exec("create table automobil(id int primary key,attribute varchar,"
                              "type varchar,kind varchar,nation int,carnumber int,elevaltor int,"
                              "distance int,oil int,temperature int)");
    if(success)
    {
        qDebug() << QObject::tr("数据库表创建成功！\n");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("数据库表创建失败！\n");
        return false;
    }
}

#endif // CONNECTION_H
