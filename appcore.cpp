#include "appcore.h"
#include <QDir>
#include <QFileInfo>

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

    QString IP = settings->value("DevicePLC/IP").toString();
    int port = settings->value("DevicePLC/port").toInt();
    int Timeout = settings->value("DevicePLC/Timeout").toInt();
    int Retries = settings->value("DevicePLC/Retries").toInt();
    int ServerId = settings->value("DevicePLC/ServerId").toInt();

    dc.connectPLC(QString("%1:%2").arg(IP).arg(port),Timeout,Retries,ServerId);

    connect(&statusQueryTimer, &QTimer::timeout, this, [this](){
//        dc.exceptionQuery();
//        dc.actionStatusQuery();
    });
    statusQueryTimer.start(1000);

    emit infoMessage(tr("Device initialized."));
}
bool AppCore::deviceIsConnected()
{
    return dc.isConnected();
}


void AppCore::setMaxAndMinPressure(float max,float min)
{
    dc.setMaxAndMinPressure(max,min);
}

void AppCore::setUninstallPressureAndPressureDiff(float uninstallPressure,float pressureDiff,int compressorNo)
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
}
//批量一次性读取3台空压机数据
void AppCore::readCompressor(QVector<quint16> &data1, QVector<quint16> &data2, QVector<quint16> &data3)
{
    dc.readCompressor(data1,data2,data3);
}
//批量一次性读取3台冷干机数据
void AppCore::readDryer(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3)
{
    dc.readDryer(data1,data2,data3);
}


//设备开关设置 1-1#空压机，2-2#空压机，3-3#空压机，4-1#冷干机，5-2#冷干机，6-3#冷干机
void AppCore::setEquipmentSwitch(int equipmentType,bool off)
{
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

void AppCore::resetOperation()
{
    dc.resetOperation();
}

void AppCore::setRunMode(DeviceRunMode mode)
{
    dc.setRunMode(mode);
}

