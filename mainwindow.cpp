#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDesktopWidget>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"
#include "savelog.h"

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

    SaveLog::Instance()->setPath(qApp->applicationDirPath());

    SaveLog::Instance()->start();

//    ui->sys_img->hide();
//    ui->sys_name->hide();
//    ui->sys_img->setPixmap(QPixmap(":/images/images/log.png"));
//    ui->sys_name->setText("长虹智能空压站房系统");
//    setWindowTitle("长虹智能空压站房系统");
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::initForm()
{
    QDate date = QDate::currentDate();   //获取当前日期
    READ_TIME =      5000;    //定时读取间隔时间
    ui->listView->setIcoColorBg(false);
    ui->listView->setColorLine(QColor(193, 193, 193));
    ui->listView->setColorBg(QColor(255, 255, 255), QColor(232, 236, 245), QColor(242, 242, 242));
    ui->listView->setColorText(QColor(19, 36, 62), QColor(19, 36, 62), QColor(19, 36, 62));
    //加载xml文件形式
    ui->listView->readData(":/image/config.xml");
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

    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);

    QTimer *compressorTimer = new QTimer(this);//读取空压机定时器 5秒钟读取一次
    connect(compressorTimer,SIGNAL(timeout()),this,SLOT(readCompressorTimer()));

    connect(ui->userTable,SIGNAL(cellClicked(int,int)),this,SLOT(checkRowSlot(int,int)));
    /*数据处理的消息传输*/
    connect(&dataOper, &DataOper::sendDataMessage, this, [this](const QString& info){
        QMessageBox::information(this, tr("提示"), info,tr("确定"));
    });
    connect(&appcore, &AppCore::deviceConnected, this, [this,compressorTimer](){
        qDebug() << "emit deviceConnected()";
        if(!compressorTimer->isActive()){
            compressorTimer->start(READ_TIME);
        }
    });
    connect(&appcore, &AppCore::deviceDisconnected, this, [this,compressorTimer](){
        qDebug() << "emit deviceDisconnected()";
        if(compressorTimer->isActive()){
            compressorTimer->stop();
        }
    });
    connect(&appcore, &AppCore::errorMessage, this, [this](const QString& error){
        qDebug() << "emit errorMessage(error):" << error;
    });
    connect(&appcore, &AppCore::infoMessage, this, [this](const QString& info){
        qDebug() << "emit infoMessage(info):" << info;
    });
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
    QModelIndex index = ui->listView->currentIndex();
    QString text = index.data().toString();

//    <Node label="主界面"></Node>
//<Node label="设备状态"></Node>
//    <Node label="设备控制"></Node>
//<Node label="操作日志"></Node>
//    <Node label="系统报警"></Node>
//    <Node label="历史数据"></Node>
//    <Node label="用户管理"></Node>


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
        on_warningResetBtn_clicked();
    } else if (text == "历史数据") {
        ui->stackedWidget->setCurrentIndex(5);
    } else if (text == "用户管理") {
        ui->stackedWidget->setCurrentIndex(6);
        on_userClearBtn_clicked();
    }
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
void MainWindow::pressure_call_back(float maxPressure,float minPressure)
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
void MainWindow::compressor_call_back(int compressorNO,float uninstallPressure,float pressureDiff)
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
//    switch (compressorNO) {
//    case 1:
//        qDebug() << "on_compressor_call_back1";
//        dataOper.saveLog(STATION_HOUSE,"1#空压机","压差设置",userName,currentTime);
//        break;
//    case 2:
//        qDebug() << "on_compressor_call_back2";
//        dataOper.saveLog(STATION_HOUSE,"2#空压机","压差设置",userName,currentTime);
//        break;
//    case 3:
//        qDebug() << "on_compressor_call_back3";
//        dataOper.saveLog(STATION_HOUSE,"3#空压机","压差设置",userName,currentTime);
//        break;
//    default:
//        break;
//    }
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
    appcore.readCompressor(compressor1,compressor2,compressor3);
    appcore.readDryer(dryer1,dryer2,dryer3);

    int data1_size = compressor1.size();
    int data2_size = compressor2.size();
    int data3_size = compressor3.size();
    int dryerData1_size = dryer1.size();
    int dryerData2_size = dryer2.size();
    int dryerData3_size = dryer3.size();

    dealCompressor1(compressor1,dryer1);
    dealCompressor2(compressor2,dryer2);
    dealCompressor3(compressor3,dryer2);

    if(storageInterval == STORE_TIME){
        storageInterval = 0;
        QString storageTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        //存储读取数据导数据
        if(data1_size != 0){
            dataOper.saveReadCompressor1(compressor1,storageTime);
        }
        if(data2_size != 0){
            dataOper.saveReadCompressor2(compressor2,storageTime);
        }
        if(data3_size != 0){
            dataOper.saveReadCompressor3(compressor3,storageTime);
        }
        if(dryerData1_size != 0){
            dataOper.saveReadDryer1(dryer1,storageTime);
        }
        if(dryerData2_size != 0){
            dataOper.saveReadDryer2(dryer2,storageTime);
        }
        if(dryerData3_size != 0){
            dataOper.saveReadDryer3(dryer3,storageTime);
        }

    }
    storageInterval += READ_TIME;

}
//显示1#空压机读取的数据
void MainWindow::dealCompressor1(QVector<quint16> compressor, QVector<quint16> dryer)
{
    int compressor_size = compressor.size();
    int dryer_size = dryer.size();
    if(compressor_size != 0){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 runTimeH = compressor.at(1);//累计运行时间 H
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 loadTimeH = compressor.at(3);//累计加载时间 H
        quint16 electricityType = compressor.at(4);//机型（电流类型）
        quint16 airDemand = compressor.at(5);//供气量
        quint16 jointControlMode = compressor.at(6);//联控模式
        quint16 voltageDeviation = compressor.at(7);//电压偏差
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 dewPointTemperature = compressor.at(9);//露点温度 Td
        quint16 EnvironmentalTemperature = compressor.at(10);//环境温度
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 T3 = compressor.at(15);//T3
        quint16 T4 = compressor.at(16);//T4
        quint16 P3 = compressor.at(17);//P3
        quint16 P4 = compressor.at(18);//P4
        quint16 T5 = compressor.at(19);//T5
        quint16 T6 = compressor.at(20);//T6
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 dp1 = compressor.at(24);//dp1（06－3－13 >v2.49）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        quint16 MaxManifoldPressure = compressor.at(27);//最高总管压力
        quint16 MinManifoldPressure = compressor.at(28);//最低总管压力
        quint16 MinimalPressure = compressor.at(29);//最低压力
        quint16 StartLoadDelayTime = compressor.at(30);//启动加载延时时间
        quint16 StopTime = compressor.at(31);//卸载停机时间
        quint16 OrderTime = compressor.at(32);//顺序时间
        quint16 RotateTime = compressor.at(33);//轮换时间
        quint16 TransitionTime = compressor.at(34);//Y-△转换时间
        saveWarning(runMode1, 1,1);
        saveWarning(runMode2, 1,2);
        saveWarning(runMode3, 1,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        ui->runningState1->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->ventingPressure1->setText(QString::number(P2));//排气压力
        ui->ventingT1->setText(QString::number(T2));//排气温度
        ui->runningT1->setText(QString::number(runTimeL));//运行时间
        ui->hostA1->setText(QString::number(hostCurrent));//主机电流
        ui->sysT1->setText(QString::number(runTimeL));//系统温度
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
    if(dryer_size != 0){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示
        quint16 compressor = dryer.at(2);//压缩机
        quint16 drainer = dryer.at(3);//排水器
        quint16 phaseOrderFault = dryer.at(4);//相序故障
        quint16 overloadSave = dryer.at(5);//故障保护
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障
        quint16 dewPointH = dryer.at(9);//露点偏高
        quint16 dewPointL = dryer.at(10);//露点偏低
        quint16 faultWarn = dryer.at(11);//故障报警
        quint16 faultStop = dryer.at(12);//故障停机
        quint16 countDown = dryer.at(13);//倒计时
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）

//        saveWarningDryer(runMode1_dryer, 3,1);
//        saveWarningDryer(runMode2_dryer, 3,2);
//        saveWarningDryer(runMode3_dryer, 3,3);

//        QVector<bool> aa_dryer = dec2BinTrans(runMode3_dryer);

        //主界面
        ui->valveState1->setText(QString::number(runHint));//阀门状态
        ui->refrigerantH1->setText(QString::number(sysHighVoltage));//冷媒高压
        ui->refrigerantL1->setText(QString::number(sysLowVoltage));//冷媒低压
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
    int compressor_size = compressor.size();
    int dryer_size = dryer.size();
    if(compressor_size != 0){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 runTimeH = compressor.at(1);//累计运行时间 H
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 loadTimeH = compressor.at(3);//累计加载时间 H
        quint16 electricityType = compressor.at(4);//机型（电流类型）
        quint16 airDemand = compressor.at(5);//供气量
        quint16 jointControlMode = compressor.at(6);//联控模式
        quint16 voltageDeviation = compressor.at(7);//电压偏差
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 dewPointTemperature = compressor.at(9);//露点温度 Td
        quint16 EnvironmentalTemperature = compressor.at(10);//环境温度
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 T3 = compressor.at(15);//T3
        quint16 T4 = compressor.at(16);//T4
        quint16 P3 = compressor.at(17);//P3
        quint16 P4 = compressor.at(18);//P4
        quint16 T5 = compressor.at(19);//T5
        quint16 T6 = compressor.at(20);//T6
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 dp1 = compressor.at(24);//dp1（06－3－13 >v2.49）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        quint16 MaxManifoldPressure = compressor.at(27);//最高总管压力
        quint16 MinManifoldPressure = compressor.at(28);//最低总管压力
        quint16 MinimalPressure = compressor.at(29);//最低压力
        quint16 StartLoadDelayTime = compressor.at(30);//启动加载延时时间
        quint16 StopTime = compressor.at(31);//卸载停机时间
        quint16 OrderTime = compressor.at(32);//顺序时间
        quint16 RotateTime = compressor.at(33);//轮换时间
        quint16 TransitionTime = compressor.at(34);//Y-△转换时间
        saveWarning(runMode1, 2,1);
        saveWarning(runMode2, 2,2);
        saveWarning(runMode3, 2,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        ui->runningState2->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->ventingPressure2->setText(QString::number(P2));//排气压力
        ui->ventingT2->setText(QString::number(T2));//排气温度
        ui->runningT2->setText(QString::number(runTimeL));//运行时间
        ui->hostA2->setText(QString::number(hostCurrent));//主机电流
        ui->sysT2->setText(QString::number(runTimeL));//系统温度
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
    if(dryer_size != 0){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示
        quint16 compressor = dryer.at(2);//压缩机
        quint16 drainer = dryer.at(3);//排水器
        quint16 phaseOrderFault = dryer.at(4);//相序故障
        quint16 overloadSave = dryer.at(5);//故障保护
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障
        quint16 dewPointH = dryer.at(9);//露点偏高
        quint16 dewPointL = dryer.at(10);//露点偏低
        quint16 faultWarn = dryer.at(11);//故障报警
        quint16 faultStop = dryer.at(12);//故障停机
        quint16 countDown = dryer.at(13);//倒计时
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）

//        saveWarningDryer(runMode1_dryer, 3,1);
//        saveWarningDryer(runMode2_dryer, 3,2);
//        saveWarningDryer(runMode3_dryer, 3,3);

//        QVector<bool> aa_dryer = dec2BinTrans(runMode3_dryer);

        //主界面
        ui->valveState2->setText(QString::number(runHint));//阀门状态
        ui->refrigerantH2->setText(QString::number(sysHighVoltage));//冷媒高压
        ui->refrigerantL2->setText(QString::number(sysLowVoltage));//冷媒低压
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
    int compressor_size = compressor.size();
    int dryer_size = dryer.size();
    if(compressor_size != 0){
        quint16 runTimeL = compressor.at(0);//累计运行时间 L
        quint16 runTimeH = compressor.at(1);//累计运行时间 H
        quint16 loadTimeL = compressor.at(2);//累计加载时间 L
        quint16 loadTimeH = compressor.at(3);//累计加载时间 H
        quint16 electricityType = compressor.at(4);//机型（电流类型）
        quint16 airDemand = compressor.at(5);//供气量
        quint16 jointControlMode = compressor.at(6);//联控模式
        quint16 voltageDeviation = compressor.at(7);//电压偏差
        quint16 hostCurrent = compressor.at(8);//主电机电流 Ic
        quint16 dewPointTemperature = compressor.at(9);//露点温度 Td
        quint16 EnvironmentalTemperature = compressor.at(10);//环境温度
        quint16 T1 = compressor.at(11);//T1
        quint16 T2 = compressor.at(12);//T2
        quint16 P1 = compressor.at(13);//P1
        quint16 P2 = compressor.at(14);//P2
        quint16 T3 = compressor.at(15);//T3
        quint16 T4 = compressor.at(16);//T4
        quint16 P3 = compressor.at(17);//P3
        quint16 P4 = compressor.at(18);//P4
        quint16 T5 = compressor.at(19);//T5
        quint16 T6 = compressor.at(20);//T6
        quint16 runMode1 = compressor.at(21);//运行状态 1 （注 1）
        quint16 runMode2 = compressor.at(22);//运行状态 2 （注 2）
        quint16 runMode3 = compressor.at(23);//运行状态 3 （注 3）
        quint16 dp1 = compressor.at(24);//dp1（06－3－13 >v2.49）
        quint16 pressureDiff = compressor.at(25);//加载压差
        quint16 uninstallPressure = compressor.at(26);//卸载压力
        quint16 MaxManifoldPressure = compressor.at(27);//最高总管压力
        quint16 MinManifoldPressure = compressor.at(28);//最低总管压力
        quint16 MinimalPressure = compressor.at(29);//最低压力
        quint16 StartLoadDelayTime = compressor.at(30);//启动加载延时时间
        quint16 StopTime = compressor.at(31);//卸载停机时间
        quint16 OrderTime = compressor.at(32);//顺序时间
        quint16 RotateTime = compressor.at(33);//轮换时间
        quint16 TransitionTime = compressor.at(34);//Y-△转换时间
        saveWarning(runMode1, 3,1);
        saveWarning(runMode2, 3,2);
        saveWarning(runMode3, 3,3);
        QVector<bool> aa = dec2BinTrans(runMode3);
        //主界面
        ui->runningState3->setText(aa.at(9) ? "运行中" : "停止");//运行状态
        ui->ventingPressure3->setText(QString::number(P2));//排气压力
        ui->ventingT3->setText(QString::number(T2));//排气温度
        ui->runningT3->setText(QString::number(runTimeL));//运行时间
        ui->hostA3->setText(QString::number(hostCurrent));//主机电流
        ui->sysT3->setText(QString::number(runTimeL));//系统温度
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
    if(dryer_size != 0){
        quint16 runHint = dryer.at(0);//运行提示
        quint16 faultHint = dryer.at(1);//故障提示
        quint16 compressor = dryer.at(2);//压缩机
        quint16 drainer = dryer.at(3);//排水器
        quint16 phaseOrderFault = dryer.at(4);//相序故障
        quint16 overloadSave = dryer.at(5);//故障保护
        quint16 sysHighVoltage = dryer.at(6);//系统高压
        quint16 sysLowVoltage = dryer.at(7);//系统低压
        quint16 dewPointProbeFault = dryer.at(8);//露点探头故障
        quint16 dewPointH = dryer.at(9);//露点偏高
        quint16 dewPointL = dryer.at(10);//露点偏低
        quint16 faultWarn = dryer.at(11);//故障报警
        quint16 faultStop = dryer.at(12);//故障停机
        quint16 countDown = dryer.at(13);//倒计时
        quint16 dewPointT = dryer.at(14);//露点温度
        quint16 runTimeH = dryer.at(15);//运行计时（时）
        quint16 runTimeM = dryer.at(16);//运行计时（分）

//        saveWarningDryer(runMode1_dryer, 3,1);
//        saveWarningDryer(runMode2_dryer, 3,2);
//        saveWarningDryer(runMode3_dryer, 3,3);

//        QVector<bool> aa_dryer = dec2BinTrans(runMode3_dryer);

        //主界面
        ui->valveState3->setText(QString::number(runHint));//阀门状态
        ui->refrigerantH3->setText(QString::number(sysHighVoltage));//冷媒高压
        ui->refrigerantL3->setText(QString::number(sysLowVoltage));//冷媒低压
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
void MainWindow::saveWarningDryer(quint16 warningInfo, int dryerNo, int runState)
{
    Warning warning;
    QVector<bool> aa = dec2BinTrans(warningInfo);
    warning.address = STATION_HOUSE;
    warning.deviceName = QString("%1#冷干机").arg(dryerNo);
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





