#pragma execution_character_set("utf-8")  //加了这句话之后，就不用QStringLiteral了
#include "pressuresetdialog.h"
#include "ui_pressuresetdialog.h"
#include <QMessageBox>

/**
 * @brief PressureSetDialog::PressureSetDialog   压力设置窗口
 * @param parent
 */
PressureSetDialog::PressureSetDialog(QString user, AppCore *core, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PressureSetDialog),
    currentUser(user),
    appcore(core)
{
    ui->setupUi(this);
    // 去掉问号，只保留关闭
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle("压力设置");

}

PressureSetDialog::~PressureSetDialog()
{
    delete ui;
}
//压力设置按钮
void PressureSetDialog::on_setPressureBtn_clicked()
{
    if(ui->maxPressure->text().isEmpty() || ui->minPressure->text().isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("请输入值最大最小压力值！"),tr("确定"));
        return;
    }
    float maxPressure = ui->maxPressure->text().toFloat();
    float minPressure = ui->minPressure->text().toFloat();
    int maxPressure_int = QString::number(maxPressure,'f',1).toFloat() * 10;
    int minPressure_int = QString::number(minPressure,'f',1).toFloat() * 10;


    appcore->setMaxAndMinPressure(maxPressure_int,minPressure_int);

    emit closeCurrentDialog(maxPressure_int,maxPressure_int);
    this->close();
}
