#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "compressorsetdialog.h"
#include "ui_compressorsetdialog.h"
#include <QMessageBox>

CompressorSetDialog::CompressorSetDialog(QString user, int type, AppCore *core, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompressorSetDialog),
    currentUser(user),
    compressorNo(type),
    appcore(core)
{
    ui->setupUi(this);
    // 去掉问号，只保留关闭
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    QString title = QString("%1#空压机参数设置").arg(compressorNo);
    this->setWindowTitle(title);

    CompressorSet compressorSet;
    dataOper.getLastCompressorSet(compressorSet,compressorNo);
    ui->uninstallPressure->setText(QString::number(compressorSet.uninstallPressure,'f',1));
    ui->pressureDiff->setText(QString::number(compressorSet.pressureDiff,'f',1));

}

CompressorSetDialog::~CompressorSetDialog()
{
    delete ui;
}

void CompressorSetDialog::on_setCompressorBtn_clicked()
{
    if(ui->uninstallPressure->text().isEmpty() || ui->pressureDiff->text().isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("请输入值！"),tr("确定"));
        return;
    }

    float uninstallPressure = ui->uninstallPressure->text().toFloat();
    float pressureDiff = ui->pressureDiff->text().toFloat();

    if(uninstallPressure < 0.0 || pressureDiff > 2.0 || uninstallPressure < 0.0 || pressureDiff > 2.0){
        QMessageBox::information(this, tr("提示"), tr("卸载压差(加载压力)值,只能是0-2的两位小数值！"),tr("确定"));
        return;
    }


    int uninstallPressure_int = QString::number(uninstallPressure,'f',2).toInt() * 142;
    int pressureDiff_int = QString::number(pressureDiff,'f',2).toInt() * 142;

    appcore->setUninstallPressureAndPressureDiff(uninstallPressure_int,pressureDiff_int,compressorNo);

    emit closeCurrentDialog(compressorNo,uninstallPressure_int,pressureDiff_int);
    this->close();
}
