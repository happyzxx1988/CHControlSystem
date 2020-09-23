#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "dataoper.h"
#include <QtSql>


DataOper::DataOper(QObject *parent) : QObject(parent)
{

}

//新增用户
void DataOper::saveUser(User u)
{
    QSqlQuery query;
    query.prepare("INSERT INTO USER(name,password,date,isUse) VALUES (?,?,?,?)");
    query.addBindValue(u.name);
    query.addBindValue(u.password);
    query.addBindValue(u.date);
    query.addBindValue(u.isUse);
    if (query.exec()){ //进行批处理，如果出错就输出错误
        emit sendDataMessage(tr("保存用户成功!"));
    }else{
         qDebug() << tr("保存用户失败: ") + query.lastError().text();
    }
}
//更新用户
void DataOper::updateUser(User u)
{
    QSqlQuery query;
    query.prepare("UPDATE USER SET name = ?,password = ? ,isUse = ? where name = ?");
    query.addBindValue(u.name);
    query.addBindValue(u.password);
    query.addBindValue(u.isUse);
    query.addBindValue(u.name);
    if (query.exec()){
        emit sendDataMessage(tr("修改用户成功!"));
    }else{
         qDebug() << tr("修改用户失败: ") + query.lastError().text();
    }
}
//判断当前新增用户是否存在
bool DataOper::userIsExist(User u)
{
    QSqlQuery query;
    query.prepare("select  name from USER where name = ?");
    query.addBindValue(u.name);
    if (query.exec()){
        if(query.next()){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}
//根据用户名删除用户
bool DataOper::deleteUserByName(QString userName)
{
    QSqlQuery query;
    QString sql = QString("delete from USER where name = '%1' ").arg(userName);
    if(query.exec(sql)){
//        emit sendDataMessage(tr("删除用户成功! "));
        return true;
    }else{
//        emit sendDataMessage(tr("删除用户失败: ")+query.lastError().text());
        return false;
    }
}
//获取所有用户信息
void DataOper::getAllUsersInfo(vector<User> &users, QString userName)
{
    User  u;
    QSqlQuery query;
    QString sql = nullptr;
    if(userName == nullptr){
        sql = "SELECT name,password,date,isUse FROM USER ";
    }else{
        sql = QString("SELECT name,password,date,isUse FROM USER where name = '%1'").arg(userName);
    }

    bool successFlag = query.exec(sql);

    if(successFlag){
        while(query.next()){
            u.name = query.value(0).toString();
            u.password = query.value(1).toString();
            u.date = query.value(2).toString();
            u.isUse = query.value(3).toInt();
            users.push_back(u);
        }
    }else{
        qDebug() << tr("获取用户信息失败: ") + query.lastError().text();
    }
}
//获取所有报警信息
void DataOper::getAllWarningInfo(vector<Warning> &warnings, int falg,QString s_start,QString e_time)
{
    Warning  w;
    QSqlQuery query;
    QString sql = nullptr;
    if(falg == 1){
        sql  = "SELECT address,deviceName,info,date FROM Warning  ORDER BY date DESC ";
    }else{
        sql  = "SELECT address,deviceName,info,date FROM Warning where 1 = 1 ";
        if (s_start.compare("1") != 0)//比较大小
        {
            sql += " AND date >= '" + s_start+" 00:00:00' AND date <= '" + e_time +" 23:59:59' ";
        }
        sql += " ORDER BY date DESC";
    }

    bool successFlag = query.exec(sql);
    if(successFlag){
        while(query.next()){
            w.address = query.value(0).toString();
            w.deviceName = query.value(1).toString();
            w.info = query.value(2).toString();
            w.date = query.value(3).toString();
            warnings.push_back(w);
        }
    }else{
        qDebug() << tr("获取报警信息失败: ") + query.lastError().text();
    }
}
//获取所有日志信息
void DataOper::getAllLogInfo(vector<Log> &logs, int falg,QString s_start,QString e_time)
{
    Log  l;
    QSqlQuery query;
    QString sql = nullptr;
    if(falg == 1){
        sql  = "SELECT address,device,operType,userName,date FROM operlog  ORDER BY date DESC ";
    }else{
        sql  = "SELECT address,device,operType,userName,date FROM operlog where 1 = 1 ";
        if (s_start.compare("1") != 0)//比较大小
        {
            sql += " AND date >= '" + s_start+" 00:00:00' AND date <= '" + e_time +" 23:59:59' ";
        }
        sql += " ORDER BY date DESC";
    }

    bool successFlag = query.exec(sql);
    if(successFlag){
        while(query.next()){
            l.address = query.value(0).toString();
            l.device = query.value(1).toString();
            l.operType = query.value(2).toString();
            l.userName = query.value(3).toString();
            l.date = query.value(4).toString();
            logs.push_back(l);
        }
    }else{
        qDebug() << tr("获取日志信息失败: ") + query.lastError().text();
    }
}
//保存日志数据
void DataOper::saveLog(Log log)
{
    QSqlQuery query;
    query.prepare("INSERT INTO operlog(address,device,operType,userName,date) VALUES (?,?,?,?,?)");
    query.addBindValue(log.address);
    query.addBindValue(log.device);
    query.addBindValue(log.operType);
    query.addBindValue(log.userName);
    query.addBindValue(log.date);
    if (!query.exec()){
        qDebug() << tr("保存日志失败: ") + query.lastError().text();
    }
}
//保存日志数据
void DataOper::saveLog(QString address,QString device,QString operType,QString userName,QString date)
{
    QSqlQuery query;
    query.prepare("INSERT INTO operlog(address,device,operType,userName,date) VALUES (?,?,?,?,?)");
    query.addBindValue(address);
    query.addBindValue(device);
    query.addBindValue(operType);
    query.addBindValue(userName);
    query.addBindValue(date);
    if (!query.exec()){ //进行批处理，如果出错就输出错误
        qDebug() << tr("保存日志失败: ") + query.lastError().text();
    }
}
//保存空压机压力参数设置
void DataOper::saveCompressorSet(CompressorSet compressorSet)
{
    QSqlQuery query;
    query.prepare("INSERT INTO CompressorSet(uninstallPressure,pressureDiff,date,flag) VALUES (?,?,?,?)");
    query.addBindValue(compressorSet.uninstallPressure);
    query.addBindValue(compressorSet.pressureDiff);
    query.addBindValue(compressorSet.date);
    query.addBindValue(compressorSet.flag);
    if (!query.exec()){ //进行批处理，如果出错就输出错误
        qDebug() << tr("保存空压机压力参数设置失败: ") + query.lastError().text();
    }
}
//获取最近的一次空压机设置参数
void DataOper::getLastCompressorSet(CompressorSet &compressorSet,int compressorNo)
{
    QSqlQuery query;
    query.prepare("select uninstallPressure,pressureDiff,date from  CompressorSet where flag = ? order by date DESC limit 1");
    query.addBindValue(compressorNo);
    if (query.exec()){
        if(query.next()){
            compressorSet.uninstallPressure = query.value(0).toFloat();
            compressorSet.pressureDiff = query.value(1).toFloat();
            compressorSet.date = query.value(2).toString();
        }
    }else{
        qDebug() << tr("获取最近的一次空压机设置参数失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadCompressor1(QVector<quint16> compressor, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Compressor1(runTimeL,runTimeH,loadTimeL,loadTimeH,electricityType,airDemand,jointControlMode,"
                  "voltageDeviation,hostCurrent,dewPointTemperature,EnvironmentalTemperature,T1,T2,P1,P2,T3,T4,P3,P4,T5,T6,"
                  "runMode1,runMode2,runMode3,dp1,pressureDiff,uninstallPressure,MaxManifoldPressure,MinManifoldPressure,"
                  "MinimalPressure,StartLoadDelayTime,StopTime,OrderTime,RotateTime,TransitionTime,date) VALUES (?,?,?,?,?,?,?,?,?,?,"
                  "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(compressor.at(0));
    query.addBindValue(compressor.at(1));
    query.addBindValue(compressor.at(2));
    query.addBindValue(compressor.at(3));
    query.addBindValue(compressor.at(4));
    query.addBindValue(compressor.at(5));
    query.addBindValue(compressor.at(6));
    query.addBindValue(compressor.at(7));
    query.addBindValue(compressor.at(8));
    query.addBindValue(compressor.at(9));
    query.addBindValue(compressor.at(10));
    query.addBindValue(compressor.at(11));
    query.addBindValue(compressor.at(12));
    query.addBindValue(compressor.at(13));
    query.addBindValue(compressor.at(14));
    query.addBindValue(compressor.at(15));
    query.addBindValue(compressor.at(16));
    query.addBindValue(compressor.at(17));
    query.addBindValue(compressor.at(18));
    query.addBindValue(compressor.at(19));
    query.addBindValue(compressor.at(20));
    query.addBindValue(compressor.at(21));
    query.addBindValue(compressor.at(22));
    query.addBindValue(compressor.at(23));
    query.addBindValue(compressor.at(24));
    query.addBindValue(compressor.at(25));
    query.addBindValue(compressor.at(26));
    query.addBindValue(compressor.at(27));
    query.addBindValue(compressor.at(28));
    query.addBindValue(compressor.at(29));
    query.addBindValue(compressor.at(30));
    query.addBindValue(compressor.at(31));
    query.addBindValue(compressor.at(32));
    query.addBindValue(compressor.at(33));
    query.addBindValue(compressor.at(34));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的空压机数据失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadCompressor2(QVector<quint16> compressor, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Compressor2(runTimeL,runTimeH,loadTimeL,loadTimeH,electricityType,airDemand,jointControlMode,"
                  "voltageDeviation,hostCurrent,dewPointTemperature,EnvironmentalTemperature,T1,T2,P1,P2,T3,T4,P3,P4,T5,T6,"
                  "runMode1,runMode2,runMode3,dp1,pressureDiff,uninstallPressure,MaxManifoldPressure,MinManifoldPressure,"
                  "MinimalPressure,StartLoadDelayTime,StopTime,OrderTime,RotateTime,TransitionTime,date) VALUES (?,?,?,?,?,?,?,?,?,?,"
                  "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(compressor.at(0));
    query.addBindValue(compressor.at(1));
    query.addBindValue(compressor.at(2));
    query.addBindValue(compressor.at(3));
    query.addBindValue(compressor.at(4));
    query.addBindValue(compressor.at(5));
    query.addBindValue(compressor.at(6));
    query.addBindValue(compressor.at(7));
    query.addBindValue(compressor.at(8));
    query.addBindValue(compressor.at(9));
    query.addBindValue(compressor.at(10));
    query.addBindValue(compressor.at(11));
    query.addBindValue(compressor.at(12));
    query.addBindValue(compressor.at(13));
    query.addBindValue(compressor.at(14));
    query.addBindValue(compressor.at(15));
    query.addBindValue(compressor.at(16));
    query.addBindValue(compressor.at(17));
    query.addBindValue(compressor.at(18));
    query.addBindValue(compressor.at(19));
    query.addBindValue(compressor.at(20));
    query.addBindValue(compressor.at(21));
    query.addBindValue(compressor.at(22));
    query.addBindValue(compressor.at(23));
    query.addBindValue(compressor.at(24));
    query.addBindValue(compressor.at(25));
    query.addBindValue(compressor.at(26));
    query.addBindValue(compressor.at(27));
    query.addBindValue(compressor.at(28));
    query.addBindValue(compressor.at(29));
    query.addBindValue(compressor.at(30));
    query.addBindValue(compressor.at(31));
    query.addBindValue(compressor.at(32));
    query.addBindValue(compressor.at(33));
    query.addBindValue(compressor.at(34));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的空压机数据失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadCompressor3(QVector<quint16> compressor, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Compressor3(runTimeL,runTimeH,loadTimeL,loadTimeH,electricityType,airDemand,jointControlMode,"
                  "voltageDeviation,hostCurrent,dewPointTemperature,EnvironmentalTemperature,T1,T2,P1,P2,T3,T4,P3,P4,T5,T6,"
                  "runMode1,runMode2,runMode3,dp1,pressureDiff,uninstallPressure,MaxManifoldPressure,MinManifoldPressure,"
                  "MinimalPressure,StartLoadDelayTime,StopTime,OrderTime,RotateTime,TransitionTime,date) VALUES (?,?,?,?,?,?,?,?,?,?,"
                  "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(compressor.at(0));
    query.addBindValue(compressor.at(1));
    query.addBindValue(compressor.at(2));
    query.addBindValue(compressor.at(3));
    query.addBindValue(compressor.at(4));
    query.addBindValue(compressor.at(5));
    query.addBindValue(compressor.at(6));
    query.addBindValue(compressor.at(7));
    query.addBindValue(compressor.at(8));
    query.addBindValue(compressor.at(9));
    query.addBindValue(compressor.at(10));
    query.addBindValue(compressor.at(11));
    query.addBindValue(compressor.at(12));
    query.addBindValue(compressor.at(13));
    query.addBindValue(compressor.at(14));
    query.addBindValue(compressor.at(15));
    query.addBindValue(compressor.at(16));
    query.addBindValue(compressor.at(17));
    query.addBindValue(compressor.at(18));
    query.addBindValue(compressor.at(19));
    query.addBindValue(compressor.at(20));
    query.addBindValue(compressor.at(21));
    query.addBindValue(compressor.at(22));
    query.addBindValue(compressor.at(23));
    query.addBindValue(compressor.at(24));
    query.addBindValue(compressor.at(25));
    query.addBindValue(compressor.at(26));
    query.addBindValue(compressor.at(27));
    query.addBindValue(compressor.at(28));
    query.addBindValue(compressor.at(29));
    query.addBindValue(compressor.at(30));
    query.addBindValue(compressor.at(31));
    query.addBindValue(compressor.at(32));
    query.addBindValue(compressor.at(33));
    query.addBindValue(compressor.at(34));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的空压机数据失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadDryer1(QVector<quint16> dryer, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO dryer1(runHint,faultHint,compressor,drainer,phaseOrderFault,overloadSave,sysHighVoltage,"
                  "sysLowVoltage,dewPointProbeFault,dewPointH,dewPointL,faultWarn,faultStop,countDown,dewPointT,runTimeH,runTimeM,date)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(dryer.at(0));
    query.addBindValue(dryer.at(1));
    query.addBindValue(dryer.at(2));
    query.addBindValue(dryer.at(3));
    query.addBindValue(dryer.at(4));
    query.addBindValue(dryer.at(5));
    query.addBindValue(dryer.at(6));
    query.addBindValue(dryer.at(7));
    query.addBindValue(dryer.at(8));
    query.addBindValue(dryer.at(9));
    query.addBindValue(dryer.at(10));
    query.addBindValue(dryer.at(11));
    query.addBindValue(dryer.at(12));
    query.addBindValue(dryer.at(13));
    query.addBindValue(dryer.at(14));
    query.addBindValue(dryer.at(15));
    query.addBindValue(dryer.at(16));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的1#冷干机数据失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadDryer2(QVector<quint16> dryer, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO dryer2(runHint,faultHint,compressor,drainer,phaseOrderFault,overloadSave,sysHighVoltage,"
                  "sysLowVoltage,dewPointProbeFault,dewPointH,dewPointL,faultWarn,faultStop,countDown,dewPointT,runTimeH,runTimeM,date)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(dryer.at(0));
    query.addBindValue(dryer.at(1));
    query.addBindValue(dryer.at(2));
    query.addBindValue(dryer.at(3));
    query.addBindValue(dryer.at(4));
    query.addBindValue(dryer.at(5));
    query.addBindValue(dryer.at(6));
    query.addBindValue(dryer.at(7));
    query.addBindValue(dryer.at(8));
    query.addBindValue(dryer.at(9));
    query.addBindValue(dryer.at(10));
    query.addBindValue(dryer.at(11));
    query.addBindValue(dryer.at(12));
    query.addBindValue(dryer.at(13));
    query.addBindValue(dryer.at(14));
    query.addBindValue(dryer.at(15));
    query.addBindValue(dryer.at(16));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的2#冷干机数据失败: ") + query.lastError().text();
    }
}
//存储读取的空压机数据
void DataOper::saveReadDryer3(QVector<quint16> dryer, QString storageTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO dryer3(runHint,faultHint,compressor,drainer,phaseOrderFault,overloadSave,sysHighVoltage,"
                  "sysLowVoltage,dewPointProbeFault,dewPointH,dewPointL,faultWarn,faultStop,countDown,dewPointT,runTimeH,runTimeM,date)"
                  " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(dryer.at(0));
    query.addBindValue(dryer.at(1));
    query.addBindValue(dryer.at(2));
    query.addBindValue(dryer.at(3));
    query.addBindValue(dryer.at(4));
    query.addBindValue(dryer.at(5));
    query.addBindValue(dryer.at(6));
    query.addBindValue(dryer.at(7));
    query.addBindValue(dryer.at(8));
    query.addBindValue(dryer.at(9));
    query.addBindValue(dryer.at(10));
    query.addBindValue(dryer.at(11));
    query.addBindValue(dryer.at(12));
    query.addBindValue(dryer.at(13));
    query.addBindValue(dryer.at(14));
    query.addBindValue(dryer.at(15));
    query.addBindValue(dryer.at(16));
    query.addBindValue(storageTime);
    if (!query.exec()){
        qDebug() << tr("保存存储读取的3#冷干机数据失败: ") + query.lastError().text();
    }
}
//保存报警信息
void DataOper::saveWarningInfo(Warning warning)
{
    QSqlQuery query;
    query.prepare("INSERT INTO warning(address,DeviceName,Info,Date) VALUES (?,?,?,?)");
    query.addBindValue(warning.address);
    query.addBindValue(warning.deviceName);
    query.addBindValue(warning.info);
    query.addBindValue(warning.date);
    if (!query.exec()){
        qDebug() << tr("保存空压机报警信息失败: ") + query.lastError().text();
    }
}
//根据起止时间查询加载压差和卸载压力
void DataOper::getCompressorInfo(vector<Compressor> &compressors, int CompressorNo, QString s_start, QString e_time)
{
    Compressor  compressor;
    QSqlQuery query;
    QString sql = nullptr;
    if(CompressorNo == 1){
        sql  = "SELECT pressureDiff,uninstallPressure,date FROM Compressor1 where 1 = 1 ";
        if (s_start.compare("1") != 0)//比较大小
        {
            sql += " AND date >= '" + s_start+" 00:00:00' AND date <= '" + e_time +" 23:59:59' ";
        }
        sql += " ORDER BY date DESC";
    }else if(CompressorNo == 2){
        sql  = "SELECT pressureDiff,uninstallPressure,date FROM Compressor2 where 1 = 1 ";
        if (s_start.compare("1") != 0)//比较大小
        {
            sql += " AND date >= '" + s_start+" 00:00:00' AND date <= '" + e_time +" 23:59:59' ";
        }
        sql += " ORDER BY date DESC";
    }else if(CompressorNo == 3){
        sql  = "SELECT pressureDiff,uninstallPressure,date FROM Compressor3 where 1 = 1 ";
        if (s_start.compare("1") != 0)//比较大小
        {
            sql += " AND date >= '" + s_start+" 00:00:00' AND date <= '" + e_time +" 23:59:59' ";
        }
        sql += " ORDER BY date DESC";
    }
    bool successFlag = query.exec(sql);
    if(successFlag){
        while(query.next()){
            compressor.pressureDiff = QString::number(query.value(0).toDouble()/142.0,'f',2).toDouble();
            compressor.uninstallPressure = QString::number(query.value(1).toDouble()/142.0,'f',2).toDouble();
            compressor.date = query.value(2).toDateTime();
            compressors.push_back(compressor);
        }
    }else{
        qDebug() << tr("加载数据失败: ") + query.lastError().text();
    }
}



//删除给定时间之前的采集的数据，
void DataOper::deleteGrabDataInfo(QString tableName,QString time)
{
    QSqlQuery query;
    QString sql = nullptr;
    sql += QString("delete from %1 where date <= '%2 23:59:59'").arg(tableName).arg(time);
    qDebug() << sql;
    if(query.exec(sql)){
//        qDebug() << QString("delete %1 success").arg(tableName);
    }else{
//        qDebug() << QString("delete %1 fail: %2").arg(tableName).arg(query.lastError().text());
    }
}
//删除给定时间之前的日志数据，
void DataOper::deleteLogDataInfo(QString time)
{
    QSqlQuery query;
    QString sql = nullptr;
    sql += QString("delete from operlog where date <= '%1 23:59:59'").arg(time);
    qDebug() << sql;
    if(query.exec(sql)){
//        qDebug() << "delete log success";
    }else{
//        qDebug() << "delete log fail: " + query.lastError().text();
    }
}
//删除给定时间之前的警告数据
void DataOper::deleteWarningDataInfo(QString time)
{
    QSqlQuery query;
    QString sql = nullptr;
    sql += QString("delete from warning where date <= '%1 23:59:59'").arg(time);
    qDebug() << sql;
    if(query.exec(sql)){
//        qDebug() << "delete warning success";
    }else{
//        qDebug() << "delete warning fail: " + query.lastError().text();
    }
}
