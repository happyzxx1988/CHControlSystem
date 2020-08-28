#ifndef DEVICECOMMUNICATION_H
#define DEVICECOMMUNICATION_H

#include <QObject>
#include <QModbusDataUnit>
#include <QSerialPort>
#include <functional>

QT_BEGIN_NAMESPACE

class QModbusClient;
class QModbusReply;

QT_END_NAMESPACE


enum DeviceRunMode
{
    AutoMode = 0,
    ManualMode
};

class DeviceCommunication : public QObject
{
    Q_OBJECT
public:
    explicit DeviceCommunication(QObject *parent = nullptr);

    void connectPLC(const QString &port, int parity, int baud, int dataBits, int stopBits, int timeout, int retries, int serverId);
    void connectPLC(const QString &ip, int timeout, int retries, int serverId);
    void disconnectDevice();
    bool isConnected();

protected:
    void readRequest(QModbusDataUnit unit, std::function<void(QModbusDataUnit)> func);
    void writeRequest(QModbusDataUnit unit, std::function<void(void)> func);

    void writePulse(int address, std::function<void ()> func);
    void writeFloat32(int address, float f, std::function<void ()> func = nullptr);
    void writeUint16(int address, quint16 u, std::function<void ()> func = nullptr);

private slots:
    void onStateChanged(int state);


signals:
    void deviceDisconnected();
    void deviceConnected();
    void modbusError(const QString& error);

private:
    QModbusClient *modbusDevice;
    int             srvId;

public:
    void setMaxAndMinPressure(float max,float min);
    void compressorSwitch1(bool off);
    void compressorSwitch2(bool off);
    void compressorSwitch3(bool off);
    void dryerSwitch1(bool off);
    void dryerSwitch2(bool off);
    void dryerSwitch3(bool off);
    void resetOperation();
    void setRunMode(DeviceRunMode mode);
    void setUninstallPressureAndPressureDiff1(float uninstallPressure,float pressureDiff);
    void setUninstallPressureAndPressureDiff2(float uninstallPressure,float pressureDiff);
    void setUninstallPressureAndPressureDiff3(float uninstallPressure,float pressureDiff);
    void readCompressor(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3);
    void readDryer(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3);

};

#endif // DEVICECOMMUNICATION_H
