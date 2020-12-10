#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDesktopWidget>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

//#define STATION_HOUSE   "202"        //站房201-3    102-2   202-2
//#define STORE_TIME      60000      //储存读取数据的时间间隔  暂定1分钟

MainWindow::MainWindow(int num, QString u, QString p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    userName(u),
    pass(p),
    device_num(num)
{
    ui->setupUi(this);
    this->initForm();
    this->initTable();
    this->initChart();
    this->initChart2();


    ui->sys_img->setPixmap(QPixmap(":/images/images/log.png"));
    ui->sys_name->setText("双创园空压机站房恒压控制系统");
    setWindowTitle("双创园空压机站房恒压控制系统");

}

MainWindow::~MainWindow()
{
    delete ui;
    if(compressorTimer.isActive()){
        compressorTimer.stop();
    }

    QProcess *process = new QProcess;
    process->start("taskkill", QStringList() << "/f" << "/im" << "CHControlSystem.exe");
}


void MainWindow::initForm()
{
    QDate date = QDate::currentDate();   //获取当前日期
    READ_TIME =      1000;    //定时读取间隔时间

    ui->logResetBtn->hide();
    ui->warningResetBtn->hide();

    ui->warningStartime->setDate(date);
    ui->warningEndTime->setDate(date);
    ui->logStartime->setDate(date);
    ui->logEndTime->setDate(date);
    ui->runningChartS->setDate(date);
    ui->runningChartE->setDate(date);
    ui->runningChartS_2->setDate(date);
    ui->runningChartE_2->setDate(date);

    ui->warningStartime->setDisplayFormat("yyyy-MM-dd");
    ui->warningEndTime->setDisplayFormat("yyyy-MM-dd");
    ui->logStartime->setDisplayFormat("yyyy-MM-dd");
    ui->logEndTime->setDisplayFormat("yyyy-MM-dd");
    ui->runningChartS->setDisplayFormat("yyyy-MM-dd");
    ui->runningChartE->setDisplayFormat("yyyy-MM-dd");
    ui->runningChartS_2->setDisplayFormat("yyyy-MM-dd");
    ui->runningChartE_2->setDisplayFormat("yyyy-MM-dd");

    ui->currentUser->setText(userName);
    edit_userName = nullptr;
    edit_pass = nullptr;
    ui->stackedWidget->setCurrentIndex(0);
    dryerSwitch1 = false;
    dryerSwitch2 = false;
    dryerSwitch3 = false;
    compressorSwitch1 = false;
    compressorSwitch2 = false;
    compressorSwitch3 = false;
    isShowWarning = true;
    compressorIsShowWarning1 = true;
    compressorIsShowWarning2 = true;
    compressorIsShowWarning3 = true;
    dryerIsShowWarning1 = true;
    dryerIsShowWarning2 = true;
    dryerIsShowWarning3 = true;
    PLCIsConnected = false;

    isGetEquipmentStatus = true;
    isFristGetEquipmentStatus1 = true;
    isFristGetEquipmentStatus2 = true;
    isFristGetEquipmentStatus3 = true;
    isFristGetEquipmentStatus4 = true;
    isFristGetEquipmentStatus5 = true;
    isFristGetEquipmentStatus6 = true;

    isStoreData = true;

    lockUiOperation();

    storageInterval = 0;
    storageInterval2 = 0;

    QStringList aa ;
    switch (device_num) {
    case 2:
        aa  << "选择空压机" << "1#空压机" << "2#空压机";
        ui->groupBox_12->hide();
        ui->groupBox_15->hide();
        ui->groupBox_21->hide();
        ui->groupBox_24->hide();
        ui->groupBox_12->hide();
        ui->groupBox_15->hide();
        ui->groupBox_8->hide();
        ui->groupBox_5->hide();
        ui->stopCompressor3->hide();
        ui->stopDryer3->hide();
        ui->radioButton_3->hide();
        ui->radioButton_6->hide();
        break;
    case 3:
        aa  << "选择空压机" << "1#空压机" << "2#空压机" << "3#空压机";
        break;
    }
    ui->compressorNO->clear();
    ui->compressorNO->addItems(aa);


    connect(&timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer.start(1000);

    connect(&compressorTimer,SIGNAL(timeout()),this,SLOT(readCompressorTimer()));

//    connect(&compressorTimer,SIGNAL(timeout()),this,SLOT(readWarningHintTimer()));

    connect(ui->userTable,SIGNAL(cellClicked(int,int)),this,SLOT(checkRowSlot(int,int)));
    /*数据处理的消息传输*/
    connect(&dataOper, &DataOper::sendDataMessage, this, [this](const QString& info){
        QMessageBox::information(this, tr("提示"), info,tr("确定"));
    });
    connect(&appcore, &AppCore::deviceConnected, this, [this](){
        qDebug() << "emit deviceConnected()";

        ui->connectPLCBtn->setText("已连接");
        ui->connectPLCBtn->setStyleSheet("background-color: rgb(85, 255, 0);");
        ui->connectPLCBtn->setEnabled(false);

        unlockUiOperation();

        PLCIsConnected = true;

        //设备启动，设置手动模式
        appcore.setRunMode(ManualMode);
        ui->runningMode->setText("手动");
        ui->manualOper->setChecked(true);
//        getInitEquipmentStatus();//读取设备初始化状态

        if(!compressorTimer.isActive()){
            compressorTimer.start(READ_TIME);
        }

    });
    connect(&appcore, &AppCore::deviceDisconnected, this, [this](){
        qDebug() << "emit deviceDisconnected()";
        ui->connectPLCBtn->setText("已断开");
        ui->connectPLCBtn->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->connectPLCBtn->setEnabled(true);
        lockUiOperation();
        PLCIsConnected = false;
        if(compressorTimer.isActive()){
            compressorTimer.stop();
        }
    });
    connect(&appcore, &AppCore::errorMessage, this, [this](const QString& error){
        qDebug() << "emit errorMessage(error):" << error;
        ui->statusBar->showMessage(error);
    });
    connect(&appcore, &AppCore::infoMessage, this, [this](const QString& info){
        qDebug() << "emit infoMessage(info):" << info;
    });
    connect(ui->pushButton_1, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_8, &QPushButton::clicked, this, &MainWindow::navigation_pressed);
    connect(ui->pushButton_9, &QPushButton::clicked, this, &MainWindow::navigation_pressed);


    connect(ui->radioButton_1, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);
    connect(ui->radioButton_3, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);
    connect(ui->radioButton_4, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);
    connect(ui->radioButton_5, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);
    connect(ui->radioButton_6, &QRadioButton::toggled, this, &MainWindow::radioButton_pressed);


    connect(ui->stopCompressor1, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);
    connect(ui->stopCompressor2, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);
    connect(ui->stopCompressor3, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);
    connect(ui->stopDryer1, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);
    connect(ui->stopDryer2, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);
    connect(ui->stopDryer3, &QCheckBox::toggled, this, &MainWindow::checkBox_pressed);


    // Initialize settings
    appcore.initSettings();
    // Connect to Device
    appcore.initDevice();

    //删除给定时间之前的数据，包括采集数据，警告数据，日志数据 默认保留10的数据

//    QString defineTime = this->getDefineTimeByDay(-10);


    //不需要系统高压、系统低压参数
    ui->label_101->hide();
    ui->refrigerantH1->hide();
    ui->label_70->hide();

    ui->label_107->hide();
    ui->refrigerantH2->hide();
    ui->label_84->hide();

    ui->label_113->hide();
    ui->refrigerantH3->hide();
    ui->label_98->hide();

    ui->label_103->hide();
    ui->refrigerantL1->hide();
    ui->label_54->hide();

    ui->label_109->hide();
    ui->refrigerantL2->hide();
    ui->label_88->hide();

    ui->label_115->hide();
    ui->refrigerantL3->hide();
    ui->label_100->hide();


}
void MainWindow::initTable()
{
    //设置列数和列宽 用户管理
    int width = qApp->desktop()->availableGeometry().width() - 120;
    ui->userTable->setColumnCount(5);
    ui->userTable->setColumnWidth(0, width * 0.03);
    ui->userTable->setColumnWidth(1, width * 0.07);
    ui->userTable->setColumnWidth(2, width * 0.15);
    ui->userTable->setColumnWidth(3, width * 0.08);
    ui->userTable->setColumnWidth(4, width * 0.06);
    ui->userTable->verticalHeader()->setDefaultSectionSize(25);
    QStringList UserHeadText;
    UserHeadText << "序号" << "用户名" << "密码(已加密)" << "创建时间" << "是否可用";
    ui->userTable->setHorizontalHeaderLabels(UserHeadText);
    ui->userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->userTable->setAlternatingRowColors(true);
    ui->userTable->verticalHeader()->setVisible(false);
    ui->userTable->horizontalHeader()->setStretchLastSection(true);
    //设置行高
    ui->userTable->setRowCount(30);

    //设置列数和列宽 系统报警
    ui->warningTable->setColumnCount(5);
    ui->warningTable->setColumnWidth(0, width * 0.03);
    ui->warningTable->setColumnWidth(1, width * 0.06);
    ui->warningTable->setColumnWidth(2, width * 0.08);
    ui->warningTable->setColumnWidth(3, width * 0.15);
    ui->warningTable->setColumnWidth(4, width * 0.10);
    ui->warningTable->verticalHeader()->setDefaultSectionSize(25);
    QStringList warningHeadText;
    warningHeadText << "序号" << "站房" << "设备" << "报警" << "触发时间";
    ui->warningTable->setHorizontalHeaderLabels(warningHeadText);
    ui->warningTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->warningTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->warningTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->warningTable->setAlternatingRowColors(true);
    ui->warningTable->verticalHeader()->setVisible(false);
    ui->warningTable->horizontalHeader()->setStretchLastSection(true);
    //设置行高
    ui->warningTable->setRowCount(30);

    //设置列数和列宽 设备运行记录
    ui->logTable->setColumnCount(6);
    ui->logTable->setColumnWidth(0, width * 0.03);
    ui->logTable->setColumnWidth(1, width * 0.03);
    ui->logTable->setColumnWidth(2, width * 0.08);
    ui->logTable->setColumnWidth(3, width * 0.15);
    ui->logTable->setColumnWidth(4, width * 0.06);
    ui->logTable->setColumnWidth(5, width * 0.06);
    ui->logTable->verticalHeader()->setDefaultSectionSize(25);
    QStringList logHeadText;
    logHeadText << "序号" << "站房" << "设备" << "操作说明" << "用户" << "时间";
    ui->logTable->setHorizontalHeaderLabels(logHeadText);
    ui->logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTable->setAlternatingRowColors(true);
    ui->logTable->verticalHeader()->setVisible(false);
    ui->logTable->horizontalHeader()->setStretchLastSection(true);
    //设置行高
    ui->logTable->setRowCount(30);



}
void MainWindow::initChart()
{
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet(QString("QLabel{color:#1564FF; font-family:\"Microsoft Yahei\"; font-size:12px; font-weight:bold;"
                                        " background-color:rgba(21, 100, 255, 51); border-radius:4px; text-align:center;}"));
    m_valueLabel->setFixedSize(180, 30);
    m_valueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_valueLabel->hide();
    ui->upWidget->setRenderHint(QPainter::Antialiasing);
    ui->pdWidget->setRenderHint(QPainter::Antialiasing);

    up_series = new QLineSeries();
    up_x = new QDateTimeAxis();
    up_y = new QValueAxis();
    up_chart = ui->upWidget->chart();

    pd_series = new QLineSeries();
    pd_x = new QDateTimeAxis();//X轴
    pd_y = new QValueAxis();//Y轴
    pd_chart = ui->pdWidget->chart();

    up_x->setTickCount(10);
    up_x->setFormat("yyyy-MM-dd hh:mm:ss");
    up_x->setLabelsAngle(20);
    up_x->setTitleText("采集时间");
    up_y->setTitleText("卸载压力");
    up_chart->addSeries(up_series);
    up_chart->setAxisX(up_x, up_series);
    up_chart->setAxisY(up_y, up_series);
    up_chart->legend()->hide();
    up_chart->setTitle("卸载压力时间变化曲线");
    up_chart->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    up_chart->setDropShadowEnabled(true);

    pd_x->setTickCount(10);
    pd_x->setFormat("yyyy-MM-dd hh:mm:ss");
    pd_x->setLabelsAngle(20);
    pd_x->setTitleText("采集时间");
    pd_y->setTitleText("加载压差");
    pd_chart->addSeries(pd_series);
    pd_chart->setAxisX(pd_x, pd_series);
    pd_chart->setAxisY(pd_y, pd_series);
    pd_chart->legend()->hide();
    pd_chart->setTitle("加载压差时间变化曲线");
    pd_chart->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    pd_chart->setDropShadowEnabled(true);

    connect(up_series, &QLineSeries::hovered, this, &MainWindow::slotPointHoverd);//用于鼠标移动到点上显示数值
    connect(pd_series, &QLineSeries::hovered, this, &MainWindow::slotPointHoverd);//用于鼠标移动到点上显示数值
}
void MainWindow::initChart2()
{
    m_valueLabel2 = new QLabel(this);
    m_valueLabel2->setStyleSheet(QString("QLabel{color:#1564FF; font-family:\"Microsoft Yahei\"; font-size:12px; font-weight:bold;"
                                        " background-color:rgba(21, 100, 255, 51); border-radius:4px; text-align:center;}"));
    m_valueLabel2->setFixedSize(180, 30);
    m_valueLabel2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_valueLabel2->hide();
    ui->P2Widget->setRenderHint(QPainter::Antialiasing);
    ui->dewPointTWidget->setRenderHint(QPainter::Antialiasing);

    p2_series = new QLineSeries();
    p2_x = new QDateTimeAxis();
    p2_y = new QValueAxis();
    p2_chart = ui->P2Widget->chart();

    T_series = new QLineSeries();
    T_x = new QDateTimeAxis();//X轴
    T_y = new QValueAxis();//Y轴
    T_chart = ui->dewPointTWidget->chart();

    p2_x->setTickCount(10);
    p2_x->setFormat("yyyy-MM-dd hh:mm:ss");
    p2_x->setLabelsAngle(20);
    p2_x->setTitleText("采集时间");
    p2_y->setTitleText("排气压力");
    p2_chart->addSeries(p2_series);
    p2_chart->setAxisX(p2_x, p2_series);
    p2_chart->setAxisY(p2_y, p2_series);
    p2_chart->legend()->hide();
    p2_chart->setTitle("排气压力时间变化曲线");
    p2_chart->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    p2_chart->setDropShadowEnabled(true);

    T_x->setTickCount(10);
    T_x->setFormat("yyyy-MM-dd hh:mm:ss");
    T_x->setLabelsAngle(20);
    T_x->setTitleText("采集时间");
    T_y->setTitleText("露点温度");
    T_chart->addSeries(T_series);
    T_chart->setAxisX(T_x, T_series);
    T_chart->setAxisY(T_y, T_series);
    T_chart->legend()->hide();
    T_chart->setTitle("露点温度时间变化曲线");
    T_chart->setTitleFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    T_chart->setDropShadowEnabled(true);

    connect(p2_series, &QLineSeries::hovered, this, &MainWindow::slotPointHoverd2);//用于鼠标移动到点上显示数值
    connect(T_series, &QLineSeries::hovered, this, &MainWindow::slotPointHoverd2);//用于鼠标移动到点上显示数值
}

//历史数据图表
void MainWindow::on_loadDataBtn_clicked()
{
    QString s_time = ui->runningChartS->text();//开始时间
    QString e_time = ui->runningChartE->text();//结束时间

    vector<Compressor> compressors;
    int compressorNo = ui->compressorNO->currentIndex();
    QDateTime start = QDateTime::fromString(s_time, "yyyy-MM-dd");
    QDateTime end = QDateTime::fromString(e_time, "yyyy-MM-dd");
    uint stime = start.toTime_t();
    uint etime = end.toTime_t();
    int end_ = stime - etime;
    if(end_ > 0 ){
        QMessageBox::information(this, "提示", "结束日期小于开始日期，重新选择日期","确定");
        return;
    }
    if(compressorNo == 0 ){
        QMessageBox::information(this, "提示", "选择需要加载的空压机编号","确定");
        return;
    }

    dataOper.getCompressorInfo(compressors,compressorNo,s_time,e_time);//降序
    int compressors_size = compressors.size();
    if(compressors_size == 0){
        up_series->clear();
        pd_series->clear();
        return;
    }
    up_series->clear();
    pd_series->clear();
    double up_y_max = compressors.at(0).uninstallPressure;
    double up_y_min = compressors.at(0).uninstallPressure;
    double pd_y_max = compressors.at(0).pressureDiff;
    double pd_y_min = compressors.at(0).pressureDiff;

    for(int i = 0; i < compressors_size; ++i){
        up_y_max = compressors.at(i).uninstallPressure > up_y_max ? compressors.at(i).uninstallPressure : up_y_max;
        up_y_min = compressors.at(i).uninstallPressure < up_y_min ? compressors.at(i).uninstallPressure : up_y_min;
        pd_y_max = compressors.at(i).pressureDiff > pd_y_max ? compressors.at(i).pressureDiff : pd_y_max;
        pd_y_min = compressors.at(i).pressureDiff < pd_y_min ? compressors.at(i).pressureDiff : pd_y_min;

        up_series->append(compressors.at(i).date.toMSecsSinceEpoch(), compressors.at(i).uninstallPressure);
        pd_series->append(compressors.at(i).date.toMSecsSinceEpoch(), compressors.at(i).pressureDiff);
    }

    up_x->setRange(compressors.at(compressors_size - 1).date, compressors.at(0).date);
    up_y->setRange(up_y_min,up_y_max);

    pd_x->setRange(compressors.at(compressors_size - 1).date, compressors.at(0).date);
    pd_y->setRange(pd_y_min,pd_y_max);
}

void MainWindow::on_loadDataBtn_2_clicked()
{
    QString s_time = ui->runningChartS_2->text();//开始时间
    QString e_time = ui->runningChartE_2->text();//结束时间
    vector<Compressor> compressors;
    vector<Dryer> dryers;
    int compressorNo = ui->compressorNO_2->currentIndex();
    int dryerNO = ui->dryerNO->currentIndex();
    QDateTime start = QDateTime::fromString(s_time, "yyyy-MM-dd");
    QDateTime end = QDateTime::fromString(e_time, "yyyy-MM-dd");
    uint stime = start.toTime_t();
    uint etime = end.toTime_t();
    int end_ = stime - etime;
    if(end_ > 0 ){
        QMessageBox::information(this, "提示", "结束日期小于开始日期，重新选择日期","确定");
        return;
    }
    if(compressorNo == 0 && dryerNO == 0){
        QMessageBox::information(this, "提示", "选择需要加载的空压机编号或者冷干机编号","确定");
        return;
    }

    dataOper.getCompressorInfo(compressors,compressorNo,s_time,e_time);//降序 排气压力，空压机
    dataOper.getDryerInfo(dryers,dryerNO,s_time,e_time);//降序 露点温度，冷干机

    int compressors_size = compressors.size();
    int dryers_size = dryers.size();
    if(compressors_size != 0){
        p2_series->clear();
        double p2_y_max = compressors.at(0).P2;
        double p2_y_min = compressors.at(0).P2;

        for(int i = 0; i < compressors_size; ++i){
            p2_y_max = compressors.at(i).P2 > p2_y_max ? compressors.at(i).P2 : p2_y_max;
            p2_y_min = compressors.at(i).P2 < p2_y_min ? compressors.at(i).P2 : p2_y_min;

            p2_series->append(compressors.at(i).date.toMSecsSinceEpoch(), compressors.at(i).P2);
        }

        p2_x->setRange(compressors.at(compressors_size - 1).date, compressors.at(0).date);
        p2_y->setRange(p2_y_min-0.1,p2_y_max+0.1);
    }else{
        p2_series->clear();
    }

    if(dryers_size != 0){
        T_series->clear();
        double T_y_max = dryers.at(0).dewPointT;
        double T_y_min = dryers.at(0).dewPointT;

        for(int i = 0; i < dryers_size; ++i){
            T_y_max = dryers.at(i).dewPointT > T_y_max ? dryers.at(i).dewPointT : T_y_max;
            T_y_min = dryers.at(i).dewPointT < T_y_min ? dryers.at(i).dewPointT : T_y_min;

            T_series->append(dryers.at(i).date.toMSecsSinceEpoch(), dryers.at(i).dewPointT);
        }

        T_x->setRange(dryers.at(dryers_size - 1).date, dryers.at(0).date);
        T_y->setRange(T_y_min-10,T_y_max+10);
    }else{
        T_series->clear();
    }



}
//界面切换函数
void MainWindow::navigation_pressed()
{
    QPushButton *b = (QPushButton *)sender();
    QString text = b->text();

    if (text == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (text == "设备状态") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (text == "设备控制") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (text == "操作日志") {
        ui->stackedWidget->setCurrentIndex(3);
        on_logResetBtn_clicked();
    } else if (text == "系统报警") {
        ui->stackedWidget->setCurrentIndex(4);
//        on_warningResetBtn_clicked();
    } else if (text == "变化曲线") {
        ui->stackedWidget->setCurrentIndex(8);  //   5是加载压差  卸载压力
    } else if (text == "用户管理") {
        ui->stackedWidget->setCurrentIndex(6);
        on_userClearBtn_clicked();
    } else if (text == "历史数据") {
        ui->stackedWidget->setCurrentIndex(7);
    } else if (text == "数据管理") {
        ui->stackedWidget->setCurrentIndex(9);
    }
}
//设置是否启用报警弹窗功能
void MainWindow::checkBox_pressed(bool checked)
{

    QCheckBox *b = (QCheckBox *)sender();
    QString text = b->text();
    if(!PLCIsConnected){
        return;
    }
    if (text == "1#空压机是否启用") {
        if(isFristGetEquipmentStatus1){
            isFristGetEquipmentStatus1 = false;
        }else{
            compressorIsShowWarning1 = checked;
            appcore.setEquipmentEnable(1,checked);
        }
    } else if (text == "2#空压机是否启用") {
        if(isFristGetEquipmentStatus2){
            isFristGetEquipmentStatus2 = false;
        }else{
            compressorIsShowWarning2 = checked;
            appcore.setEquipmentEnable(2,checked);
        }
    } else if (text == "3#空压机是否启用") {
        if(isFristGetEquipmentStatus3){
            isFristGetEquipmentStatus3 = false;
        }else{
            compressorIsShowWarning3 = checked;
            appcore.setEquipmentEnable(3,checked);
        }
    } else if (text == "1#冷干机是否启用") {
        if(isFristGetEquipmentStatus4){
            isFristGetEquipmentStatus4 = false;
        }else{
            dryerIsShowWarning1 = checked;
            appcore.setEquipmentEnable(4,checked);
        }
    } else if (text == "2#冷干机是否启用") {
        if(isFristGetEquipmentStatus5){
            isFristGetEquipmentStatus5 = false;
        }else{
            dryerIsShowWarning2 = checked;
            appcore.setEquipmentEnable(5,checked);
        }
    } else if (text == "3#冷干机是否启用") {
        if(isFristGetEquipmentStatus6){
            isFristGetEquipmentStatus6 = false;
        }else{
            dryerIsShowWarning3 = checked;
            appcore.setEquipmentEnable(6,checked);
        }
    }
}

//连接PLC
void MainWindow::on_connectPLCBtn_clicked()
{
    appcore.connectPLC();
//    int h_val = 0;
//    int l_val = 0;
//    binToDec(3078, h_val, l_val);
//    qDebug() << "h_val:" << h_val <<"----" << "l_val:" << l_val ;

//    dec2BinTrans(3078);
}

//用户管理--增加用户
void MainWindow::on_userAddBtn_clicked()
{
    userDialog = new UserDialog(userName,edit_userName, edit_pass,1);
    connect(userDialog,&UserDialog::closeCurrentDialog,this,&MainWindow::userEdit_call_back);
    edit_userName = nullptr;
    edit_pass = nullptr;
    userDialog->setAttribute(Qt::WA_DeleteOnClose);
    userDialog->setWindowModality(Qt::ApplicationModal);
    userDialog->show();
}
//用户管理--修改用户
void MainWindow::on_userEditBtn_clicked()
{
    if(edit_userName == nullptr){
        QMessageBox::information(this, "提示", "选择需要修改的用户","确定");
        return;
    }
    userDialog = new UserDialog(userName,edit_userName, edit_pass,2);
    connect(userDialog,&UserDialog::closeCurrentDialog,this,&MainWindow::userEdit_call_back);
    edit_userName = nullptr;
    edit_pass = nullptr;
    userDialog->setAttribute(Qt::WA_DeleteOnClose);
    userDialog->setWindowModality(Qt::ApplicationModal);
    userDialog->show();
}
//用户管理--清空搜索条件
void MainWindow::on_userClearBtn_clicked()
{
    ui->searchName->clear();
    on_userSerachBtn_clicked();
}
//用户管理--搜素用户
void MainWindow::on_userSerachBtn_clicked()
{
    QString searchName = ui->searchName->text();
    vector<User> users;
    if(searchName.isEmpty()){
        dataOper.getAllUsersInfo(users);
    }else{
        dataOper.getAllUsersInfo(users,searchName);
    }
    int dataSize = users.size();
    if(dataSize == 0){
        return;
    }else{
        clearTable(ui->userTable);
        ui->userTable->setRowCount(dataSize);
        for(int i = 0; i < dataSize; i++ ){
            for(int j = 0; j < 5; j++){
                if(j == 0){
                    ui->userTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                }else if (j == 1){
                    ui->userTable->setItem(i,j,new QTableWidgetItem(users[i].name));
                }else if(j == 2){
                    ui->userTable->setItem(i,j,new QTableWidgetItem(users[i].password));
                }else if(j == 3){
                    ui->userTable->setItem(i,j,new QTableWidgetItem(users[i].date));
                }else if(j == 4){
                    ui->userTable->setItem(i,j,new QTableWidgetItem(users[i].isUse ? "可用" : "不可用" ));
                }
            }
        }
    }


}
//用户管理--单击表格获取用户信息
void MainWindow::checkRowSlot(int row, int col)
{
//    UserheadText << "序号" << "用户名" << "密码" << "创建时间" << "状态";
//    QTableWidgetItem *aa = ui->userTable->item(row,col);
//    ui->userTable->openPersistentEditor(aa);

    if(col == 0){
        edit_userName = ui->userTable->item(row,col+1)->text();
        edit_pass = ui->userTable->item(row,col+2)->text();
    }else if(col == 1){
        edit_userName = ui->userTable->item(row,col)->text();
        edit_pass = ui->userTable->item(row,col+1)->text();
    }else if(col == 2){
        edit_userName = ui->userTable->item(row,col-1)->text();
        edit_pass = ui->userTable->item(row,col)->text();
    }else if(col == 3){
        edit_userName = ui->userTable->item(row,col-2)->text();
        edit_pass = ui->userTable->item(row,col-1)->text();
    }else if(col == 4){
        edit_userName = ui->userTable->item(row,col-3)->text();
        edit_pass = ui->userTable->item(row,col-2)->text();
    }
}
//用户管理--删除用户
void MainWindow::on_userDeleteBtn_clicked()
{
    if(edit_userName == nullptr){
        QMessageBox::information(this, "提示", "选择需要删除的用户","确定");
        return;
    }
    dataOper.deleteUserByName(edit_userName);
    dataOper.saveLog(STATION_HOUSE,"","删除用户",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    edit_userName = nullptr;
    edit_pass = nullptr;
    on_userClearBtn_clicked();
}
//用户管理--用户编辑回调事件
void MainWindow::userEdit_call_back(int userEditType)
{
    switch (userEditType) {
    case 1:
        dataOper.saveLog(STATION_HOUSE,"","新增用户",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        break;
    case 2:
        dataOper.saveLog(STATION_HOUSE,"","修改用户",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        break;
    }
    on_userClearBtn_clicked();
}




//系统报警--搜索按钮
void MainWindow::on_warningSearchBtn_clicked()
{
//    QString s_time = ui->warningStartime->text();//开始时间
//    QString e_time = ui->warningEndTime->text();//结束时间
//    QDateTime start = QDateTime::fromString(s_time, "yyyy-MM-dd");
//    QDateTime end = QDateTime::fromString(e_time, "yyyy-MM-dd");
//    uint stime = start.toTime_t();
//    uint etime = end.toTime_t();
//    int end_ = stime - etime;
//    if(end_ > 0 ){
//        QMessageBox::information(this, "提示", "结束日期小于开始日期，重新选择日期","确定");
//        return;
//    }
//    vector<Warning> warnings;
//    dataOper.getAllWarningInfo(warnings,2,s_time,e_time);
//    dataOper.saveLog(STATION_HOUSE,"","搜索报警数据",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
//    int dataSize = warnings.size();
//    if(dataSize == 0){
//        return;
//    }else{
//        clearTable(ui->warningTable);
//        ui->warningTable->setRowCount(dataSize);
//        for(int i = 0; i < dataSize; i++ ){
//            for(int j = 0; j < 5; j++){
//                if(j == 0){
//                    ui->warningTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
//                }else if (j == 1){
//                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].address));
//                }else if(j == 2){
//                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].deviceName));
//                }else if(j == 3){
//                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].info));
//                }else if(j == 4){
//                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].date));
//                }
//            }
//        }
//    }

    initDatatable_w();

}
//系统报警--清空按钮，即加载所有数据
void MainWindow::on_warningResetBtn_clicked()
{
    vector<Warning> warnings;
    dataOper.getAllWarningInfo(warnings,1);
    int dataSize = warnings.size();
    if(dataSize == 0){
        return;
    }else{
        clearTable(ui->warningTable);
        ui->warningTable->setRowCount(dataSize);
        for(int i = 0; i < dataSize; i++ ){
            for(int j = 0; j < 5; j++){
                if(j == 0){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                }else if (j == 1){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].address));
                }else if(j == 2){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].deviceName));
                }else if(j == 3){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].info));
                }else if(j == 4){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].date));
                }
            }
        }
    }
}

void MainWindow::initDatatable_w()
{
    currentPage_w = 1;
    GetTotalRecordCount_w();
    GetPageCount_w();
    UpdateStatus_w();

    RecordQuery_w(currentPage_w-1);

}
//总记录数
void MainWindow::GetTotalRecordCount_w()
{
    dataOper.GetTotalRecordCount("warning", totalRecrodCount_w);
}

//得到页数
void MainWindow::GetPageCount_w()
{
    PageRecordCount_w = ui->pageRecordCount_2->currentText().toInt();
    totalPage_w = (totalRecrodCount_w % PageRecordCount_w == 0) ? (totalRecrodCount_w / PageRecordCount_w) : (totalRecrodCount_w / PageRecordCount_w + 1);
}
//查询数据
void MainWindow::RecordQuery_w(int limitIndex)
{
    QString s_time = ui->warningStartime->text();//开始时间
    QString e_time = ui->warningEndTime->text();//结束时间
    QDateTime start = QDateTime::fromString(s_time, "yyyy-MM-dd");
    QDateTime end = QDateTime::fromString(e_time, "yyyy-MM-dd");
    uint stime = start.toTime_t();
    uint etime = end.toTime_t();
    int end_ = stime - etime;
    if(end_ > 0 ){
        QMessageBox::information(this, "提示", "结束日期小于开始日期，重新选择日期","确定");
        return;
    }
    vector<Warning> warnings;
    dataOper.RecordQuery_w(limitIndex,PageRecordCount_w, s_time, e_time,warnings);
    dataOper.saveLog(STATION_HOUSE,"","搜索报警数据",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    int dataSize = warnings.size();
    if(dataSize == 0){
        return;
    }else{
        clearTable(ui->warningTable);
        ui->warningTable->setRowCount(dataSize);
        for(int i = 0; i < dataSize; i++ ){
            for(int j = 0; j < 5; j++){
                if(j == 0){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                }else if (j == 1){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].address));
                }else if(j == 2){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].deviceName));
                }else if(j == 3){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].info));
                }else if(j == 4){
                    ui->warningTable->setItem(i,j,new QTableWidgetItem(warnings[i].date));
                }
            }
        }
    }
}

void MainWindow::UpdateStatus_w()
{
    ui->currentPage_2->setText(QString::number(currentPage_w));
    ui->totalPage_2->setText(QString::number(totalPage_w));
    if(currentPage_w == 1){
        ui->prevButton_2->setEnabled(false);
        ui->nextButton_2->setEnabled(true);
    }else if(currentPage_w == totalPage_w){
        ui->prevButton_2->setEnabled(true);
        ui->nextButton_2->setEnabled(false);
    }else{
        ui->prevButton_2->setEnabled(true);
        ui->nextButton_2->setEnabled(true);
    }
}

void MainWindow::on_fristButton_2_clicked()
{
    currentPage_w = 1;
    GetTotalRecordCount_w();
    GetPageCount_w();
    UpdateStatus_w();
    RecordQuery_w(currentPage_w-1);
}

void MainWindow::on_prevButton_2_clicked()
{
    GetTotalRecordCount_w();
    GetPageCount_w();
    int limitIndex = (currentPage_w - 2) * PageRecordCount_w;
    RecordQuery_w(limitIndex);
    currentPage_w -= 1;
    UpdateStatus_w();
}

void MainWindow::on_switchPageButton_2_clicked()
{
    GetTotalRecordCount_w();
    GetPageCount_w();
    //得到输入字符串
    QString szText = ui->switchPageLineEdit_2->text();
    //数字正则表达式
    QRegExp regExp("-?[0-9]*");
    //判断是否为数字
    if(!regExp.exactMatch(szText))
    {
        QMessageBox::information(this, tr("提示"), tr("请输入数字"));
        ui->switchPageLineEdit_2->clear();
        return;
    }
    //是否为空
    if(szText.isEmpty())
    {
        QMessageBox::information(this, tr("提示"), tr("请输入跳转页面"));
        ui->switchPageLineEdit_2->clear();
        return;
    }
    //得到页数
    int pageIndex = szText.toInt();
    //判断是否有指定页
    if(pageIndex > totalPage_w || pageIndex < 1)
    {
        QMessageBox::information(this, tr("提示"), tr("没有指定的页面，请重新输入"));
        ui->switchPageLineEdit_2->clear();
        return;
    }
    //得到查询起始行号
    int limitIndex = (pageIndex-1) * PageRecordCount_w;
    //记录查询
    RecordQuery_w(limitIndex);
    //设置当前页
    currentPage_w = pageIndex;
    //刷新状态
    UpdateStatus_w();
}

void MainWindow::on_nextButton_2_clicked()
{
    GetTotalRecordCount_w();
    GetPageCount_w();

    int limitIndex = currentPage_w * PageRecordCount_w;
    RecordQuery_w(limitIndex);
    currentPage_w += 1;
    UpdateStatus_w();
}

void MainWindow::on_lastButton_2_clicked()
{
    GetTotalRecordCount_w();
    GetPageCount_w();
    int limitIndex = (totalPage_w - 1) * PageRecordCount_w;
    currentPage_w = totalPage_w;
    UpdateStatus_w();
    RecordQuery_w(limitIndex);
}







//设备运行记录--搜索按钮
void MainWindow::on_logSearchBtn_clicked()
{
    QString s_time = ui->logStartime->text();//开始时间
    QString e_time = ui->logEndTime->text();//结束时间
    QDateTime start = QDateTime::fromString(s_time, "yyyy-MM-dd");
    QDateTime end = QDateTime::fromString(e_time, "yyyy-MM-dd");
    uint stime = start.toTime_t();
    uint etime = end.toTime_t();
    int end_ = stime - etime;
    if(end_ > 0 ){
        QMessageBox::information(this, "提示", "结束日期小于开始日期，重新选择日期","确定");
        return;
    }
    vector<Log> logs;
    dataOper.getAllLogInfo(logs,2,s_time,e_time);
    dataOper.saveLog(STATION_HOUSE,"","搜索设备运行数据",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    int dataSize = logs.size();
    if(dataSize == 0){
        return;
    }else{
        clearTable(ui->logTable);
        ui->logTable->setRowCount(dataSize);
        for(int i = 0; i < dataSize; i++ ){
            for(int j = 0; j < 6; j++){
                if(j == 0){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                }else if (j == 1){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].address));
                }else if(j == 2){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].device));
                }else if(j == 3){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].operType));
                }else if(j == 4){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].userName));
                }else if(j == 5){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].date));
                }
            }
        }
    }
}
//设备运行记录--清空按钮，即加载所有数据
void MainWindow::on_logResetBtn_clicked()
{
    vector<Log> logs;
    dataOper.getAllLogInfo(logs,1);
    int dataSize = logs.size();
    if(dataSize == 0){
        return;
    }else{
        clearTable(ui->logTable);
        ui->logTable->setRowCount(dataSize);
        for(int i = 0; i < dataSize; i++ ){
            for(int j = 0; j < 6; j++){
                if(j == 0){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                }else if (j == 1){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].address));
                }else if(j == 2){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].device));
                }else if(j == 3){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].operType));
                }else if(j == 4){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].userName));
                }else if(j == 5){
                    ui->logTable->setItem(i,j,new QTableWidgetItem(logs[i].date));
                }
            }
        }
    }
}




//设备控制---手动模式
void MainWindow::on_manualOper_clicked()
{
    appcore.setRunMode(ManualMode);
    dataOper.saveLog(STATION_HOUSE,"运行模式","手动模式设置",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    ui->dryerBtn1->setEnabled(true);
    ui->dryerBtn2->setEnabled(true);
    ui->dryerBtn3->setEnabled(true);
    ui->compressorBtn1->setEnabled(true);
    ui->compressorBtn2->setEnabled(true);
    ui->compressorBtn3->setEnabled(true);
    ui->runningMode->setText("手动");
}
//设备控制---自动模式
void MainWindow::on_autoOper_clicked()
{
    appcore.setRunMode(AutoMode);
    dataOper.saveLog(STATION_HOUSE,"运行模式","自动模式设置",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    ui->dryerBtn1->setEnabled(false);
    ui->dryerBtn2->setEnabled(false);
    ui->dryerBtn3->setEnabled(false);
    ui->compressorBtn1->setEnabled(false);
    ui->compressorBtn2->setEnabled(false);
    ui->compressorBtn3->setEnabled(false);
    ui->runningMode->setText("自动");
}

//设备控制---压力设置按钮
void MainWindow::on_pressureSetBtn_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    pressureSetDialog = new PressureSetDialog(userName,&appcore);
    connect(pressureSetDialog,&PressureSetDialog::closeCurrentDialog,this,&MainWindow::pressure_call_back);
    pressureSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    pressureSetDialog->setWindowModality(Qt::ApplicationModal);
    pressureSetDialog->show();
}
//设备控制---压力设置回调函数
void MainWindow::pressure_call_back(float maxPressure, float minPressure)
{
    dataOper.saveLog(STATION_HOUSE,"运行模式压力设置",QString("最大压力:%1,最小压力:%2").arg(maxPressure).arg(minPressure),
                     userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}
//设备控制---1#空压机设置
void MainWindow::on_compressorSetBtn1_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    compressorSetDialog = new CompressorSetDialog(userName,1,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---2#空压机设置
void MainWindow::on_compressorSetBtn2_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    compressorSetDialog = new CompressorSetDialog(userName,2,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---3#空压机设置
void MainWindow::on_compressorSetBtn3_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    compressorSetDialog = new CompressorSetDialog(userName,3,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---空压机设置完毕，回调函数
void MainWindow::compressor_call_back(int compressorNO, float uninstallPressure, float pressureDiff)
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    CompressorSet compressorSet;
    compressorSet.uninstallPressure = uninstallPressure;
    compressorSet.pressureDiff = pressureDiff;
    compressorSet.date = currentTime;
    compressorSet.flag = compressorNO;
    dataOper.saveCompressorSet(compressorSet);//保存设置参数
    dataOper.saveLog(STATION_HOUSE,QString("%1#空压机").arg(compressorNO),
                     QString("参数,加载压差:%1,卸载压力:%2").arg(pressureDiff).arg(uninstallPressure),userName,currentTime);
}
//设备控制---运行模式复位按钮
void MainWindow::on_runResetBtn_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    appcore.resetOperation();
    dataOper.saveLog(STATION_HOUSE,"运行模式","运行模式复位设置",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}
//设备控制---1#冷干机开关
void MainWindow::on_dryerBtn1_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(dryerSwitch1){
        appcore.setEquipmentSwitch(4,false);
        ui->dryerBtn1->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"1#冷干机","关闭1#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(4,true);
        ui->dryerBtn1->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"1#冷干机","打开1#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    dryerSwitch1 = !dryerSwitch1;
}
//设备控制---2#冷干机开关
void MainWindow::on_dryerBtn2_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(dryerSwitch2){
        appcore.setEquipmentSwitch(5,false);
        ui->dryerBtn2->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"2#冷干机","关闭2#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(5,true);
        ui->dryerBtn2->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"2#冷干机","打开2#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    dryerSwitch2 = !dryerSwitch2;
}
//设备控制---3#冷干机开关
void MainWindow::on_dryerBtn3_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(dryerSwitch3){
        appcore.setEquipmentSwitch(6,false);
        ui->dryerBtn3->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"3#冷干机","关闭3#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(6,true);
        ui->dryerBtn3->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"3#冷干机","打开3#冷干机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    dryerSwitch3 = !dryerSwitch3;
}
//设备控制---1#空压机开关
void MainWindow::on_compressorBtn1_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(compressorSwitch1){
        appcore.setEquipmentSwitch(1,false);
        ui->compressorBtn1->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"1#空压机","关闭1#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(1,true);
        ui->compressorBtn1->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"1#空压机","打开1#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    compressorSwitch1 = !compressorSwitch1;
}
//设备控制---2#空压机开关
void MainWindow::on_compressorBtn2_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(compressorSwitch2){
        appcore.setEquipmentSwitch(2,false);
        ui->compressorBtn2->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"2#空压机","关闭2#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(2,true);
        ui->compressorBtn2->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"2#空压机","打开2#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    compressorSwitch2 = !compressorSwitch2;
}
//设备控制---3#空压机开关
void MainWindow::on_compressorBtn3_clicked()
{
    if(PlcIsConnect()){
        return;
    }
    if(compressorSwitch3){
        appcore.setEquipmentSwitch(3,false);
        ui->compressorBtn3->setIcon(QIcon(":/images/images/btncheckoff2.png"));
        dataOper.saveLog(STATION_HOUSE,"3#空压机","关闭3#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }else{
        appcore.setEquipmentSwitch(3,true);
        ui->compressorBtn3->setIcon(QIcon(":/images/images/btncheckon2.png"));
        dataOper.saveLog(STATION_HOUSE,"3#空压机","打开3#空压机",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    compressorSwitch3 = !compressorSwitch3;
}


//定时读取3台空压机数据
void MainWindow::readCompressorTimer()
{
    QVector<quint16> buffer_;
    QVector<quint16> compressor1;
    QVector<quint16> compressor2;
    QVector<quint16> compressor3;
    QVector<quint16> dryer1;
    QVector<quint16> dryer2;
    QVector<quint16> dryer3;
    QVector<quint16> float2;
    QVector<quint16> warningInfo;
    QVector<quint16> equipmentStatus;

    appcore.readUint16(75,292,buffer_);

    //批量存储采集的数据
    if(isStoreData){//正在往数据库存储的时候，不要往临时变量里面存储
        saveBatchTimingData(buffer_);
    }
    for(int i = 0; i < buffer_.size(); i++){
        if(i>= 0 && i <= 5){
            warningInfo.push_back(buffer_.at(i));
        }
        if(i>= 10 && i <= 15){
            equipmentStatus.push_back(buffer_.at(i));
        }
        if(i>= 25 && i <= 59){
            compressor1.push_back(buffer_.at(i));
        }
        if(i>= 75 && i <= 109){
            compressor2.push_back(buffer_.at(i));
        }
        if(i>= 125 && i <= 159){
            compressor3.push_back(buffer_.at(i));
        }
        if(i>= 175 && i <= 191){
            dryer1.push_back(buffer_.at(i));
        }
        if(i>= 195 && i <= 198){
            float2.push_back(buffer_.at(i));
        }
        if(i>= 225 && i <= 241){
            dryer2.push_back(buffer_.at(i));
        }
        if(i>= 275 && i <= 291){
            dryer3.push_back(buffer_.at(i));
        }

    }

    switch (device_num) {
    case 2:
        dealCompressor1(compressor1,dryer1,float2);
        dealCompressor2(compressor2,dryer2);

        if(compressor1.size() == 35){
//            saveWarning(buffer_.at(46), 1,1);
//            saveWarning(buffer_.at(47), 1,2);
        }
        if(compressor2.size() == 35){
//            saveWarning(buffer_.at(96), 2,1);
//            saveWarning(buffer_.at(97), 2,2);
        }

        if(dryer1.size() == 17){
            if(buffer_.at(176)){
//                saveWarningDryer(1,1);
            }
            if(buffer_.at(179)){
//                saveWarningDryer(1,2);
            }
            if(buffer_.at(180)){
//                saveWarningDryer(1,3);
            }
            if(buffer_.at(183)){
//                saveWarningDryer(1,4);
            }
            if(buffer_.at(186)){
//                saveWarningDryer(1,5);
            }
            if(buffer_.at(187)){
//                saveWarningDryer(1,6);
            }
        }
        if(dryer2.size() == 17){
            if(buffer_.at(226)){
//                saveWarningDryer(2,1);
            }
            if(buffer_.at(229)){
//                saveWarningDryer(2,2);
            }
            if(buffer_.at(230)){
//                saveWarningDryer(2,3);
            }
            if(buffer_.at(233)){
//                saveWarningDryer(2,4);
            }
            if(buffer_.at(236)){
//                saveWarningDryer(2,5);
            }
            if(buffer_.at(237)){
//                saveWarningDryer(2,6);
            }
        }

        break;
    case 3:
        dealCompressor1(compressor1,dryer1,float2);
        dealCompressor2(compressor2,dryer2);
        dealCompressor3(compressor3,dryer3);

        if(compressor1.size() == 35){
//            saveWarning(buffer_.at(46), 1,1);
//            saveWarning(buffer_.at(47), 1,2);
        }
        if(compressor2.size() == 35){
//            saveWarning(buffer_.at(96), 2,1);
//            saveWarning(buffer_.at(97), 2,2);
        }
        if(compressor3.size() == 35){
//            saveWarning(buffer_.at(146), 3,1);
//            saveWarning(buffer_.at(147), 3,2);
        }
        if(dryer1.size() == 17){
            if(buffer_.at(176)){
//                saveWarningDryer(1,1);
            }
            if(buffer_.at(179)){
//                saveWarningDryer(1,2);
            }
            if(buffer_.at(180)){
//                saveWarningDryer(1,3);
            }
            if(buffer_.at(183)){
//                saveWarningDryer(1,4);
            }
            if(buffer_.at(186)){
//                saveWarningDryer(1,5);
            }
            if(buffer_.at(187)){
//                saveWarningDryer(1,6);
            }
        }
        if(dryer2.size() == 17){
            if(buffer_.at(226)){
//                saveWarningDryer(2,1);
            }
            if(buffer_.at(229)){
//                saveWarningDryer(2,2);
            }
            if(buffer_.at(230)){
//                saveWarningDryer(2,3);
            }
            if(buffer_.at(233)){
//                saveWarningDryer(2,4);
            }
            if(buffer_.at(236)){
//                saveWarningDryer(2,5);
            }
            if(buffer_.at(237)){
//                saveWarningDryer(2,6);
            }
        }
        if(dryer3.size() == 17){
            if(buffer_.at(276)){
//                saveWarningDryer(3,1);
            }
            if(buffer_.at(279)){
//                saveWarningDryer(3,2);
            }
            if(buffer_.at(280)){
//                saveWarningDryer(3,3);
            }
            if(buffer_.at(283)){
//                saveWarningDryer(3,4);
            }
            if(buffer_.at(286)){
//                saveWarningDryer(3,5);
            }
            if(buffer_.at(287)){
//                saveWarningDryer(3,6);
            }
        }
        break;
    }

    QString info = nullptr;
    if(warningInfo.size() != 0){
        if(warningInfo.at(0) == 1 && compressorIsShowWarning1){
            info += "1号空压机通讯失败!\n";
        }
        if(warningInfo.at(1) == 1 && compressorIsShowWarning2){
            info += "2号空压机通讯失败!\n";
        }
        if(warningInfo.at(2) == 1 && compressorIsShowWarning3){
            info += "3号空压机通讯失败!\n";
        }
        if(warningInfo.at(3) == 1 && dryerIsShowWarning1){
            info += "1号冷干机通讯失败!\n";
        }
        if(warningInfo.at(4) == 1 && dryerIsShowWarning2){
            info += "2号冷干机通讯失败!\n";
        }
        if(warningInfo.at(5) == 1 && dryerIsShowWarning3){
            info += "3号冷干机通讯失败!\n";
        }
        if(isShowWarning && info != nullptr ){
            isShowWarning = false;
            warningHintDialog = new WarningHintDialog(device_num,!compressorIsShowWarning1,!compressorIsShowWarning2,!compressorIsShowWarning3,
                                                      !dryerIsShowWarning1,!dryerIsShowWarning2,!dryerIsShowWarning3,info);
            connect(warningHintDialog,&WarningHintDialog::closeCurrentDialog,this,&MainWindow::warningHint_call_back);
            warningHintDialog->setAttribute(Qt::WA_DeleteOnClose);
            warningHintDialog->show();
        }
     }

    if(isGetEquipmentStatus){
        isGetEquipmentStatus = false;
        ui->stopCompressor1->setChecked(!equipmentStatus.at(0));
        ui->stopCompressor2->setChecked(!equipmentStatus.at(1));
        ui->stopCompressor3->setChecked(!equipmentStatus.at(2));
        ui->stopDryer1->setChecked(!equipmentStatus.at(3));
        ui->stopDryer2->setChecked(!equipmentStatus.at(4));
        ui->stopDryer3->setChecked(!equipmentStatus.at(5));

    }

    if(storageInterval == STORE_TIME){//储存空压机、冷干机数据
        storageInterval = 0;
        QString storageTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        //存储读取数据导数据
        if(compressor1.size() == 35){
//            dataOper.saveReadCompressor1(compressor1,storageTime);
        }
        if(compressor2.size() == 35){
//            dataOper.saveReadCompressor2(compressor2,storageTime);
        }
        if(compressor3.size() == 35){
//            dataOper.saveReadCompressor3(compressor3,storageTime);
        }
        if(dryer1.size() == 17){
//            dataOper.saveReadDryer1(dryer1,storageTime);
        }
        if(dryer2.size() == 17){
//            dataOper.saveReadDryer2(dryer2,storageTime);
        }
        if(dryer3.size() == 17){
//            dataOper.saveReadDryer3(dryer3,storageTime);
        }
    }
    storageInterval += READ_TIME;

    saveData(compressors1,compressors2,compressors3,dryers1,dryers2,dryers3,warnings);
}
//定时读取设备弹窗警告信息
void MainWindow::readWarningHintTimer()
{
    QVector<quint16> warningInfo;
    QString info = nullptr;
    appcore.readWarningHint(warningInfo);
    if(warningInfo.size() == 0){
        return;
    }

    if(warningInfo.at(0) == 1 && compressorIsShowWarning1){
        info += "1号空压机通讯失败!\n";
    }
    if(warningInfo.at(1) == 1 && compressorIsShowWarning2){
        info += "2号空压机通讯失败!\n";
    }
    if(warningInfo.at(2) == 1 && compressorIsShowWarning3){
        info += "3号空压机通讯失败!\n";
    }
    if(warningInfo.at(3) == 1 && dryerIsShowWarning1){
        info += "1号冷干机通讯失败!\n";
    }
    if(warningInfo.at(4) == 1 && dryerIsShowWarning2){
        info += "2号冷干机通讯失败!\n";
    }
    if(warningInfo.at(5) == 1 && dryerIsShowWarning3){
        info += "3号冷干机通讯失败!\n";
    }
    if(isShowWarning && info != nullptr ){
        isShowWarning = false;
        warningHintDialog = new WarningHintDialog(device_num,!compressorIsShowWarning1,!compressorIsShowWarning2,!compressorIsShowWarning3,
                                                  !dryerIsShowWarning1,!dryerIsShowWarning2,!dryerIsShowWarning3,info);
        connect(warningHintDialog,&WarningHintDialog::closeCurrentDialog,this,&MainWindow::warningHint_call_back);
        warningHintDialog->setAttribute(Qt::WA_DeleteOnClose);
        warningHintDialog->show();
    }
}
//弹出警告信息的回调函数
void MainWindow::warningHint_call_back(bool a, bool b, bool c, bool d, bool e, bool f)
{
    isShowWarning = true;
    switch (device_num) {
    case 2:
        compressorIsShowWarning1 = a ? false : true;
        compressorIsShowWarning2 = b ? false : true;
        dryerIsShowWarning1 = d ? false : true;
        dryerIsShowWarning2 = e ? false : true;

        ui->stopCompressor1->setChecked(compressorIsShowWarning1);
        ui->stopCompressor2->setChecked(compressorIsShowWarning2);
        ui->stopDryer1->setChecked(dryerIsShowWarning1);
        ui->stopDryer2->setChecked(dryerIsShowWarning2);
        break;
    case 3:
        compressorIsShowWarning1 = a ? false : true;
        compressorIsShowWarning2 = b ? false : true;
        compressorIsShowWarning3 = c ? false : true;
        dryerIsShowWarning1 = d ? false : true;
        dryerIsShowWarning2 = e ? false : true;
        dryerIsShowWarning3 = f ? false : true;

        ui->stopCompressor1->setChecked(compressorIsShowWarning1);
        ui->stopCompressor2->setChecked(compressorIsShowWarning2);
        ui->stopCompressor3->setChecked(compressorIsShowWarning3);
        ui->stopDryer1->setChecked(dryerIsShowWarning1);
        ui->stopDryer2->setChecked(dryerIsShowWarning2);
        ui->stopDryer3->setChecked(dryerIsShowWarning3);
        break;
    }

}
//显示1#空压机读取的数据
void MainWindow::dealCompressor1(QVector<quint16> compressor, QVector<quint16> dryer,QVector<quint16> float2)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
//        saveWarning(runMode1, 1,1);
//        saveWarning(runMode2, 1,2);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState1->setText("运行中");//运行状态
            ui->runningState1->setStyleSheet("color:#336600;");
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState1->setText("停止");//运行状态
                ui->runningState1->setStyleSheet("color:#990000;");
            }
        }
        if(aa.at(10)){
            ui->runningState1->setText("加载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState1->setText("满载");//运行状态
        }
        ui->powerStatus1->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->powerStatus1->setStyleSheet(aa.at(8) ? "color:#336600;" : "color:#990000;");
        ui->P1_1->setText(QString::number(P1/145.0,'f',2));//P1
        ui->ventingPressure1->setText(QString::number(P2/145.0,'f',2));//排气压力
        ui->ventingT1->setText(QString::number(T1));//排气温度
        ui->runningT1->setText(QString::number(runTimeL));//运行时间
        ui->hostA1->setText(QString::number(hostCurrent));//主机电流
        ui->sysT1->setText(QString::number(T2));//系统温度
        ui->uploadT1->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState1->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->compressorRunState1->setStyleSheet(aa.at(9) ? "color:#336600;" : "color:#990000;");
        ui->pressureDiff1->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->uninstallPressure1->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
        //设备控制
        ui->compressorBtn1->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch1 = aa.at(9) ? true : false;
        ui->controlPressureDiff1->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->controlUninstallPressure1->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
    }
    if(dryer.size() == 17){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示 1
        quint16 phaseOrderFault = dryer.at(4);//相序故障 2
        quint16 overloadSave = dryer.at(5);//故障保护 3
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障 4
        quint16 faultWarn = dryer.at(11);//故障报警 5
        quint16 faultStop = dryer.at(12);//故障停机 6
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）
//        if(faultHint){
//            saveWarningDryer(1,1);
//        }
//        if(phaseOrderFault){
//            saveWarningDryer(1,2);
//        }
//        if(overloadSave){
//            saveWarningDryer(1,3);
//        }
//        if(dewPointProbeFault){
//            saveWarningDryer(1,4);
//        }
//        if(faultWarn){
//            saveWarningDryer(1,5);
//        }
//        if(faultStop){
//            saveWarningDryer(1,6);
//        }
        //主界面
        ui->valveState1->setText(runHint ? "运行中" : "停机状态");//运行状态
        ui->valveState1->setStyleSheet(runHint ? "color:#336600;" : "color:#990000;");
        ui->faultHint1->setText(faultHint ? "故障" : "无故障");//故障提示
        ui->faultHint1->setStyleSheet(faultHint ? "color:#990000;" : "color:#336600;");
        ui->refrigerantH1->setText(QString::number(sysHighVoltage));//系统高压
        ui->refrigerantL1->setText(QString::number(sysLowVoltage));//系统低压
        ui->dewPointT1->setText(QString::number(dewPointT));//露点温度
        ui->runTimeH1->setText(QString::number(runTimeH));//运行计时（时）
        ui->runTimeM1->setText(QString::number(runTimeM));//运行计时（分）
        //设备状态
        ui->dryerRunState1->setText(runHint ? "开机" : "关机");//是否开机
        //设备控制
        ui->dryerBtn1->setIcon(runHint ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        dryerSwitch1 = runHint ? true : false;
    }

    if(float2.size() == 4){

        float instantVal = 0;//瞬时流量
        float accumulativeVal = 0;//累计流量
        int_to_float(float2.at(0),float2.at(1), instantVal);
        int_to_float(float2.at(2),float2.at(3), accumulativeVal);

        ui->F_labelV->setText(QString::number(instantVal,'f',2));
        ui->F_labelV2->setText(QString::number(accumulativeVal,'f',2));
    }
}
//显示2#空压机读取的数据
void MainWindow::dealCompressor2(QVector<quint16> compressor, QVector<quint16> dryer)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
//        saveWarning(runMode1, 2,1);
//        saveWarning(runMode2, 2,2);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState2->setText("运行中");//运行状态
            ui->runningState2->setStyleSheet("color:#336600;");
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState2->setText("停止");//运行状态
                ui->runningState2->setStyleSheet("color:#990000;");
            }
        }
        if(aa.at(10)){
            ui->runningState2->setText("加载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState2->setText("满载");//运行状态
        }
        ui->powerStatus2->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->powerStatus2->setStyleSheet(aa.at(8) ? "color:#336600;" : "color:#990000;");
        ui->P1_2->setText(QString::number(P1/145.0,'f',2));//P1
        ui->ventingPressure2->setText(QString::number(P2/145.0,'f',2));//排气压力
        ui->ventingT2->setText(QString::number(T1));//排气温度
        ui->runningT2->setText(QString::number(runTimeL));//运行时间
        ui->hostA2->setText(QString::number(hostCurrent));//主机电流
        ui->sysT2->setText(QString::number(T2));//系统温度
        ui->uploadT2->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState2->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->compressorRunState2->setStyleSheet(aa.at(9) ? "color:#336600;" : "color:#990000;");
        ui->pressureDiff2->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->uninstallPressure2->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
        //设备控制
        ui->compressorBtn2->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch2 = aa.at(9) ? true : false;
        ui->controlPressureDiff2->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->controlUninstallPressure2->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
    }
    if(dryer.size() == 17){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示
        quint16 phaseOrderFault = dryer.at(4);//相序故障
        quint16 overloadSave = dryer.at(5);//故障保护
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障
        quint16 faultWarn = dryer.at(11);//故障报警
        quint16 faultStop = dryer.at(12);//故障停机
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）
//        if(faultHint){
//            saveWarningDryer(2,1);
//        }
//        if(phaseOrderFault){
//            saveWarningDryer(2,2);
//        }
//        if(overloadSave){
//            saveWarningDryer(2,3);
//        }
//        if(dewPointProbeFault){
//            saveWarningDryer(2,4);
//        }
//        if(faultWarn){
//            saveWarningDryer(2,5);
//        }
//        if(faultStop){
//            saveWarningDryer(2,6);
//        }

        //主界面
        ui->valveState2->setText(runHint ? "运行中" : "停机状态");//阀门状态
        ui->valveState2->setStyleSheet(runHint ? "color:#336600;" : "color:#990000;");
        ui->faultHint2->setText(faultHint ? "故障" : "无故障");//故障提示
        ui->faultHint2->setStyleSheet(faultHint ? "color:#990000;" : "color:#336600;");
        ui->refrigerantH2->setText(QString::number(sysHighVoltage));//系统高压
        ui->refrigerantL2->setText(QString::number(sysLowVoltage));//系统低压
        ui->dewPointT2->setText(QString::number(dewPointT));//露点温度
        ui->runTimeH2->setText(QString::number(runTimeH));//运行计时（时）
        ui->runTimeM2->setText(QString::number(runTimeM));//运行计时（分）
        //设备状态
        ui->dryerRunState2->setText(runHint ? "开机" : "关机");//是否开机
        //设备控制
        ui->dryerBtn2->setIcon(runHint ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        dryerSwitch2 = runHint ? true : false;

    }

}
//显示3#空压机读取的数据
void MainWindow::dealCompressor3(QVector<quint16> compressor, QVector<quint16> dryer)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
//        saveWarning(runMode1, 3,1);
//        saveWarning(runMode2, 3,2);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState3->setText("运行中");//运行状态
            ui->runningState3->setStyleSheet("color:#336600;");
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState3->setText("停止");//运行状态
                ui->runningState3->setStyleSheet("color:#990000;");
            }
        }
        if(aa.at(10)){
            ui->runningState3->setText("加载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState3->setText("满载");//运行状态
        }
        ui->powerStatus3->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->powerStatus3->setStyleSheet(aa.at(8) ? "color:#336600;" : "color:#990000;");
        ui->P1_3->setText(QString::number(P1/145.0,'f',2));//P1
        ui->ventingPressure3->setText(QString::number(P2/145.0,'f',2));//排气压力
        ui->ventingT3->setText(QString::number(T1));//排气温度
        ui->runningT3->setText(QString::number(runTimeL));//运行时间
        ui->hostA3->setText(QString::number(hostCurrent));//主机电流
        ui->sysT3->setText(QString::number(T2));//系统温度
        ui->uploadT3->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState3->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->compressorRunState3->setStyleSheet(aa.at(9) ? "color:#336600;" : "color:#990000;");
        ui->pressureDiff3->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->uninstallPressure3->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
        //设备控制
        ui->compressorBtn3->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch3 = aa.at(9) ? true : false;
        ui->controlPressureDiff3->setText(QString::number(pressureDiff/145.0,'f',2));//加载压差
        ui->controlUninstallPressure3->setText(QString::number(uninstallPressure/145.0,'f',2));//卸载压力
    }
    if(dryer.size() == 17){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示
        quint16 phaseOrderFault = dryer.at(4);//相序故障
        quint16 overloadSave = dryer.at(5);//故障保护
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障
        quint16 faultWarn = dryer.at(11);//故障报警
        quint16 faultStop = dryer.at(12);//故障停机
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）
//        if(faultHint){
//            saveWarningDryer(3,1);
//        }
//        if(phaseOrderFault){
//            saveWarningDryer(3,2);
//        }
//        if(overloadSave){
//            saveWarningDryer(3,3);
//        }
//        if(dewPointProbeFault){
//            saveWarningDryer(3,4);
//        }
//        if(faultWarn){
//            saveWarningDryer(3,5);
//        }
//        if(faultStop){
//            saveWarningDryer(3,6);
//        }

        //主界面
        ui->valveState3->setText(runHint ? "运行中" : "停机状态");//阀门状态
        ui->valveState3->setStyleSheet(runHint ? "color:#336600;" : "color:#990000;");
        ui->faultHint3->setText(faultHint ? "故障" : "无故障");//故障提示
        ui->faultHint3->setStyleSheet(faultHint ? "color:#990000;" : "color:#336600;");
        ui->refrigerantH3->setText(QString::number(sysHighVoltage));//系统高压
        ui->refrigerantL3->setText(QString::number(sysLowVoltage));//系统低压
        ui->dewPointT3->setText(QString::number(dewPointT));//露点温度
        ui->runTimeH3->setText(QString::number(runTimeH));//运行计时（时）
        ui->runTimeM3->setText(QString::number(runTimeM));//运行计时（分）
        //设备状态
        ui->dryerRunState3->setText(runHint ? "开机" : "关机");//是否开机
        //设备控制
        ui->dryerBtn3->setIcon(runHint ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        dryerSwitch3 = runHint ? true : false;
    }
}
//存储空压机报警数据
void MainWindow::saveWarning(quint16 warningInfo, int compressorNo,int runState)
{
    Warning warning;
    QVector<bool> aa = dec2BinTrans(warningInfo);
    warning.address = STATION_HOUSE;
    warning.deviceName = QString("%1#空压机").arg(compressorNo);
    warning.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if(aa.at(0)){
        switch (runState) {
        case 1:
            warning.info = "紧急停机";
            break;
        case 2:
            warning.info = "空滤堵塞";
            break;
        case 3:
            warning.info = "高压侧报警";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(1)){
        switch (runState) {
        case 1:
            warning.info = "主机过载";
            break;
        case 2:
            warning.info = "油滤堵塞";
            break;
        case 3:
            warning.info = "316 报警停机";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(2)){
        switch (runState) {
        case 1:
            warning.info = "风机过载";
            break;
        case 2:
            warning.info = "△ P1报警";
            break;
        case 3:
            warning.info = "";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(3)){
        switch (runState) {
        case 1:
            warning.info = "电源异常";
            break;
        case 2:
            warning.info = "△ P2报警";
            break;
        case 3:
            warning.info = "";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(4)){
        switch (runState) {
        case 1:
            warning.info = "水压异常";
            break;
        case 2:
            warning.info = "T5 报警";
            break;
        case 3:
            warning.info = "T1 LOW";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(5)){
        switch (runState) {
        case 1:
            warning.info = "P1 停机";
            break;
        case 2:
            warning.info = "P1 报警";
            break;
        case 3:
            warning.info = "温度传感器故障";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(6)){
        switch (runState) {
        case 1:
            warning.info = "Ic 停机";
            break;
        case 2:
            warning.info = "P2 报警";
            break;
        case 3:
            warning.info = "压力传感器故障";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(7)){
        switch (runState) {
        case 1:
            warning.info = "T1 停机";
            break;
        case 2:
            warning.info = "T1 报警";
            break;
        case 3:
            warning.info = "210 报警停机";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(8)){
        switch (runState) {
        case 1:
            warning.info = "T2 停机";
            break;
        case 2:
            warning.info = "T2 报警";
            break;
        case 3:
            warning.info = "电源";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(9)){
        switch (runState) {
        case 1:
            warning.info = "T4 停机";
            break;
        case 2:
            warning.info = "T4 报警";
            break;
        case 3:
            warning.info = "运行";
            break;
        }
        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(10)){
        switch (runState) {
        case 1:
            warning.info = "环境温度过高停机";
            break;
        case 2:
            warning.info = "△ T3报警";
            break;
        case 3:
            warning.info = "加载";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(11)){
        switch (runState) {
        case 1:
            warning.info = "电压异常";
            break;
        case 2:
            warning.info = "△T2 报警";
            break;
        case 3:
            warning.info = "满载";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(12)){
        switch (runState) {
        case 1:
            warning.info = "△ P3 停机";
            break;
        case 2:
            warning.info = "△ T1 报警";
            break;
        case 3:
            warning.info = "轻故障";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(13)){
        switch (runState) {
        case 1:
            warning.info = "P1 LOW";
            break;
        case 2:
            warning.info = "环境温度报警";
            break;
        case 3:
            warning.info = "重故障";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(14)){
        switch (runState) {
        case 1:
            warning.info = "P1 HIGH";
            break;
        case 2:
            warning.info = "SRC报警(208)";
            break;
        case 3:
            warning.info = "";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
    if(aa.at(15)){
        switch (runState) {
        case 1:
            warning.info = "P3 LOW";
            break;
        case 2:
            warning.info = "SPR 报警(209)";
            break;
        case 3:
            warning.info = "";
            break;
        }
//        dataOper.saveWarningInfo(warning);
        warnings.push_back(warning);
    }
}
//存储冷干机报警信息
void MainWindow::saveWarningDryer(int dryerNo, int runState)
{
    Warning warning;
    warning.address = STATION_HOUSE;
    warning.deviceName = QString("%1#冷干机").arg(dryerNo);
    warning.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    switch (runState) {
    case 1:
        warning.info = "故障提示";
        break;
    case 2:
        warning.info = "相序故障";
        break;
    case 3:
        warning.info = "故障保护";
        break;
    case 4:
        warning.info = "露点探头故障";
        break;
    case 5:
        warning.info = "故障报警";
        break;
    case 6:
        warning.info = "故障停机";
        break;
    }
//    dataOper.saveWarningInfo(warning);
    warnings.push_back(warning);
}


//初始读取设备是否启用
void MainWindow::getInitEquipmentStatus()
{
    QVector<quint16> equipmentStatus;
    appcore.readEquipmentStatus(equipmentStatus);

    ui->stopCompressor1->setChecked(!equipmentStatus.at(0));
    ui->stopCompressor2->setChecked(!equipmentStatus.at(1));
    ui->stopCompressor3->setChecked(!equipmentStatus.at(2));
    ui->stopDryer1->setChecked(!equipmentStatus.at(3));
    ui->stopDryer2->setChecked(!equipmentStatus.at(4));
    ui->stopDryer3->setChecked(!equipmentStatus.at(5));
}



//鼠标移动显示曲线数据
void MainWindow::slotPointHoverd(const QPointF &point, bool state)
{
    QString time = QDateTime::fromMSecsSinceEpoch(point.x()).toString("yyyy-MM-dd hh:mm:ss");
    if (state){
        m_valueLabel->setText( "(" +QString::number(point.y(),'f',2) + "," + time + ")");
        QPoint curPos = mapFromGlobal(QCursor::pos());
        m_valueLabel->move(curPos.x() - m_valueLabel->width() / 2, curPos.y() - m_valueLabel->height() * 1.5);//移动数值
        m_valueLabel->show();//显示出来
    }else{
        m_valueLabel->hide();//进行隐藏
    }
}
void MainWindow::slotPointHoverd2(const QPointF &point, bool state)
{
    QString time = QDateTime::fromMSecsSinceEpoch(point.x()).toString("yyyy-MM-dd hh:mm:ss");
    if (state){
        m_valueLabel2->setText( "(" +QString::number(point.y(),'f',2) + "," + time + ")");
        QPoint curPos = mapFromGlobal(QCursor::pos());
        m_valueLabel2->move(curPos.x() - m_valueLabel2->width() / 2, curPos.y() - m_valueLabel2->height() * 1.5);//移动数值
        m_valueLabel2->show();//显示出来
    }else{
        m_valueLabel2->hide();//进行隐藏
    }
}

//清空表格
void MainWindow::clearTable(QTableWidget *table)
{
    for(int i = 0 ; i < table->rowCount(); i++){
        for(int j = 0 ; j < table->columnCount(); j++){
            if(table->item(i,j)== NULL || (table->item(i,j) && table->item(i,j)->text()=="")){
                continue;
            }else{
                table->item(i,j)->setText("");
            }
        }
    }
}
//显示实时时间更新
void MainWindow::timerUpdate(void)
{
    //图像采集的当前时间toString();
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->currenTime->setText(currentDateTime);//设置测量日期是当前日期
}

//返回当前时间之间的时间或者之后的时间，负数 之前的天数  整数 之后的天数
QString MainWindow::getDefineTimeByDay(int day)
{
//    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
//    QString str = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
//    QString BeforeDaystr = time.addDays(day).toString("yyyy-MM-dd hh:mm:ss");//获取前一天时间

//    qDebug() << BeforeDaystr;
//    return BeforeDaystr;
    return QDateTime::currentDateTime().addDays(day).toString("yyyy-MM-dd");
}




//数据查看单选按钮
void MainWindow::radioButton_pressed(bool checked)
{
    if(!checked){
        return;
    }
    QRadioButton *b = (QRadioButton *)sender();
    QString text = b->text();
    QStringList logHeadText;
    ui->dataTable->clear();

    if (text == "1#空压机") {
        tableName = "Compressor1";
        logHeadText << "序号" << "排气压力(Mpa)" << "排气温度(℃)" << "系统温度(℃)" << "主机电流(A)" << "加载压力(Mpa)" << "卸载压差(Mpa)" << "采集时间";
    } else if (text == "2#空压机") {
        tableName = "Compressor2";
        logHeadText << "序号" << "排气压力(Mpa)" << "排气温度(℃)" << "系统温度(℃)" << "主机电流(A)" << "加载压力(Mpa)" << "卸载压差(Mpa)" << "采集时间";
    } else if (text == "3#空压机") {
        tableName = "Compressor3";
        logHeadText << "序号" << "排气压力(Mpa)" << "排气温度(℃)" << "系统温度(℃)" << "主机电流(A)" << "加载压力(Mpa)" << "卸载压差(Mpa)" << "采集时间";
    } else if (text == "1#冷干机") {
        tableName = "dryer1";
        logHeadText << "序号" << "露点温度(℃)" << "采集时间";
    } else if (text == "2#冷干机") {
        tableName = "dryer2";
        logHeadText << "序号" << "露点温度(℃)" << "采集时间";
    } else if (text == "3#冷干机") {
        tableName = "dryer3";
        logHeadText << "序号" << "露点温度(℃)" << "采集时间";
    }

    ui->dataTable->setColumnCount(logHeadText.size());

    ui->dataTable->verticalHeader()->setDefaultSectionSize(25);

    ui->dataTable->setHorizontalHeaderLabels(logHeadText);
    ui->dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->dataTable->setAlternatingRowColors(true);
    ui->dataTable->verticalHeader()->setVisible(false);
    ui->dataTable->horizontalHeader()->setStretchLastSection(true);
    initDatatable();
}

void MainWindow::initDatatable()
{
    currentPage = 1;
    GetTotalRecordCount();
    GetPageCount();
    UpdateStatus();

    RecordQuery(currentPage-1);

}
//总记录数
void MainWindow::GetTotalRecordCount()
{
    dataOper.GetTotalRecordCount(tableName, totalRecrodCount);
}

//得到页数
void MainWindow::GetPageCount()
{
    PageRecordCount = ui->pageRecordCount->currentText().toInt();
    totalPage = (totalRecrodCount % PageRecordCount == 0) ? (totalRecrodCount / PageRecordCount) : (totalRecrodCount / PageRecordCount + 1);
}
//查询数据
void MainWindow::RecordQuery(int limitIndex)
{
    vector<Dryer> dryers;
    vector<Compressor> compressors;
    dataOper.RecordQuery(limitIndex,PageRecordCount,tableName,compressors,dryers);

    int dryersDataSize = dryers.size();
    int compressorsDataSize = compressors.size();
    int headerCount = ui->dataTable->horizontalHeader()->count();
    if(dryersDataSize == 0 && compressorsDataSize == 0){
        return;
    }else{
        clearTable(ui->dataTable);
        if(dryersDataSize != 0){
            ui->dataTable->setRowCount(dryersDataSize);
            for(int i = 0; i < dryersDataSize; i++ ){
                for(int j = 0; j < headerCount; j++){
                    if(j == 0){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                    }else if (j == 1){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(dryers[i].dewPointT)));
                    }else if(j == 2){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(dryers[i].date.toString("yyyy-MM-dd hh:mm:ss")));
                    }
                }
            }
        }
        if(compressorsDataSize != 0){
            ui->dataTable->setRowCount(compressorsDataSize);
            for(int i = 0; i < compressorsDataSize; i++ ){
                for(int j = 0; j < headerCount; j++){
                    if(j == 0){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(i+1)));
                    }else if (j == 1){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].P2)));
                    }else if(j == 2){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].T1)));
                    }else if(j == 3){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].T2)));
                    }else if(j == 4){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].hostCurrent)));
                    }else if(j == 5){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].pressureDiff)));
                    }else if(j == 6){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(QString::number(compressors[i].uninstallPressure)));
                    }else if(j == 7){
                        ui->dataTable->setItem(i,j,new QTableWidgetItem(compressors[i].date.toString("yyyy-MM-dd hh:mm:ss")));
                    }
                }
            }
        }
    }
}

void MainWindow::on_fristButton_clicked()
{
    if(!radioButtonIsChecked()){
        return;
    }
    currentPage = 1;
    GetTotalRecordCount();
    GetPageCount();
    UpdateStatus();
    RecordQuery(currentPage-1);
}

void MainWindow::on_prevButton_clicked()
{
    if(!radioButtonIsChecked()){
        return;
    }
    GetTotalRecordCount();
    GetPageCount();
    int limitIndex = (currentPage - 2) * PageRecordCount;
    RecordQuery(limitIndex);
    currentPage -= 1;
    UpdateStatus();
}

void MainWindow::on_switchPageButton_clicked()
{
    if(!radioButtonIsChecked()){
        return;
    }
    GetTotalRecordCount();
    GetPageCount();
    //得到输入字符串
    QString szText = ui->switchPageLineEdit->text();
    //数字正则表达式
    QRegExp regExp("-?[0-9]*");
    //判断是否为数字
    if(!regExp.exactMatch(szText))
    {
        QMessageBox::information(this, tr("提示"), tr("请输入数字"));
        ui->switchPageLineEdit->clear();
        return;
    }
    //是否为空
    if(szText.isEmpty())
    {
        QMessageBox::information(this, tr("提示"), tr("请输入跳转页面"));
        ui->switchPageLineEdit->clear();
        return;
    }
    //得到页数
    int pageIndex = szText.toInt();
    //判断是否有指定页
    if(pageIndex > totalPage || pageIndex < 1)
    {
        QMessageBox::information(this, tr("提示"), tr("没有指定的页面，请重新输入"));
        ui->switchPageLineEdit->clear();
        return;
    }
    //得到查询起始行号
    int limitIndex = (pageIndex-1) * PageRecordCount;
    //记录查询
    RecordQuery(limitIndex);
    //设置当前页
    currentPage = pageIndex;
    //刷新状态
    UpdateStatus();
}

void MainWindow::on_nextButton_clicked()
{
    if(!radioButtonIsChecked()){
        return;
    }
    GetTotalRecordCount();
    GetPageCount();

    int limitIndex = currentPage * PageRecordCount;
    RecordQuery(limitIndex);
    currentPage += 1;
    UpdateStatus();
}

void MainWindow::on_lastButton_clicked()
{
    if(!radioButtonIsChecked()){
        return;
    }
    GetTotalRecordCount();
    GetPageCount();
    int limitIndex = (totalPage-1) * PageRecordCount;
    currentPage = totalPage;
    UpdateStatus();
    RecordQuery(limitIndex);
}

void MainWindow::UpdateStatus()
{
    ui->currentPage->setText(QString::number(currentPage));
    ui->totalPage->setText(QString::number(totalPage));
    if(currentPage == 1){
        ui->prevButton->setEnabled(false);
        ui->nextButton->setEnabled(true);
    }else if(currentPage == totalPage){
        ui->prevButton->setEnabled(true);
        ui->nextButton->setEnabled(false);
    }else{
        ui->prevButton->setEnabled(true);
        ui->nextButton->setEnabled(true);
    }
}

bool MainWindow::radioButtonIsChecked()
{
    int a = ui->radioButton_1->isChecked();
    int b = ui->radioButton_2->isChecked();
    int c = ui->radioButton_3->isChecked();
    int d = ui->radioButton_4->isChecked();
    int e = ui->radioButton_5->isChecked();
    int f = ui->radioButton_6->isChecked();
    if(a || b || c || d || e || f){
        return true;
    }else{
        QMessageBox::information(this, "提示", "选择需要查看数据的设备！","确定");
        return false;
    }
}








//解析无符号的整型数据
QVector<bool> MainWindow::dec2BinTrans(unsigned int data)
{
//    QVector<bool> bin;
//    for(int i = 0; i < 16 && data != 0; i++)
//    {
//        bin.push_back(data%2);
//        data/=2;
//    }
//    QVector<bool> bintemp(16,0);//初始化16个元素，每个初始化为0

//    for(int i = 0; i < 8 && i < bin.size(); i++)
//    {
//        bintemp[i+8] = bin[i];
//    }
//    for(int i = 8; i < 16 && i < bin.size(); i++)
//    {
//        bintemp[i-8] = bin[i];
//    }

//    return bintemp;

    QVector<bool> bin(16,0);
    for(int i = 0; i < 16 && data != 0; i++)
    {
        bin[i] = data%2;
        data/=2;
    }


//    qDebug() << bin;
    return bin;
}

//二进制转10进制
void MainWindow::binToDec(unsigned int data, int &h_val, int &l_val)
{

    QVector<int> bin(16,0);
    for(int i = 0; i < 16 && data != 0; i++)
    {
        bin[15-i] = data%2;
        data/=2;
    }
//    qDebug() << "QVector<int> bin:" << bin.toList();

    QString aa = nullptr;
    QString bb = nullptr;

    for(int i = 0; i < bin.size(); ++i){
        if(i < 8){
            aa += QString::number(bin.at(i));
        }else{
            bb += QString::number(bin.at(i));
        }
    }

//    qDebug() << "aa:" << aa;
//    qDebug() << "bb:" << bb;

    int e = 1;
    for(int i = aa.length()-1 ; i >=0; i--){
        h_val += aa.mid(i,1).toInt()*e;
        e *= 2;
    }
    int k = 1;
    for(int i = bb.length()-1 ; i >=0; i--){
        l_val += bb.mid(i,1).toInt()*k;
        k *= 2;
    }

}

bool MainWindow::PlcIsConnect()
{
    if(!PLCIsConnected){
        QMessageBox::information(this, "提示", "PLC未连接！","确定");
        return true;
    }else{
        return false;
    }
}

void MainWindow::lockUiOperation()
{
    ui->stopCompressor1->setEnabled(false);
    ui->stopCompressor2->setEnabled(false);
    ui->stopCompressor3->setEnabled(false);
    ui->stopDryer1->setEnabled(false);
    ui->stopDryer2->setEnabled(false);
    ui->stopDryer3->setEnabled(false);

}
void MainWindow::unlockUiOperation()
{
    ui->stopCompressor1->setEnabled(true);
    ui->stopCompressor2->setEnabled(true);
    ui->stopCompressor3->setEnabled(true);
    ui->stopDryer1->setEnabled(true);
    ui->stopDryer2->setEnabled(true);
    ui->stopDryer3->setEnabled(true);
}

void MainWindow::int_to_float(quint16 a,quint16 b, float &buffer_,QString analyticalModel_)
{
    uint *pTemp=(uint *)&buffer_;
    unsigned int chTemp[4];//a,b,c,d
    chTemp[0]=a&0xff;
    chTemp[1]=(a>>8)&0xff;
    chTemp[2]=b&0xff;
    chTemp[3]=(b>>8)&0xff;
    //这是ABCD
    if(analyticalModel_ =="ABCD")
    {
        *pTemp=((chTemp[1]<<24)&0xff000000)|((chTemp[0]<<16)&0xff0000)|((chTemp[3]<<8)&0xff00)|(chTemp[2]&0xff);
    }
    else if(analyticalModel_ == "CDAB")
    {
        *pTemp=((chTemp[3]<<24)&0xff000000)|((chTemp[2]<<16)&0xff0000)|((chTemp[1]<<8)&0xff00)|(chTemp[0]&0xff);
    }
    else if(analyticalModel_ == "BADC")
    {
        *pTemp=((chTemp[0]<<24)&0xff000000)|((chTemp[1]<<16)&0xff0000)|((chTemp[2]<<8)&0xff00)|(chTemp[3]&0xff);
    }
    else//DCBA
    {
        *pTemp=((chTemp[2]<<24)&0xff000000)|((chTemp[3]<<16)&0xff0000)|((chTemp[0]<<8)&0xff00)|(chTemp[1]&0xff);
    }
}


//删除历史数据
void MainWindow::on_deleteBtn_clicked()
{
    if(ui->deleteLine->text().isEmpty()){
        QMessageBox::information(this, "提示", "输入保留天数！","确定");
        return;
    }
    QMessageBox msgBox;
    msgBox.setText("删除提示");
    msgBox.setWindowTitle("删除提示");
    msgBox.setInformativeText("是否确定删除选择的数据，此操作不可逆！");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes:
        deleteDataSure();
        break;
    case QMessageBox::No:
        break;
    default:
        break;
    }
}
void MainWindow::deleteDataSure()
{
     int day = ui->deleteLine->text().toInt();

     QString defineTime = this->getDefineTimeByDay(-day);

     if(ui->checkBox->isChecked()){
         if(dataOper.deleteGrabDataInfo("Compressor1",defineTime)){
             QMessageBox::information(this, "提示", "删除1号空压机数据成功！","确定");
             ui->checkBox->setChecked(false);
         }
     }
     if(ui->checkBox_2->isChecked()){
         if(dataOper.deleteGrabDataInfo("Compressor2",defineTime)){
             QMessageBox::information(this, "提示", "删除2号空压机数据成功！","确定");
             ui->checkBox_2->setChecked(false);
         }
     }
     if(ui->checkBox_3->isChecked()){
         if(dataOper.deleteGrabDataInfo("Compressor3",defineTime)){
             QMessageBox::information(this, "提示", "删除3号空压机数据成功！","确定");
             ui->checkBox_3->setChecked(false);
         }
     }
     if(ui->checkBox_4->isChecked()){
         if(dataOper.deleteGrabDataInfo("dryer1",defineTime)){
             QMessageBox::information(this, "提示", "删除1号冷干机数据成功！","确定");
             ui->checkBox_4->setChecked(false);
         }
     }
     if(ui->checkBox_6->isChecked()){
         if(dataOper.deleteGrabDataInfo("dryer2",defineTime)){
             QMessageBox::information(this, "提示", "删除2号冷干机数据成功！","确定");
             ui->checkBox_6->setChecked(false);
         }
     }
     if(ui->checkBox_7->isChecked()){
         if(dataOper.deleteGrabDataInfo("dryer3",defineTime)){
             QMessageBox::information(this, "提示", "删除3号冷干机数据成功！","确定");
             ui->checkBox_7->setChecked(false);
         }
     }
     if(ui->checkBox_5->isChecked()){
         if(dataOper.deleteWarningDataInfo(defineTime)){
             QMessageBox::information(this, "提示", "删除设备警告数据成功！","确定");
             ui->checkBox_5->setChecked(false);
         }
     }
     if(ui->checkBox_8->isChecked()){
         if(dataOper.deleteLogDataInfo(defineTime)){
             QMessageBox::information(this, "提示", "删除日志数据成功！","确定");
             ui->checkBox_8->setChecked(false);
         }
     }
     ui->deleteLine->clear();
}



void MainWindow::saveBatchTimingData(QVector<quint16> buffer_)
{
    QDateTime storageTime = QDateTime::currentDateTime();

    Compressor compressor1;
    Compressor compressor2;
    Compressor compressor3;
    Dryer dryer1;
    Dryer dryer2;
    Dryer dryer3;

    compressor1.runTimeL = buffer_.at(25);//累计运行时间 L
    compressor1.runTimeH = buffer_.at(26);//累计运行时间 H
    compressor1.loadTimeL = buffer_.at(27);//累计加载时间 L
    compressor1.loadTimeH = buffer_.at(28);//累计加载时间 H
    compressor1.electricityType = buffer_.at(29);//机型（电流类型）
    compressor1.airDemand = buffer_.at(30);//供气量
    compressor1.jointControlMode = buffer_.at(31);//联控模式
    compressor1.voltageDeviation = buffer_.at(32);//电压偏差
    compressor1.hostCurrent = buffer_.at(33);//主电机电流 Ic
    compressor1.dewPointTemperature = buffer_.at(34);//露点温度 Td
    compressor1.EnvironmentalTemperature = buffer_.at(35);//环境温度
    compressor1.T1 = buffer_.at(36);//T1
    compressor1.T2 = buffer_.at(37);//T2
    compressor1.P1 = buffer_.at(38);//P1
    compressor1.P2 = buffer_.at(39);//P2
    compressor1.T3 = buffer_.at(40);//T3
    compressor1.T4 = buffer_.at(41);//T4
    compressor1.P3 = buffer_.at(42);//P3
    compressor1.P4 = buffer_.at(43);//P4
    compressor1.T5 = buffer_.at(44);//T5
    compressor1.T6 = buffer_.at(45);//T6
    compressor1.runMode1 = buffer_.at(46);//运行状态 1 （注 1）
    compressor1.runMode2 = buffer_.at(47);//运行状态 2 （注 2）
    compressor1.runMode3 = buffer_.at(48);//运行状态 3 （注 3）
    compressor1.dp1 = buffer_.at(49);//dp1（06－3－13 >v2.49）
    compressor1.pressureDiff = buffer_.at(50);//加载压差
    compressor1.uninstallPressure = buffer_.at(51);//卸载压力
    compressor1.MaxManifoldPressure = buffer_.at(52);//最高总管压力
    compressor1.MinManifoldPressure = buffer_.at(53);//最低总管压力
    compressor1.MinimalPressure = buffer_.at(54);//最低压力
    compressor1.StartLoadDelayTime = buffer_.at(55);//启动加载延时时间
    compressor1.StopTime = buffer_.at(56);//卸载停机时间
    compressor1.OrderTime = buffer_.at(57);//顺序时间
    compressor1.RotateTime = buffer_.at(58);//轮换时间
    compressor1.TransitionTime = buffer_.at(59);//Y-△转换时间
    compressor1.date = storageTime;//存储时间

    compressors1.push_back(compressor1);

    compressor2.runTimeL = buffer_.at(75);//累计运行时间 L
    compressor2.runTimeH = buffer_.at(76);//累计运行时间 H
    compressor2.loadTimeL = buffer_.at(77);//累计加载时间 L
    compressor2.loadTimeH = buffer_.at(78);//累计加载时间 H
    compressor2.electricityType = buffer_.at(79);//机型（电流类型）
    compressor2.airDemand = buffer_.at(80);//供气量
    compressor2.jointControlMode = buffer_.at(81);//联控模式
    compressor2.voltageDeviation = buffer_.at(82);//电压偏差
    compressor2.hostCurrent = buffer_.at(83);//主电机电流 Ic
    compressor2.dewPointTemperature = buffer_.at(84);//露点温度 Td
    compressor2.EnvironmentalTemperature = buffer_.at(85);//环境温度
    compressor2.T1 = buffer_.at(86);//T1
    compressor2.T2 = buffer_.at(87);//T2
    compressor2.P1 = buffer_.at(88);//P1
    compressor2.P2 = buffer_.at(89);//P2
    compressor2.T3 = buffer_.at(90);//T3
    compressor2.T4 = buffer_.at(91);//T4
    compressor2.P3 = buffer_.at(92);//P3
    compressor2.P4 = buffer_.at(93);//P4
    compressor2.T5 = buffer_.at(94);//T5
    compressor2.T6 = buffer_.at(95);//T6
    compressor2.runMode1 = buffer_.at(96);//运行状态 1 （注 1）
    compressor2.runMode2 = buffer_.at(97);//运行状态 2 （注 2）
    compressor2.runMode3 = buffer_.at(98);//运行状态 3 （注 3）
    compressor2.dp1 = buffer_.at(99);//dp1（06－3－13 >v2.49）
    compressor2.pressureDiff = buffer_.at(100);//加载压差
    compressor2.uninstallPressure = buffer_.at(101);//卸载压力
    compressor2.MaxManifoldPressure = buffer_.at(102);//最高总管压力
    compressor2.MinManifoldPressure = buffer_.at(103);//最低总管压力
    compressor2.MinimalPressure = buffer_.at(104);//最低压力
    compressor2.StartLoadDelayTime = buffer_.at(105);//启动加载延时时间
    compressor2.StopTime = buffer_.at(106);//卸载停机时间
    compressor2.OrderTime = buffer_.at(107);//顺序时间
    compressor2.RotateTime = buffer_.at(108);//轮换时间
    compressor2.TransitionTime = buffer_.at(109);//Y-△转换时间
    compressor2.date = storageTime;//存储时间

    compressors2.push_back(compressor2);

    compressor3.runTimeL = buffer_.at(125);//累计运行时间 L
    compressor3.runTimeH = buffer_.at(126);//累计运行时间 H
    compressor3.loadTimeL = buffer_.at(127);//累计加载时间 L
    compressor3.loadTimeH = buffer_.at(128);//累计加载时间 H
    compressor3.electricityType = buffer_.at(129);//机型（电流类型）
    compressor3.airDemand = buffer_.at(130);//供气量
    compressor3.jointControlMode = buffer_.at(131);//联控模式
    compressor3.voltageDeviation = buffer_.at(132);//电压偏差
    compressor3.hostCurrent = buffer_.at(133);//主电机电流 Ic
    compressor3.dewPointTemperature = buffer_.at(134);//露点温度 Td
    compressor3.EnvironmentalTemperature = buffer_.at(135);//环境温度
    compressor3.T1 = buffer_.at(136);//T1
    compressor3.T2 = buffer_.at(137);//T2
    compressor3.P1 = buffer_.at(138);//P1
    compressor3.P2 = buffer_.at(139);//P2
    compressor3.T3 = buffer_.at(140);//T3
    compressor3.T4 = buffer_.at(141);//T4
    compressor3.P3 = buffer_.at(142);//P3
    compressor3.P4 = buffer_.at(143);//P4
    compressor3.T5 = buffer_.at(144);//T5
    compressor3.T6 = buffer_.at(145);//T6
    compressor3.runMode1 = buffer_.at(146);//运行状态 1 （注 1）
    compressor3.runMode2 = buffer_.at(147);//运行状态 2 （注 2）
    compressor3.runMode3 = buffer_.at(148);//运行状态 3 （注 3）
    compressor3.dp1 = buffer_.at(149);//dp1（06－3－13 >v2.49）
    compressor3.pressureDiff = buffer_.at(150);//加载压差
    compressor3.uninstallPressure = buffer_.at(151);//卸载压力
    compressor3.MaxManifoldPressure = buffer_.at(152);//最高总管压力
    compressor3.MinManifoldPressure = buffer_.at(153);//最低总管压力
    compressor3.MinimalPressure = buffer_.at(154);//最低压力
    compressor3.StartLoadDelayTime = buffer_.at(155);//启动加载延时时间
    compressor3.StopTime = buffer_.at(156);//卸载停机时间
    compressor3.OrderTime = buffer_.at(157);//顺序时间
    compressor3.RotateTime = buffer_.at(158);//轮换时间
    compressor3.TransitionTime = buffer_.at(159);//Y-△转换时间
    compressor3.date = storageTime;//存储时间

    compressors3.push_back(compressor3);

    dryer1.runHint = buffer_.at(175);//运行提示
    dryer1.faultHint = buffer_.at(176);//故障提示 1
    dryer1.compressor = buffer_.at(177);//压缩机
    dryer1.drainer = buffer_.at(178);//排水器
    dryer1.phaseOrderFault = buffer_.at(179);//相序故障 2
    dryer1.overloadSave = buffer_.at(180);//故障保护 3
    dryer1.sysHighVoltage = buffer_.at(181);//系统高压
    dryer1.sysLowVoltage = buffer_.at(182);//系统低压
    dryer1.dewPointProbeFault = buffer_.at(183);//露点探头故障 4
    dryer1.dewPointH = buffer_.at(184);//露点偏高
    dryer1.dewPointL = buffer_.at(185);//露点偏低
    dryer1.faultWarn = buffer_.at(186);//故障报警 5
    dryer1.faultStop = buffer_.at(187);//故障停机 6
    dryer1.countDown = buffer_.at(188);//倒计时
    dryer1.dewPointT = buffer_.at(189);//露点温度
    dryer1.runTimeH = buffer_.at(190);//运行计时（时）
    dryer1.runTimeM = buffer_.at(191);//运行计时（分）
    dryer1.date = storageTime;//存储时间

    dryers1.push_back(dryer1);

    dryer2.runHint = buffer_.at(225);//运行提示
    dryer2.faultHint = buffer_.at(226);//故障提示 1
    dryer2.compressor = buffer_.at(227);//压缩机
    dryer2.drainer = buffer_.at(228);//排水器
    dryer2.phaseOrderFault = buffer_.at(229);//相序故障 2
    dryer2.overloadSave = buffer_.at(230);//故障保护 3
    dryer2.sysHighVoltage = buffer_.at(231);//系统高压
    dryer2.sysLowVoltage = buffer_.at(232);//系统低压
    dryer2.dewPointProbeFault = buffer_.at(233);//露点探头故障 4
    dryer2.dewPointH = buffer_.at(234);//露点偏高
    dryer2.dewPointL = buffer_.at(235);//露点偏低
    dryer2.faultWarn = buffer_.at(236);//故障报警 5
    dryer2.faultStop = buffer_.at(237);//故障停机 6
    dryer2.countDown = buffer_.at(238);//倒计时
    dryer2.dewPointT = buffer_.at(239);//露点温度
    dryer2.runTimeH = buffer_.at(240);//运行计时（时）
    dryer2.runTimeM = buffer_.at(241);//运行计时（分）
    dryer2.date = storageTime;//存储时间

    dryers2.push_back(dryer2);

    dryer3.runHint = buffer_.at(275);//运行提示
    dryer3.faultHint = buffer_.at(276);//故障提示 1
    dryer3.compressor = buffer_.at(277);//压缩机
    dryer3.drainer = buffer_.at(278);//排水器
    dryer3.phaseOrderFault = buffer_.at(279);//相序故障 2
    dryer3.overloadSave = buffer_.at(280);//故障保护 3
    dryer3.sysHighVoltage = buffer_.at(281);//系统高压
    dryer3.sysLowVoltage = buffer_.at(282);//系统低压
    dryer3.dewPointProbeFault = buffer_.at(283);//露点探头故障 4
    dryer3.dewPointH = buffer_.at(284);//露点偏高
    dryer3.dewPointL = buffer_.at(285);//露点偏低
    dryer3.faultWarn = buffer_.at(286);//故障报警 5
    dryer3.faultStop = buffer_.at(287);//故障停机 6
    dryer3.countDown = buffer_.at(288);//倒计时
    dryer3.dewPointT = buffer_.at(289);//露点温度
    dryer3.runTimeH = buffer_.at(290);//运行计时（时）
    dryer3.runTimeM = buffer_.at(291);//运行计时（分）
    dryer3.date = storageTime;//存储时间

    dryers3.push_back(dryer3);

    //存储 空压机 报警数据


    saveWarning(buffer_.at(46), 1,1);
    saveWarning(buffer_.at(47), 1,2);

    saveWarning(buffer_.at(96), 2,1);
    saveWarning(buffer_.at(97), 2,2);

    saveWarning(buffer_.at(146), 3,1);
    saveWarning(buffer_.at(147), 3,2);


    if(buffer_.at(176)){
        saveWarningDryer(1,1);
    }
    if(buffer_.at(179)){
        saveWarningDryer(1,2);
    }
    if(buffer_.at(180)){
        saveWarningDryer(1,3);
    }
    if(buffer_.at(183)){
        saveWarningDryer(1,4);
    }
    if(buffer_.at(186)){
        saveWarningDryer(1,5);
    }
    if(buffer_.at(187)){
        saveWarningDryer(1,6);
    }



    if(buffer_.at(226)){
        saveWarningDryer(2,1);
    }
    if(buffer_.at(229)){
        saveWarningDryer(2,2);
    }
    if(buffer_.at(230)){
        saveWarningDryer(2,3);
    }
    if(buffer_.at(233)){
        saveWarningDryer(2,4);
    }
    if(buffer_.at(236)){
        saveWarningDryer(2,5);
    }
    if(buffer_.at(237)){
        saveWarningDryer(2,6);
    }



    if(buffer_.at(276)){
        saveWarningDryer(3,1);
    }
    if(buffer_.at(279)){
        saveWarningDryer(3,2);
    }
    if(buffer_.at(280)){
        saveWarningDryer(3,3);
    }
    if(buffer_.at(283)){
        saveWarningDryer(3,4);
    }
    if(buffer_.at(286)){
        saveWarningDryer(3,5);
    }
    if(buffer_.at(287)){
        saveWarningDryer(3,6);
    }
}


void MainWindow::saveData(vector<Compressor> c1,vector<Compressor> c2,vector<Compressor> c3,
                          vector<Dryer> d1,vector<Dryer> d2,vector<Dryer> d3,vector<Warning> w)
{
    if(storageInterval2 == STORE_TIME2){
        isStoreData = false;
        storageInterval2 = 0;
        if(dataOper.saveData(c1,c2,c3,d1,d2,d3,w)){
            compressors1.clear();
            compressors2.clear();
            compressors3.clear();
            dryers1.clear();
            dryers2.clear();
            dryers3.clear();
            warnings.clear();
            isStoreData = true;
        }
    }
    storageInterval2 += READ_TIME;
}


