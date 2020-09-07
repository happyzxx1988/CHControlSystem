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

    void sendReadData(const QVector<quint16> compressor1,
                      const QVector<quint16> compressor2,
                      const QVector<quint16> compressor3,
                      const QVector<quint16> dryer1,
                      const QVector<quint16> dryer2,
                      const QVector<quint16> dryer3);

    void readALLOverData();

    void readDryerData();

    void readOverCompressorData1();
    void readOverCompressorData2();
    void readOverCompressorData3();
    void readOverDryer1Data1();
    void readOverDryer1Data2();
    void readOverDryer1Data3();

    void readUint16Signal_D();

private:
    QModbusClient *modbusDevice;
    int             srvId;

public:
    void setMaxAndMinPressure(int max, int min);
    void compressorSwitch1(bool off);
    void compressorSwitch2(bool off);
    void compressorSwitch3(bool off);
    void dryerSwitch1(bool off);
    void dryerSwitch2(bool off);
    void dryerSwitch3(bool off);
    void resetOperation();
    void setRunMode(DeviceRunMode mode);
    void setUninstallPressureAndPressureDiff1(int uninstallPressure,int pressureDiff);
    void setUninstallPressureAndPressureDiff2(int uninstallPressure,int pressureDiff);
    void setUninstallPressureAndPressureDiff3(int uninstallPressure,int pressureDiff);
    void readCompressor(QVector<quint16> &compressor1,
                        QVector<quint16> &compressor2,
                        QVector<quint16> &compressor3,
                        QVector<quint16> &dryer1,
                        QVector<quint16> &dryer2,
                        QVector<quint16> &dryer3);

    void readCompressor_old(QVector<quint16> &compressor1,
                        QVector<quint16> &compressor2,
                        QVector<quint16> &compressor3,
                        QVector<quint16> &dryer1,
                        QVector<quint16> &dryer2,
                        QVector<quint16> &dryer3);

    void readCompressor1(QVector<quint16> &compressor1);
    void readCompressor2(QVector<quint16> &compressor2);
    void readCompressor3(QVector<quint16> &compressor3);
    void dryer1(QVector<quint16> &dryer1);
    void dryer2(QVector<quint16> &dryer2);
    void dryer3(QVector<quint16> &dryer3);

    void readDryer(QVector<quint16> &dryer1, QVector<quint16> &dryer2, QVector<quint16> &dryer3);

    void readUint16(int address_, int count_, std::vector<quint16> &buffer_);

    void writeAddress26();
    void writeAddress410(int val);


};

#endif // DEVICECOMMUNICATION_H
