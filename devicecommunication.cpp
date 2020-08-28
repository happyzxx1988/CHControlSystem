#include "devicecommunication.h"
#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>
#include <QUrl>
#include <QVariant>
#include <QDebug>


const static QModbusDataUnit::RegisterType registerType1 = QModbusDataUnit::Coils;
const static QModbusDataUnit::RegisterType registerType2 = QModbusDataUnit::DiscreteInputs;
const static QModbusDataUnit::RegisterType registerType3 = QModbusDataUnit::InputRegisters;
const static QModbusDataUnit::RegisterType registerType4 = QModbusDataUnit::HoldingRegisters;

DeviceCommunication::DeviceCommunication(QObject *parent) : QObject(parent)
{
    srvId = 0;
    modbusDevice = nullptr;
}

void DeviceCommunication::connectPLC(const QString& port, int parity, int baud,int dataBits, int stopBits,
                                     int timeout, int retries, int serverId)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }
    srvId = serverId;
    modbusDevice = new QModbusRtuSerialMaster(this);
    connect(modbusDevice, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error) {
        emit modbusError(modbusDevice->errorString());
    });
    if (!modbusDevice){
        emit deviceDisconnected();
        emit modbusError(tr("Could not create connection."));
        return;
    }else{
        connect(modbusDevice, &QModbusClient::stateChanged,  this, &DeviceCommunication::onStateChanged);
    }

    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,port);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,parity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,baud);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,dataBits);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,stopBits);
    modbusDevice->setTimeout(timeout);
    modbusDevice->setNumberOfRetries(retries);
    if (!modbusDevice->connectDevice()) {
        emit modbusError(tr("Connect failed: ") + modbusDevice->errorString());
    } else {
        emit modbusError(tr("Connect success！"));
    }

}
void DeviceCommunication::connectPLC(const QString &ip, int timeout, int retries, int serverId)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }
    srvId = serverId;
    modbusDevice = new QModbusTcpClient(this);
    connect(modbusDevice, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error) {
        emit modbusError(tr("refused:")+modbusDevice->errorString());
    });
    if (!modbusDevice){
        emit deviceDisconnected();
        emit modbusError(tr("Could not create connection."));
        return;
    }else{
        connect(modbusDevice, &QModbusClient::stateChanged,  this, &DeviceCommunication::onStateChanged);
    }
    const QUrl url = QUrl::fromUserInput(ip);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
    modbusDevice->setTimeout(timeout);
    modbusDevice->setNumberOfRetries(retries);
    if (!modbusDevice->connectDevice()) {
        emit modbusError(tr("Connect failed: ") + modbusDevice->errorString());
    } else {
//        emit modbusError(tr("Connect success!"));
    }

}


void DeviceCommunication::onStateChanged(int state)
{
    //用于重新连接的判断
    if (state == QModbusDevice::UnconnectedState)
    {
        emit deviceDisconnected();
        emit modbusError(tr("Device disonnected!"));
    }
    else if (state == QModbusDevice::ConnectedState)
    {
        emit deviceConnected();
        emit modbusError(tr("Device connected."));
    }
}

void DeviceCommunication::disconnectDevice()
{
    if (modbusDevice){
        modbusDevice->disconnectDevice();
    }
    delete modbusDevice;
    modbusDevice = nullptr;
}
bool DeviceCommunication::isConnected()
{
    return modbusDevice ? (modbusDevice->state() == QModbusDevice::ConnectedState) : false;
}
void DeviceCommunication::readRequest(QModbusDataUnit unit, std::function<void(QModbusDataUnit)> func)
{
    if (!modbusDevice){
        return;
    }
    if (auto *reply = modbusDevice->sendReadRequest(unit, srvId)){
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, [this, reply, func](){
                if (reply->error() == QModbusDevice::NoError){
                    if (func) func(reply->result());
               }else{
                    emit modbusError(tr("Device response error: %1 (code: 0x%2)").arg(reply->errorString()).arg(reply->error(), -1, 16));
                }
                reply->deleteLater();
            });
        }else{
            delete reply; // broadcast replies return immediately
        }
    }else{
        emit modbusError(tr("Device response error: ") + modbusDevice->errorString());
    }
}

void DeviceCommunication::writeRequest(QModbusDataUnit unit, std::function<void ()> func)
{
    if (!modbusDevice){
        return;
    }
    QModbusReply *reply = modbusDevice->sendWriteRequest(unit, srvId);
    if (!reply){
        emit modbusError(tr("Device response error:") + modbusDevice->errorString());
        return;
    }

    if (reply->isFinished()){
        reply->deleteLater();
    }else{
        connect(reply, &QModbusReply::finished, this, [this, reply, func](){
            if (reply->error() != QModbusDevice::NoError){
                emit modbusError(tr("Device response error: %1 (code: 0x%2)").arg(reply->errorString()).arg(reply->error(), -1, 16));
            }else{
                if (func){
                    func();
                }
            }
            reply->deleteLater();
        });
    }
}
void DeviceCommunication::writePulse(int address, std::function<void ()> func)
{
    QModbusDataUnit writeUnit1(registerType4, address, 1);
    writeUnit1.setValue(0, 0x100);
    writeRequest(writeUnit1, nullptr);

    QModbusDataUnit writeUnit2(registerType4, address, 1);
    writeUnit2.setValue(0, 0);
    writeRequest(writeUnit2, func);
}

void DeviceCommunication::writeUint16(int address, quint16 u, std::function<void ()> func)
{
    QModbusDataUnit writeUnit(registerType4, address, 1);
    writeUnit.setValue(0, u);
    writeRequest(writeUnit, func);
}

void DeviceCommunication::writeFloat32(int address, float f, std::function<void ()> func)
{
    QModbusDataUnit writeUnit(registerType4, address, 2);

    float a = f;
    quint16* b = (quint16*)&a;
    quint16 c1 = b[1];
    quint16 c2 = b[0];

    writeUnit.setValue(0, c1);
    writeUnit.setValue(1, c2);

    writeRequest(writeUnit, func);
}


void DeviceCommunication::setMaxAndMinPressure(float max,float min)
{
    writeFloat32(13, max, [max](){
        qDebug() << "Device: Manual Move Speed Changed:" << max;
    });

    writeFloat32(12, min, [min](){
        qDebug() << "Device: Manual Move Speed Changed:" << min;
    });
}

void DeviceCommunication::setUninstallPressureAndPressureDiff1(float uninstallPressure,float pressureDiff)
{
    writeFloat32(1551, uninstallPressure, [uninstallPressure](){
        qDebug() << "Device: Manual Move Speed Changed:" << uninstallPressure;
    });

    writeFloat32(1552, pressureDiff, [pressureDiff](){
        qDebug() << "Device: Manual Move Speed Changed:" << pressureDiff;
    });
}
void DeviceCommunication::setUninstallPressureAndPressureDiff2(float uninstallPressure,float pressureDiff)
{
    writeFloat32(1561, uninstallPressure, [uninstallPressure](){
        qDebug() << "Device: Manual Move Speed Changed:" << uninstallPressure;
    });

    writeFloat32(1562, pressureDiff, [pressureDiff](){
        qDebug() << "Device: Manual Move Speed Changed:" << pressureDiff;
    });
}
void DeviceCommunication::setUninstallPressureAndPressureDiff3(float uninstallPressure,float pressureDiff)
{
    writeFloat32(1571, uninstallPressure, [uninstallPressure](){
        qDebug() << "Device: Manual Move Speed Changed:" << uninstallPressure;
    });

    writeFloat32(1572, pressureDiff, [pressureDiff](){
        qDebug() << "Device: Manual Move Speed Changed:" << pressureDiff;
    });
}
void DeviceCommunication::compressorSwitch1(bool off)
{
    writeUint16(1550, (off ? 1 : 2));
}
void DeviceCommunication::compressorSwitch2(bool off)
{
    writeUint16(1560, (off ? 1 : 2));
}
void DeviceCommunication::compressorSwitch3(bool off)
{
    writeUint16(1570, (off ? 1 : 2));
}
void DeviceCommunication::dryerSwitch1(bool off)
{
    writeUint16(224, (off ? 1 : 2));
}
void DeviceCommunication::dryerSwitch2(bool off)
{
    writeUint16(224, (off ? 0x100 : 0));
}
void DeviceCommunication::dryerSwitch3(bool off)
{
    writeUint16(224, (off ? 0x100 : 0));
}

void DeviceCommunication::resetOperation()
{
    writeUint16(5, 1);
}

void DeviceCommunication::setRunMode(DeviceRunMode mode)
{
    quint16 code = 0;

    switch (mode) {
    case AutoMode:
        code = 2;
        break;
    case ManualMode:
        code = 1;
        break;
    default:
        return;
        break;
    }
    writeUint16(1, code);
}

void DeviceCommunication::readCompressor(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3)
{
    QModbusDataUnit readUnit(registerType4, 100, 105);/*类型、首地址、长度*/// 100   105
    readRequest(readUnit, [this,&data1,&data2,&data3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            QString adress_str = tr("%1").arg(unit.startAddress() + i);
            QString value_str = tr("%1").arg(QString::number(unit.value(i)));
//            qDebug() << "adress_str:" << adress_str << "value_str:" << value_str;

            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;

            qDebug() << "adress_str:" << currentAddres << "value_str:" << value;

            if(currentAddres <=134 && currentAddres >=100){
                data1.append(value);
            }
            if(currentAddres <=184 && currentAddres >=150){
                data2.append(value);
            }
            if(currentAddres <=234 && currentAddres >=200){
                data3.append(value);
            }
        }
    });
}
void DeviceCommunication::readDryer(QVector<quint16> &data1,QVector<quint16> &data2,QVector<quint16> &data3)
{
    QModbusDataUnit readUnit(registerType4, 100, 105);/*类型、首地址、长度*///13
    readRequest(readUnit, [this,&data1,&data2,&data3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            QString adress_str = tr("%1").arg(unit.startAddress() + i);
            QString value_str = tr("%1").arg(QString::number(unit.value(i)));
            qDebug() << "adress_str:" << adress_str << "value_str:" << value_str;

            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;

            if(currentAddres <=134 && currentAddres >=100){
                data1.append(value);
            }
            if(currentAddres <=184 && currentAddres >=150){
                data2.append(value);
            }
            if(currentAddres <=234 && currentAddres >=200){
                data3.append(value);
            }
        }
    });
}
