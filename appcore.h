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
    QTimer              statusQueryTimer;
    DeviceCommunication dc;

public:
    QSettings*          appSettings()       { return settings; }
    bool                deviceIsConnected();
    QSettings*          initSettings();
    void                initDevice();

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
    void sleep(unsigned int msec);

};

#endif // APPCORE_H
