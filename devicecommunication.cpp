﻿#include "devicecommunication.h"
#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>
#include <QUrl>
#include <QVariant>
#include <QDebug>
#include <QEventLoop>
#include <QTime>
#include <QCoreApplication>



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


void DeviceCommunication::setMaxAndMinPressure(int max,int min)
{
//    writeUint16(23, max);
//    writeUint16(24, min);


    writeUint16(23, max, [this](){ emit setMaxPressureFinished(); });

    QEventLoop loop;
    connect(this, &DeviceCommunication::setMaxPressureFinished,&loop, &QEventLoop::quit);
    loop.exec();

    writeUint16(24, min, [this](){ emit setMinPressureFinished(); });

    connect(this, &DeviceCommunication::setMinPressureFinished,&loop, &QEventLoop::quit);
    loop.exec();

}



void DeviceCommunication::setUninstallPressureAndPressureDiff1(int uninstallPressure,int pressureDiff)
{
//    writeUint16(400, uninstallPressure);
//    writeUint16(401, pressureDiff);

    writeUint16(400, uninstallPressure, [this](){ emit setUninstallPressure1Finished(); });

    QEventLoop loop;
    connect(this, &DeviceCommunication::setUninstallPressure1Finished,&loop, &QEventLoop::quit);
    loop.exec();

    writeUint16(401, pressureDiff, [this](){ emit setPressureDiff1Finished(); });

    connect(this, &DeviceCommunication::setPressureDiff1Finished,&loop, &QEventLoop::quit);
    loop.exec();

}
void DeviceCommunication::setUninstallPressureAndPressureDiff2(int uninstallPressure,int pressureDiff)
{
//    writeUint16(402, uninstallPressure);
//    writeUint16(403, pressureDiff);

    writeUint16(402, uninstallPressure, [this](){ emit setUninstallPressure2Finished(); });

    QEventLoop loop;
    connect(this, &DeviceCommunication::setUninstallPressure2Finished,&loop, &QEventLoop::quit);
    loop.exec();

    writeUint16(403, pressureDiff, [this](){ emit setPressureDiff2Finished(); });
    connect(this, &DeviceCommunication::setPressureDiff2Finished,&loop, &QEventLoop::quit);
    loop.exec();
}
void DeviceCommunication::setUninstallPressureAndPressureDiff3(int uninstallPressure,int pressureDiff)
{
//    writeUint16(404, uninstallPressure);
//    writeUint16(405, pressureDiff);

    writeUint16(404, uninstallPressure, [this](){ emit setUninstallPressure3Finished(); });

    QEventLoop loop;
    connect(this, &DeviceCommunication::setUninstallPressure3Finished,&loop, &QEventLoop::quit);
    loop.exec();

    writeUint16(405, pressureDiff, [this](){ emit setPressureDiff3Finished(); });
    connect(this, &DeviceCommunication::setPressureDiff3Finished,&loop, &QEventLoop::quit);
    loop.exec();

}
void DeviceCommunication::compressorSwitch1(bool off)
{
    writeUint16(50, (off ? 1 : 2));
}
void DeviceCommunication::compressorSwitch2(bool off)
{
    writeUint16(51, (off ? 1 : 2));
}
void DeviceCommunication::compressorSwitch3(bool off)
{
    writeUint16(52, (off ? 1 : 2));
}
void DeviceCommunication::dryerSwitch1(bool off)
{
    writeUint16(53, (off ? 1 : 0));
}
void DeviceCommunication::dryerSwitch2(bool off)
{
    writeUint16(54, (off ? 1 : 0));
}
void DeviceCommunication::dryerSwitch3(bool off)
{
    writeUint16(55, (off ? 1 : 0));
}

void DeviceCommunication::writeAddress26()
{
    writeUint16(26, 1);
}

void DeviceCommunication::writeAddress410(int val)
{
    writeUint16(410, val);
}

void DeviceCommunication::resetOperation()
{
    writeUint16(37, 1);
}

void DeviceCommunication::setRunMode(DeviceRunMode mode)
{
    quint16 code = 0;

    switch (mode) {
    case AutoMode:
        code = 1;
        break;
    case ManualMode:
        code = 2;
        break;
    default:
        return;
        break;
    }
    writeUint16(35, code);
}

void DeviceCommunication::readCompressor(QVector<quint16> &compressor1,
                                         QVector<quint16> &compressor2,
                                         QVector<quint16> &compressor3,
                                         QVector<quint16> &dryer1,
                                         QVector<quint16> &dryer2,
                                         QVector<quint16> &dryer3)
{
    QModbusDataUnit compressor1_readUnit(registerType4, 100, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor1_readUnit, [this,&compressor1](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
//            qDebug() << "compressor1_adress:" << currentAddres << "compressor1_value:" << value;
            compressor1.push_back(value);
        }
        emit readOverCompressorData1();
    });
    QModbusDataUnit compressor2_readUnit(registerType4, 150, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor2_readUnit, [this,&compressor2](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
            compressor2.push_back(value);
        }
        emit readOverCompressorData2();
    });
    QModbusDataUnit compressor3_readUnit(registerType4, 200, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor3_readUnit, [this,&compressor3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
            compressor3.push_back(value);
        }
        emit readOverCompressorData3();
    });
    QModbusDataUnit dryer1_readUnit(registerType4, 250, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer1_readUnit, [this,&dryer1](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
            dryer1.push_back(value);
        }
        emit readOverDryer1Data1();
    });
    QModbusDataUnit dryer2_readUnit(registerType4, 300, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer2_readUnit, [this,&dryer2](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
            dryer2.push_back(value);
        }
        emit readOverDryer1Data2();
    });
    QModbusDataUnit dryer3_readUnit(registerType4, 350, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer3_readUnit, [this,&dryer3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;
            dryer3.push_back(value);
        }
        emit readOverDryer1Data3();
    });

    // Waiting for receiving data.
    QEventLoop loop;
    connect(this, &DeviceCommunication::readOverCompressorData1,&loop, &QEventLoop::quit);
    loop.exec();
    connect(this, &DeviceCommunication::readOverCompressorData2,&loop, &QEventLoop::quit);
    loop.exec();
    connect(this, &DeviceCommunication::readOverCompressorData3,&loop, &QEventLoop::quit);
    loop.exec();
    connect(this, &DeviceCommunication::readOverDryer1Data1,&loop, &QEventLoop::quit);
    loop.exec();
    connect(this, &DeviceCommunication::readOverDryer1Data2,&loop, &QEventLoop::quit);
    loop.exec();
    connect(this, &DeviceCommunication::readOverDryer1Data3,&loop, &QEventLoop::quit);
    loop.exec();
    // Got it!

}
void DeviceCommunication::readDryer(QVector<quint16> &dryer1, QVector<quint16> &dryer2, QVector<quint16> &dryer3)
{
    QModbusDataUnit readUnit(registerType4, 250, 117);/*类型、首地址、长度*///13
    readRequest(readUnit, [this,&dryer1,&dryer2,&dryer3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            QString adress_str = tr("%1").arg(unit.startAddress() + i);
            QString value_str = tr("%1").arg(QString::number(unit.value(i)));
//            qDebug() << "Dryer_adress_str:" << adress_str << "Dryer_value_str:" << value_str;

            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;

            qDebug() << "Dryer_adress:" << currentAddres << "Dryer_value:" << value;

            if(currentAddres <=266 && currentAddres >=250){
                dryer1.push_back(value);
            }
            if(currentAddres <=316 && currentAddres >=300){
                dryer2.push_back(value);
            }
            if(currentAddres <=366 && currentAddres >=350){
                dryer3.push_back(value);
            }
        }
        emit readDryerData();
    });
    // Waiting for receiving data.
    QEventLoop loop;
    connect(this, &DeviceCommunication::readDryerData,&loop, &QEventLoop::quit);
    loop.exec();
    // Got it!
}

void DeviceCommunication::readCompressor_old(QVector<quint16> &compressor1,
                                         QVector<quint16> &compressor2,
                                         QVector<quint16> &compressor3,
                                         QVector<quint16> &dryer1,
                                         QVector<quint16> &dryer2,
                                         QVector<quint16> &dryer3)
{
    QModbusDataUnit readUnit(registerType4, 100, 130);/*类型、首地址、长度*/// 100   267
    readRequest(readUnit, [this,&compressor1,&compressor2,&compressor3,&dryer1,&dryer2,&dryer3](QModbusDataUnit unit){
        qDebug() << "unit.valueCount():" << unit.valueCount();
        for (uint i = 0; i < unit.valueCount(); i++) {
            QString adress_str = tr("%1").arg(unit.startAddress() + i);
            QString value_str = tr("%1").arg(QString::number(unit.value(i)));
//            qDebug() << "adress_str:" << adress_str << "value_str:" << value_str;

            quint16 value = unit.value(i);
            int currentAddres = unit.startAddress() + i;

//            qDebug() << "adress:" << currentAddres << "value:" << value;
            //空压机
            if(currentAddres <=134 && currentAddres >=100){
                compressor1.push_back(value);
            }
            if(currentAddres <=184 && currentAddres >=150){
                compressor2.push_back(value);
            }
            if(currentAddres <=234 && currentAddres >=200){
                compressor3.push_back(value);
            }
            //冷干机
            if(currentAddres <=266 && currentAddres >=250){
                dryer1.push_back(value);
            }
            if(currentAddres <=316 && currentAddres >=300){
                dryer2.push_back(value);
            }
            if(currentAddres <=366 && currentAddres >=350){
                dryer3.push_back(value);
            }
        }
        emit readALLOverData();
    });
    QEventLoop loop;
    connect(this, &DeviceCommunication::readALLOverData,&loop, &QEventLoop::quit);
    loop.exec();

}

void DeviceCommunication::readCompressor1(QVector<quint16> &compressor1)
{
    QModbusDataUnit compressor1_readUnit(registerType4, 100, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor1_readUnit, [this,&compressor1](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            compressor1.push_back(value);
        }
        emit readOverCompressorData1();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverCompressorData1,&loop, &QEventLoop::quit);
        loop.exec();
    }

}
void DeviceCommunication::readCompressor2(QVector<quint16> &compressor2)
{

    QModbusDataUnit compressor2_readUnit(registerType4, 150, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor2_readUnit, [this,&compressor2](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            compressor2.push_back(value);
        }
        emit readOverCompressorData2();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverCompressorData2,&loop, &QEventLoop::quit);
        loop.exec();
    }
}
void DeviceCommunication::readCompressor3(QVector<quint16> &compressor3)
{
    QModbusDataUnit compressor3_readUnit(registerType4, 200, 35);/*类型、首地址、长度*/// 100   267
    readRequest(compressor3_readUnit, [this,&compressor3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            compressor3.push_back(value);
        }
        emit readOverCompressorData3();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverCompressorData3,&loop, &QEventLoop::quit);
        loop.exec();
    }
}
void DeviceCommunication::dryer1(QVector<quint16> &dryer1)
{
    QModbusDataUnit dryer1_readUnit(registerType4, 250, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer1_readUnit, [this,&dryer1](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            dryer1.push_back(value);
        }
        emit readOverDryer1Data1();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverDryer1Data1,&loop, &QEventLoop::quit);
        loop.exec();
    }
}
void DeviceCommunication::dryer2(QVector<quint16> &dryer2)
{
    QModbusDataUnit dryer2_readUnit(registerType4, 300, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer2_readUnit, [this,&dryer2](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            dryer2.push_back(value);
        }
        emit readOverDryer1Data2();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverDryer1Data2,&loop, &QEventLoop::quit);
        loop.exec();
    }
}
void DeviceCommunication::dryer3(QVector<quint16> &dryer3)
{
    QModbusDataUnit dryer3_readUnit(registerType4, 350, 17);/*类型、首地址、长度*/// 100   267
    readRequest(dryer3_readUnit, [this,&dryer3](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            dryer3.push_back(value);
        }
        emit readOverDryer1Data3();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readOverDryer1Data3,&loop, &QEventLoop::quit);
        loop.exec();
    }
}

void DeviceCommunication::readWarningHint(QVector<quint16> &warningInfo)
{
    QModbusDataUnit readUnit(registerType4, 75, 6);/*类型、首地址、长度*/// 100   267
    readRequest(readUnit, [this,&warningInfo](QModbusDataUnit unit){
        for (uint i = 0; i < unit.valueCount(); i++) {
            quint16 value = unit.value(i);
            warningInfo.push_back(value);
//            qDebug() << "warningInfo:"<<value;
        }
        emit readWarningHintInfo();
    });
    if(modbusDevice){
        QEventLoop loop;
        connect(this, &DeviceCommunication::readWarningHintInfo,&loop, &QEventLoop::quit);
        loop.exec();
    }
}
void DeviceCommunication::compressorEnable1(bool off)
{
    writeUint16(85, (off ? 0 : 1));
}
void DeviceCommunication::compressorEnable2(bool off)
{
    writeUint16(86, (off ? 0 : 1));
}
void DeviceCommunication::compressorEnable3(bool off)
{
    writeUint16(87, (off ? 0 : 1));
}
void DeviceCommunication::dryerEnable1(bool off)
{
    writeUint16(88, (off ? 0 : 1));
}
void DeviceCommunication::dryerEnable2(bool off)
{
    writeUint16(89, (off ? 0 : 1));
}
void DeviceCommunication::dryerEnable3(bool off)
{
    writeUint16(90, (off ? 0 : 1));
}


void DeviceCommunication::readUint16(int address_, int count_, QVector<quint16> &buffer_)
{
    buffer_.clear();
    int num = count_ / 120;
    for(int i = 0; i <= num; i++)
    {
        int count = count_ > 120 ? 120 : (count_ % 120);
        count_ = count_ - 120;

        QModbusDataUnit readUnit(registerType4, address_ + 120*i, count);
        this->readRequest(readUnit,[&](QModbusDataUnit unit)
        {
            for(int j = 0; j < count; j++)
            {
                buffer_.push_back(unit.value(j));
            }
            emit readUint16Signal_D();
        });
        if(modbusDevice){
            QEventLoop loop;
            connect(this, &DeviceCommunication::readUint16Signal_D,&loop, &QEventLoop::quit);
            loop.exec();
        }
    }
}



//以毫秒为单位的延时函数
void DeviceCommunication::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
