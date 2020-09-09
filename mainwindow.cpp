#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDesktopWidget>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"

#define STATION_HOUSE   "G11"        //站房
#define STORE_TIME      60000      //储存读取数据的时间间隔  暂定1分钟

MainWindow::MainWindow(QString u, QString p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    userName(u),
    pass(p)
{
    ui->setupUi(this);
    this->initForm();
    this->initTable();
    this->initChart();

//    ui->sys_img->setPixmap(QPixmap(":/images/images/log.png"));
//    ui->sys_name->setText("长虹智能空压站房系统");
//    setWindowTitle("长虹智能空压站房系统");
}

MainWindow::~MainWindow()
{
    delete ui;
    if(compressorTimer.isActive()){
        compressorTimer.stop();
    }
//    system("taskkill /f /t /im CHControlSystem.exe");

    QProcess *process = new QProcess;
    process->start("taskkill", QStringList() << "/f" << "/im" << "CHControlSystem.exe");
}


void MainWindow::initForm()
{
    QDate date = QDate::currentDate();   //获取当前日期
    READ_TIME =      10;    //定时读取间隔时间

    ui->warningStartime->setDate(date);
    ui->warningEndTime->setDate(date);
    ui->logStartime->setDate(date);
    ui->logEndTime->setDate(date);
    ui->runningChartS->setDate(date);
    ui->runningChartE->setDate(date);
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
    storageInterval = 0;

    connect(&timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer.start(1000);

    connect(&compressorTimer,SIGNAL(timeout()),this,SLOT(readCompressorTimer()));

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

        //设备启动，设置手动模式
        appcore.setRunMode(ManualMode);
        ui->runningMode->setText("手动");
        ui->manualOper->setChecked(true);

        if(!compressorTimer.isActive()){
            compressorTimer.start(READ_TIME);
        }
    });
    connect(&appcore, &AppCore::deviceDisconnected, this, [this](){
        qDebug() << "emit deviceDisconnected()";
        ui->connectPLCBtn->setText("已断开");
        ui->connectPLCBtn->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->connectPLCBtn->setEnabled(true);
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
    connect(ui->pushButton_1, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_4, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_5, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_6, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));
    connect(ui->pushButton_7, SIGNAL(clicked()), this, SLOT(on_listView_pressed()));

    // Initialize settings
    appcore.initSettings();
    // Connect to Device
    appcore.initDevice();

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
    m_valueLabel->setFixedSize(44, 24);
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
    up_x->setFormat("yy-MM-dd hh:mm");
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
    pd_x->setFormat("yy-MM-dd hh:mm");
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
void MainWindow::on_listView_pressed()
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
    } else if (text == "历史数据") {
        ui->stackedWidget->setCurrentIndex(5);
    } else if (text == "用户管理") {
        ui->stackedWidget->setCurrentIndex(6);
        on_userClearBtn_clicked();
    }
}
//连接PLC
void MainWindow::on_connectPLCBtn_clicked()
{
    appcore.connectPLC();
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
    dataOper.getAllWarningInfo(warnings,2,s_time,e_time);
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
    pressureSetDialog = new PressureSetDialog(userName,&appcore);
    connect(pressureSetDialog,&PressureSetDialog::closeCurrentDialog,this,&MainWindow::pressure_call_back);
    pressureSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    pressureSetDialog->setWindowModality(Qt::ApplicationModal);
    pressureSetDialog->show();
}
//设备控制---压力设置回调函数
void MainWindow::pressure_call_back(int maxPressure, int minPressure)
{
    dataOper.saveLog(STATION_HOUSE,"运行模式压力设置",QString("最大压力:%1,最小压力:%2").arg(maxPressure).arg(minPressure),
                     userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}
//设备控制---1#空压机设置
void MainWindow::on_compressorSetBtn1_clicked()
{

    compressorSetDialog = new CompressorSetDialog(userName,1,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---2#空压机设置
void MainWindow::on_compressorSetBtn2_clicked()
{
    compressorSetDialog = new CompressorSetDialog(userName,2,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---3#空压机设置
void MainWindow::on_compressorSetBtn3_clicked()
{
    compressorSetDialog = new CompressorSetDialog(userName,3,&appcore);
    connect(compressorSetDialog,&CompressorSetDialog::closeCurrentDialog,this,&MainWindow::compressor_call_back);
    compressorSetDialog->setAttribute(Qt::WA_DeleteOnClose);
    compressorSetDialog->setWindowModality(Qt::ApplicationModal);
    compressorSetDialog->show();
}
//设备控制---空压机设置完毕，回调函数
void MainWindow::compressor_call_back(int compressorNO,int uninstallPressure,int pressureDiff)
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
    appcore.resetOperation();
    dataOper.saveLog(STATION_HOUSE,"运行模式","运行模式复位设置",userName,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}
//设备控制---1#冷干机开关
void MainWindow::on_dryerBtn1_clicked()
{
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
    QVector<quint16> compressor1;
    QVector<quint16> compressor2;
    QVector<quint16> compressor3;
    QVector<quint16> dryer1;
    QVector<quint16> dryer2;
    QVector<quint16> dryer3;
//    appcore.readCompressor(compressor1,compressor2,compressor3,dryer1,dryer2,dryer3);

    appcore.readCompressor1(compressor1);
    appcore.readCompressor2(compressor2);
    appcore.readCompressor3(compressor3);
    appcore.dryer1(dryer1);
    appcore.dryer2(dryer2);
    appcore.dryer3(dryer3);


    dealCompressor1(compressor1,dryer1);
    dealCompressor2(compressor2,dryer2);
    dealCompressor3(compressor3,dryer3);

    if(storageInterval == STORE_TIME){
        storageInterval = 0;
        QString storageTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        //存储读取数据导数据
        if(compressor1.size() == 35){
            dataOper.saveReadCompressor1(compressor1,storageTime);
        }
        if(compressor2.size() == 35){
            dataOper.saveReadCompressor2(compressor2,storageTime);
        }
        if(compressor3.size() == 35){
            dataOper.saveReadCompressor3(compressor3,storageTime);
        }
        if(dryer1.size() == 17){
            dataOper.saveReadDryer1(dryer1,storageTime);
        }
        if(dryer2.size() == 17){
            dataOper.saveReadDryer2(dryer2,storageTime);
        }
        if(dryer3.size() == 17){
            dataOper.saveReadDryer3(dryer3,storageTime);
        }

    }
    storageInterval += READ_TIME;

}
//显示1#空压机读取的数据
void MainWindow::dealCompressor1(QVector<quint16> compressor, QVector<quint16> dryer)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        saveWarning(runMode1, 1,1);
        saveWarning(runMode2, 1,2);
//        saveWarning(runMode3, 1,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState1->setText("运行中");//运行状态
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState1->setText("停止");//运行状态
            }
        }
        if(aa.at(10)){
            ui->runningState1->setText("记载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState1->setText("满载");//运行状态
        }
        ui->powerStatus1->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->ventingPressure1->setText(QString::number(P2/142.0,'f',2));//排气压力
        ui->ventingT1->setText(QString::number(T1));//排气温度
        ui->runningT1->setText(QString::number(runTimeL));//运行时间
        ui->hostA1->setText(QString::number(hostCurrent));//主机电流
        ui->sysT1->setText(QString::number(T2));//系统温度
        ui->uploadT1->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState1->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->pressureDiff1->setText(QString::number(pressureDiff));//加载压差
        ui->uninstallPressure1->setText(QString::number(uninstallPressure));//卸载压力
        //设备控制
        ui->compressorBtn1->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch1 = aa.at(9) ? true : false;
        ui->controlPressureDiff1->setText(QString::number(pressureDiff));//加载压差
        ui->controlUninstallPressure1->setText(QString::number(uninstallPressure));//卸载压力
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
        if(faultHint){
            saveWarningDryer(1,1);
        }
        if(phaseOrderFault){
            saveWarningDryer(1,2);
        }
        if(overloadSave){
            saveWarningDryer(1,3);
        }
        if(dewPointProbeFault){
            saveWarningDryer(1,4);
        }
        if(faultWarn){
            saveWarningDryer(1,5);
        }
        if(faultStop){
            saveWarningDryer(1,6);
        }
        //主界面
        ui->valveState1->setText(runHint ? "运行中" : "停机状态");//运行状态
        ui->faultHint1->setText(faultHint ? "故障" : "无故障");//故障提示
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
}
//显示1#空压机读取的数据
void MainWindow::dealCompressor2(QVector<quint16> compressor, QVector<quint16> dryer)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        saveWarning(runMode1, 2,1);
        saveWarning(runMode2, 2,2);
//        saveWarning(runMode3, 2,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState2->setText("运行中");//运行状态
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState2->setText("停止");//运行状态
            }
        }
        if(aa.at(10)){
            ui->runningState2->setText("记载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState2->setText("满载");//运行状态
        }
        ui->powerStatus2->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->ventingPressure2->setText(QString::number(P2/142.0,'f',2));//排气压力
        ui->ventingT2->setText(QString::number(T1));//排气温度
        ui->runningT2->setText(QString::number(runTimeL));//运行时间
        ui->hostA2->setText(QString::number(hostCurrent));//主机电流
        ui->sysT2->setText(QString::number(T2));//系统温度
        ui->uploadT2->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState2->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->pressureDiff2->setText(QString::number(pressureDiff));//加载压差
        ui->uninstallPressure2->setText(QString::number(uninstallPressure));//卸载压力
        //设备控制
        ui->compressorBtn2->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch2 = aa.at(9) ? true : false;
        ui->controlPressureDiff2->setText(QString::number(pressureDiff));//加载压差
        ui->controlUninstallPressure2->setText(QString::number(uninstallPressure));//卸载压力
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
        if(faultHint){
            saveWarningDryer(2,1);
        }
        if(phaseOrderFault){
            saveWarningDryer(2,2);
        }
        if(overloadSave){
            saveWarningDryer(2,3);
        }
        if(dewPointProbeFault){
            saveWarningDryer(2,4);
        }
        if(faultWarn){
            saveWarningDryer(2,5);
        }
        if(faultStop){
            saveWarningDryer(2,6);
        }

        //主界面
        ui->valveState2->setText(runHint ? "运行中" : "停机状态");//阀门状态
        ui->faultHint2->setText(faultHint ? "故障" : "无故障");//故障提示
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
//显示1#空压机读取的数据
void MainWindow::dealCompressor3(QVector<quint16> compressor, QVector<quint16> dryer)
{
    if(compressor.size() == 35){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P2 = compressor.at(14);//P2
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        saveWarning(runMode1, 3,1);
        saveWarning(runMode2, 3,2);
//        saveWarning(runMode3, 3,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        if(aa.at(9)){
            ui->runningState3->setText("运行中");//运行状态
        }else{
            if(!aa.at(10) && !aa.at(11)){
                ui->runningState3->setText("停止");//运行状态
            }
        }
        if(aa.at(10)){
            ui->runningState3->setText("记载");//运行状态
        }
        if(aa.at(11)){
            ui->runningState3->setText("满载");//运行状态
        }
        ui->powerStatus3->setText(aa.at(8) ? "上电" : "断电");//电源
        ui->ventingPressure3->setText(QString::number(P2/142.0,'f',2));//排气压力
        ui->ventingT3->setText(QString::number(T1));//排气温度
        ui->runningT3->setText(QString::number(runTimeL));//运行时间
        ui->hostA3->setText(QString::number(hostCurrent));//主机电流
        ui->sysT3->setText(QString::number(T2));//系统温度
        ui->uploadT3->setText(QString::number(loadTimeL));//加载时间
        //设备状态
        ui->compressorRunState3->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->pressureDiff3->setText(QString::number(pressureDiff));//加载压差
        ui->uninstallPressure3->setText(QString::number(uninstallPressure));//卸载压力
        //设备控制
        ui->compressorBtn3->setIcon(aa.at(9) ? QIcon(":/images/images/btncheckon2.png") : QIcon(":/images/images/btncheckoff2.png"));
        compressorSwitch3 = aa.at(9) ? true : false;
        ui->controlPressureDiff3->setText(QString::number(pressureDiff));//加载压差
        ui->controlUninstallPressure3->setText(QString::number(uninstallPressure));//卸载压力
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
        if(faultHint){
            saveWarningDryer(3,1);
        }
        if(phaseOrderFault){
            saveWarningDryer(3,2);
        }
        if(overloadSave){
            saveWarningDryer(3,3);
        }
        if(dewPointProbeFault){
            saveWarningDryer(3,4);
        }
        if(faultWarn){
            saveWarningDryer(3,5);
        }
        if(faultStop){
            saveWarningDryer(3,6);
        }

        //主界面
        ui->valveState3->setText(runHint ? "运行中" : "停机状态");//阀门状态
        ui->faultHint3->setText(faultHint ? "故障" : "无故障");//故障提示
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
        dataOper.saveWarningInfo(warning);
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
    dataOper.saveWarningInfo(warning);
}





//鼠标移动显示曲线数据
void MainWindow::slotPointHoverd(const QPointF &point, bool state)
{
    if (state){
        m_valueLabel->setText(QString::number(point.y()));
        QPoint curPos = mapFromGlobal(QCursor::pos());
        m_valueLabel->move(curPos.x() - m_valueLabel->width() / 2, curPos.y() - m_valueLabel->height() * 1.5);//移动数值
        m_valueLabel->show();//显示出来
    }else{
        m_valueLabel->hide();//进行隐藏
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


//解析无符号的整型数据
QVector<bool> MainWindow::dec2BinTrans(unsigned int data)
{
    QVector<bool> bin;
    for(int i = 0; i < 16 && data != 0; i++)
    {
        bin.push_back(data%2);
        data/=2;
    }
    QVector<bool> bintemp(16,0);//初始化16个元素，每个初始化为0
    for(int i = 0; i < 8 && i < bin.size(); i++)
    {
        bintemp[i+8] = bin[i];
    }
    for(int i = 8; i < 16 && i < bin.size(); i++)
    {
        bintemp[i-8] = bin[i];
    }
    return bintemp;
}

