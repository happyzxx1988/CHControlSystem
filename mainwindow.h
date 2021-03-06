﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dataoper.h>
#include <QTableWidget>
#include "userdialog.h"
#include "pressuresetdialog.h"
#include "compressorsetdialog.h"
#include "appcore.h"
#include <QtCharts>
#include "warninghintdialog.h"

#define STATION_HOUSE   "102"        //站房102-3    201-对面-2   202-2
#define STORE_TIME      60000      //储存读取数据的时间间隔  暂定1分钟
#define STORE_TIME2     60000      //储存读取数据的时间间隔  5分钟存储一次  20201204 zxx

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int num,QString u,QString p,QWidget *parent = 0);
    ~MainWindow();

private slots:
    void navigation_pressed();
    void checkBox_pressed(bool checked);
    void timerUpdate(void);
    void on_userAddBtn_clicked();
    void on_userEditBtn_clicked();
    void on_userClearBtn_clicked();
    void on_userSerachBtn_clicked();
    void checkRowSlot(int row, int col);
    void on_userDeleteBtn_clicked();
    void on_warningSearchBtn_clicked();
    void on_warningResetBtn_clicked();
    void on_logSearchBtn_clicked();
    void on_logResetBtn_clicked();
    void on_dryerBtn1_clicked();
    void on_dryerBtn2_clicked();
    void on_dryerBtn3_clicked();
    void on_pressureSetBtn_clicked();
    void on_runResetBtn_clicked();
    void on_compressorBtn1_clicked();
    void on_compressorBtn2_clicked();
    void on_compressorBtn3_clicked();
    void on_compressorSetBtn1_clicked();
    void on_compressorSetBtn2_clicked();
    void on_compressorSetBtn3_clicked();
    void on_manualOper_clicked();
    void on_autoOper_clicked();
    void compressor_call_back(int compressorNO, float uninstallPressure, float pressureDiff);
    void pressure_call_back(float maxPressure, float minPressure);
    void userEdit_call_back(int userEditType);
    void readCompressorTimer();
    void on_loadDataBtn_clicked();
    void slotPointHoverd(const QPointF &point, bool state);
    void slotPointHoverd2(const QPointF &point, bool state);

    void on_connectPLCBtn_clicked();

    void readWarningHintTimer();

    void warningHint_call_back(bool a, bool b, bool c, bool d, bool e, bool f);

    void radioButton_pressed(bool checked);

    void on_fristButton_clicked();

    void on_prevButton_clicked();

    void on_switchPageButton_clicked();

    void on_nextButton_clicked();

    void on_lastButton_clicked();

    void on_loadDataBtn_2_clicked();

    void on_deleteBtn_clicked();

    void on_fristButton_2_clicked();

    void on_prevButton_2_clicked();

    void on_switchPageButton_2_clicked();

    void on_nextButton_2_clicked();

    void on_lastButton_2_clicked();

private:
    void initForm();
    void clearTable(QTableWidget *table);
    void initTable();
    void dealCompressor1(QVector<quint16> compressor, QVector<quint16> dryer, QVector<quint16> float2);
    void dealCompressor2(QVector<quint16> compressor, QVector<quint16> dryer);
    void dealCompressor3(QVector<quint16> compressor, QVector<quint16> dryer);
    void initChart();
    void initChart2();
    QVector<bool> dec2BinTrans(unsigned int data);
    void saveWarning(quint16 warningInfo, int compressorNo, int runState);
    void saveWarningDryer(int dryerNo, int runState);

    void binToDec(unsigned int data, int &h_val, int &l_val);
    bool PlcIsConnect();
    void lockUiOperation();
    void unlockUiOperation();
    void getInitEquipmentStatus();

    QString getDefineTimeByDay(int day);

    void GetTotalRecordCount();
    void GetPageCount();
    void initDatatable();
    void RecordQuery(int limitIndex);
    void UpdateStatus();


    void GetTotalRecordCount_w();
    void GetPageCount_w();
    void initDatatable_w();
    void RecordQuery_w(int limitIndex);
    void UpdateStatus_w();



    bool radioButtonIsChecked();

    void int_to_float(quint16 a,quint16 b, float &buffer_,QString analyticalModel_ = "ABCD");

    void deleteDataSure();

    void saveBatchTimingData(QVector<quint16> buffer_);

    void saveData(vector<Compressor> c1, vector<Compressor> c2, vector<Compressor> c3,
                              vector<Dryer> d1, vector<Dryer> d2, vector<Dryer> d3, vector<Warning> w);

private:
    Ui::MainWindow *ui;
    QString userName;
    QString pass;
    QString edit_userName;
    QString edit_pass;
    int device_num;
    DataOper dataOper;
    UserDialog *userDialog;
    PressureSetDialog *pressureSetDialog;
    CompressorSetDialog *compressorSetDialog;
    bool dryerSwitch1;
    bool dryerSwitch2;
    bool dryerSwitch3;
    bool compressorSwitch1;
    bool compressorSwitch2;
    bool compressorSwitch3;
    AppCore appcore;

    int storageInterval;//存储读取数据的时间间隔
    int storageInterval2;//存储读取数据的时间间隔
    int READ_TIME;
    QTimer compressorTimer;
    QTimer timer;

    bool PLCIsConnected;

    bool isShowWarning;
    bool compressorIsShowWarning1;
    bool compressorIsShowWarning2;
    bool compressorIsShowWarning3;
    bool dryerIsShowWarning1;
    bool dryerIsShowWarning2;
    bool dryerIsShowWarning3;

    bool isGetEquipmentStatus;
    bool isFristGetEquipmentStatus1;
    bool isFristGetEquipmentStatus2;
    bool isFristGetEquipmentStatus3;
    bool isFristGetEquipmentStatus4;
    bool isFristGetEquipmentStatus5;
    bool isFristGetEquipmentStatus6;

    WarningHintDialog *warningHintDialog;



    QDateTimeAxis *up_x;
    QValueAxis    *up_y;
    QLineSeries   *up_series;
    QChart        *up_chart;

    QDateTimeAxis *pd_x;
    QValueAxis    *pd_y;
    QLineSeries   *pd_series;
    QChart        *pd_chart;

    QLabel        *m_valueLabel;


    QDateTimeAxis *p2_x;
    QValueAxis    *p2_y;
    QLineSeries   *p2_series;
    QChart        *p2_chart;

    QDateTimeAxis *T_x;
    QValueAxis    *T_y;
    QLineSeries   *T_series;
    QChart        *T_chart;

    QLabel        *m_valueLabel2;

    //批量存储空压机、冷干机采集数据，批量存储空压机、冷干机报警数据

    vector<Compressor> compressors1;
    vector<Compressor> compressors2;
    vector<Compressor> compressors3;
    vector<Dryer> dryers1;
    vector<Dryer> dryers2;
    vector<Dryer> dryers3;

    vector<Warning> warnings;

    bool isStoreData;




    //table 分页处理
    int       currentPage;      //当前页
    int       totalPage;    //总页数
    int       totalRecrodCount;     //总记录数
    int       PageRecordCount;      //每页记录数
    QString   tableName;

    //报警数据  table 分页处理
    int       currentPage_w;      //当前页
    int       totalPage_w;    //总页数
    int       totalRecrodCount_w;     //总记录数
    int       PageRecordCount_w;      //每页记录数

};

#endif // MAINWINDOW_H
