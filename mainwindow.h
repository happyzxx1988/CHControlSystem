#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dataoper.h>
#include <QTableWidget>
#include "userdialog.h"
#include "pressuresetdialog.h"
#include "compressorsetdialog.h"
#include "appcore.h"
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString u,QString p,QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_listView_pressed();
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

private:
    void initForm();
    void clearTable(QTableWidget *table);
    void initTable();
    void dealCompressor1(QVector<quint16> compressor, QVector<quint16> dryer);
    void dealCompressor2(QVector<quint16> compressor, QVector<quint16> dryer);
    void dealCompressor3(QVector<quint16> compressor, QVector<quint16> dryer);
    void initChart();
    QVector<bool> dec2BinTrans(unsigned int data);
    void saveWarning(quint16 warningInfo, int compressorNo, int runState);
    void saveWarningDryer(int dryerNo, int runState);

private:
    Ui::MainWindow *ui;
    QString userName;
    QString pass;
    QString edit_userName;
    QString edit_pass;
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
    int READ_TIME;


    QDateTimeAxis *up_x;
    QValueAxis    *up_y;
    QLineSeries   *up_series;
    QChart        *up_chart;

    QDateTimeAxis *pd_x;
    QValueAxis    *pd_y;
    QLineSeries   *pd_series;
    QChart        *pd_chart;

    QLabel        *m_valueLabel;

};

#endif // MAINWINDOW_H
