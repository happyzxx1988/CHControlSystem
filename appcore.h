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

    void sendReadData(const QVector<quint16> compressor1,
                      const QVector<quint16> compressor2,
                      const QVector<quint16> compressor3,
                      const QVector<quint16> dryer1,
                      const QVector<quint16> dryer2,
                      const QVector<quint16> dryer3);

public slots:

protected:
    QSettings*          settings;
    DeviceCommunication dc;

public:
    QSettings*          appSettings()       { return settings; }
    bool                deviceIsConnected();
    void                disconnectDevice();
    QSettings*          initSettings();
    void                initDevice();
    void connectPLC();

    void setMaxAndMinPressure(int max, int min);
    void setEquipmentSwitch(int equipmentType, bool off);
    void resetOperation();
    void setRunMode(DeviceRunMode mode);
    void setUninstallPressureAndPressureDiff(int uninstallPressure,int pressureDiff,int compressorNo);
    void readCompressor(QVector<quint16> &compressor1,
                        QVector<quint16> &compressor2,
                        QVector<quint16> &compressor3,
                        QVector<quint16> &dryer1,
                        QVector<quint16> &dryer2,
                        QVector<quint16> &dryer3);
    void readDryer(QVector<quint16> &dryer1, QVector<quint16> &dryer2, QVector<quint16> &dryer3);

    void readCompressor1(QVector<quint16> &compressor1);
    void readCompressor2(QVector<quint16> &compressor2);
    void readCompressor3(QVector<quint16> &compressor3);
    void dryer1(QVector<quint16> &dryer1);
    void dryer2(QVector<quint16> &dryer2);
    void dryer3(QVector<quint16> &dryer3);

    void readWarningHint(QVector<quint16> &warningInfo);

    void readEquipmentStatus(QVector<quint16> &equipmentStatus);

    void setEquipmentEnable(int equipmentType,bool off);

    void readUint16(int address_, int count_, QVector<quint16> &buffer_);


    void sleep(unsigned int msec);


};

#endif // APPCORE_H
