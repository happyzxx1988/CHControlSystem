#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include "devicecommunication.h"

class AppCore : public QObject
{
    Q_OBJECT
public:
    explicit AppCore(QObject *parent = nullptr);

signals:
    void deviceDisconnected();
    void deviceConnected();
    void errorMessage(const QString& error);
    void infoMessage(const QString& info);

public slots:

protected:
    QSettings*          settings;
    QTimer              statusQueryTimer;
    DeviceCommunication dc;

public:
    QSettings*          appSettings()       { return settings; }
    bool                deviceIsConnected();
    QSettings*          initSettings();
    void                initDevice();

    void setMaxAndMinPressure(float max,float min);
    void setEquipmentSwitch(int equipmentType, bool off);
    void resetOperation();
    void setRunMode(DeviceRunMode mode);
    void setUninstallPressureAndPressureDiff(float uninstallPressure,float pressureDiff,int compressorNo);
    void readCompressor(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3);
    void readDryer(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3);

};

#endif // APPCORE_H
