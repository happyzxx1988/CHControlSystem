#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "compressorsetdialog.h"
#include "ui_compressorsetdialog.h"

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
    ui->uninstallPressure->setText(QString::number(compressorSet.uninstallPressure,'f',4));
    ui->pressureDiff->setText(QString::number(compressorSet.pressureDiff,'f',4));

}

CompressorSetDialog::~CompressorSetDialog()
{
    delete ui;
}

void CompressorSetDialog::on_setCompressorBtn_clicked()
{
    float uninstallPressure = ui->uninstallPressure->text().isEmpty() ? 0 : ui->uninstallPressure->text().toFloat();
    float pressureDiff = ui->pressureDiff->text().isEmpty() ? 0 : ui->pressureDiff->text().toFloat();

    appcore->setUninstallPressureAndPressureDiff(uninstallPressure,pressureDiff,compressorNo);

    emit closeCurrentDialog(compressorNo,uninstallPressure,pressureDiff);
    this->close();
}
