﻿#ifndef DATAOPER_H
#define DATAOPER_H

#include <QObject>
#include <vector>
#include "objectinfo.h"

using namespace std;

/**
 * @brief The DataOper class
 * 数据处理类   主要操作数据库。
 */
class DataOper : public QObject
{
    Q_OBJECT
public:
    explicit DataOper(QObject *parent = nullptr);

    void saveUser(User u);
    void updateUser(User u);
    bool userIsExist(User u);
    void getAllUsersInfo(vector<User> &users, QString userName = nullptr);
    //1-查询所有，2-根据起止日期查询
    void getAllWarningInfo(vector<Warning> &warnings, int falg, QString s_start = nullptr, QString e_time = nullptr);
    void getAllLogInfo(vector<Log> &logs, int falg,QString s_start = nullptr,QString e_time = nullptr);
    bool deleteUserByName(QString userName);
    void saveLog(Log log);
    void saveLog(QString address,QString device,QString operType,QString userName,QString date);
    void saveCompressorSet(CompressorSet compressorSet);
    void getLastCompressorSet(CompressorSet &compressorSet , int compressorNo);
    void saveReadCompressor1(QVector<quint16> compressor, QString storageTime);
    void saveReadCompressor2(QVector<quint16> compressor, QString storageTime);
    void saveReadCompressor3(QVector<quint16> compressor, QString storageTime);
    void saveReadDryer1(QVector<quint16> dryer, QString storageTime);
    void saveReadDryer2(QVector<quint16> dryer, QString storageTime);
    void saveReadDryer3(QVector<quint16> dryer, QString storageTime);
    void saveWarningInfo(Warning warning);
    void getCompressorInfo(vector<Compressor> &compressors, int CompressorNo, QString s_start, QString e_time);

    void deleteGrabDataInfo(QString tableName,QString time);
    void deleteLogDataInfo(QString time);
    void deleteWarningDataInfo(QString time);
    void GetTotalRecordCount(QString tableName, int &totalRecordCount);
    void RecordQuery(int limitIndex,int pageRecordCount,QString tableName, vector<Compressor> &compressors,vector<Dryer> &dryers);





signals:
    void sendDataMessage(const QString& info);

};

#endif // DATAOPER_H
