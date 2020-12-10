#include "appcore.h"
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTime>

#define SETTINGS_VERSION                    "1.8"
#define DEFAULT_SETTINGS_FILENAME           "Settings.ini"
#define DEFAULT_PLUGINS_PATH                "plugins"

AppCore::AppCore(QObject *parent) : QObject(parent)
{
    settings = nullptr;
}

QSettings *AppCore::initSettings()
{
    // Initialize settings
    settings = new QSettings(DEFAULT_SETTINGS_FILENAME, QSettings::IniFormat, this);
    if (settings->value("SettingsVersion").toString() == SETTINGS_VERSION)
        return settings;

    QFileInfo fi(DEFAULT_SETTINGS_FILENAME);
    if (fi.exists())
    {
        QDir dir;
        dir.remove(DEFAULT_SETTINGS_FILENAME);
    }
    settings->setValue("SettingsVersion", SETTINGS_VERSION);
    settings->setValue("DevicePLC/IP", "127.0.0.1");
    settings->setValue("DevicePLC/port", 502);
    settings->setValue("DevicePLC/Timeout", 5000);
    settings->setValue("DevicePLC/Retries", 3);
    settings->setValue("DevicePLC/ServerId", 1);

//    settings->setValue("Device/PortName", "COM4");
//    settings->setValue("Device/Parity", 0);
//    settings->setValue("Device/BaudRate", 9600);
//    settings->setValue("Device/DataBits", 8);
//    settings->setValue("Device/StopBits", 1);
//    settings->setValue("Device/Timeout", 5000);
//    settings->setValue("Device/Retries", 2);
//    settings->setValue("Device/ServerId", 2);

    return settings;
}
void AppCore::initDevice()
{
    if (!settings) return;

    emit infoMessage(tr("Initializing device..."));

    connect(&dc, &DeviceCommunication::deviceConnected, this, [this](){
        emit deviceConnected();
    });
    connect(&dc, &DeviceCommunication::deviceDisconnected, this, [this](){
        emit deviceDisconnected();
    });
    connect(&dc, &DeviceCommunication::modbusError, this, [this](const QString& error){
        emit errorMessage(error);
    });

    connect(&dc, &DeviceCommunication::sendReadData, this, [this](
            const QVector<quint16> compressor1,
            const QVector<quint16> compressor2,
            const QVector<quint16> compressor3,
            const QVector<quint16> dryer1,
            const QVector<quint16> dryer2,
            const QVector<quint16> dryer3){
        emit sendReadData(compressor1,compressor2,compressor3,dryer1,dryer2,dryer3);
    });

//    QString IP = settings->value("DevicePLC/IP").toString();
//    int port = settings->value("DevicePLC/port").toInt();
//    int Timeout = settings->value("DevicePLC/Timeout").toInt();
//    int Retries = settings->value("DevicePLC/Retries").toInt();
//    int ServerId = settings->value("DevicePLC/ServerId").toInt();
//    dc.connectPLC(QString("%1:%2").arg(IP).arg(port),Timeout,Retries,ServerId);


    emit infoMessage(tr("Device initialized."));
}

void AppCore::connectPLC()
{
    QString IP = settings->value("DevicePLC/IP").toString();
    int port = settings->value("DevicePLC/port").toInt();
    int Timeout = settings->value("DevicePLC/Timeout").toInt();
    int Retries = settings->value("DevicePLC/Retries").toInt();
    int ServerId = settings->value("DevicePLC/ServerId").toInt();
    dc.connectPLC(QString("%1:%2").arg(IP).arg(port),Timeout,Retries,ServerId);
}

bool AppCore::deviceIsConnected()
{
    return dc.isConnected();
}

void AppCore::disconnectDevice()
{
    dc.disconnectDevice();
}

void AppCore::setMaxAndMinPressure(int max,int min)
{
    dc.setMaxAndMinPressure(max,min);
}

void AppCore::setUninstallPressureAndPressureDiff(int uninstallPressure, int pressureDiff, int compressorNo)
{
    switch (compressorNo) {
    case 1:
        dc.setUninstallPressureAndPressureDiff1(uninstallPressure,pressureDiff);
        break;
    case 2:
        dc.setUninstallPressureAndPressureDiff2(uninstallPressure,pressureDiff);
        break;
    case 3:
        dc.setUninstallPressureAndPressureDiff3(uninstallPressure,pressureDiff);
        break;
    }
    dc.writeAddress26();
    this->sleep(200);
    dc.writeAddress410(compressorNo);

}

//设备开关设置 1-1#空压机，2-2#空压机，3-3#空压机，4-1#冷干机，5-2#冷干机，6-3#冷干机
void AppCore::setEquipmentSwitch(int equipmentType,bool off)
{
    dc.writeAddress26();
    this->sleep(200);

    switch (equipmentType) {
    case 1:
        dc.compressorSwitch1(off);
        break;
    case 2:
        dc.compressorSwitch2(off);
        break;
    case 3:
        dc.compressorSwitch3(off);
        break;
    case 4:
        dc.dryerSwitch1(off);
        break;
    case 5:
        dc.dryerSwitch2(off);
        break;
    case 6:
        dc.dryerSwitch3(off);
        break;
    }
}

//设备设置是否启用 1-1#空压机，2-2#空压机，3-3#空压机，4-1#冷干机，5-2#冷干机，6-3#冷干机
void AppCore::setEquipmentEnable(int equipmentType,bool off)
{
    this->sleep(500);
    switch (equipmentType) {
    case 1:
        dc.compressorEnable1(off);
        break;
    case 2:
        dc.compressorEnable2(off);
        break;
    case 3:
        dc.compressorEnable3(off);
        break;
    case 4:
        dc.dryerEnable1(off);
        break;
    case 5:
        dc.dryerEnable2(off);
        break;
    case 6:
        dc.dryerEnable3(off);
        break;
    }
}

void AppCore::readUint16(int address_, int count_, QVector<quint16> &buffer_)
{
    dc.readUint16(address_,count_,buffer_);
}
void AppCore::readFloat32(int address_,float &buffer_)
{
    dc.readFloat32(address_,buffer_);
}


void AppCore::resetOperation()
{
    dc.resetOperation();
}

void AppCore::setRunMode(DeviceRunMode mode)
{
    dc.setRunMode(mode);
}

//以毫秒为单位的延时函数
void AppCore::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}




void AppCore::readCompressor1(QVector<quint16> &compressor1)
{
   dc.readCompressor1(compressor1);
}
void AppCore::readCompressor2(QVector<quint16> &compressor2)
{
    dc.readCompressor2(compressor2);
}
void AppCore::readCompressor3(QVector<quint16> &compressor3)
{
    dc.readCompressor3(compressor3);
}
void AppCore::dryer1(QVector<quint16> &dryer1,QVector<quint16> &float2)
{
   dc.dryer1(dryer1,float2);
}
void AppCore::dryer2(QVector<quint16> &dryer2)
{
    dc.dryer2(dryer2);
}
void AppCore::dryer3(QVector<quint16> &dryer3)
{
    dc.dryer3(dryer3);
}

void AppCore::readWarningHint(QVector<quint16> &warningInfo)
{
    dc.readWarningHint(warningInfo);
}

void AppCore::readEquipmentStatus(QVector<quint16> &equipmentStatus)
{
    dc.readEquipmentStatus(equipmentStatus);
}

