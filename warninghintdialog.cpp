#include "warninghintdialog.h"
#include "ui_warninghintdialog.h"

WarningHintDialog::WarningHintDialog(int num, bool a_, bool b_, bool c_, bool d_, bool e_, bool f_, QString operationType_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarningHintDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->operationType = operationType_;
    this->a = a_;
    this->b = b_;
    this->c = c_;
    this->d = d_;
    this->e = e_;
    this->f = f_;
    this->device_num = num;
    ui->exitBtn->hide();

    switch (device_num) {
    case 2:
        ui->stopCompressor3->hide();
        ui->stopDryer3->hide();
        break;
    }




    this->init();
}

WarningHintDialog::~WarningHintDialog()
{
    delete ui;
}

void WarningHintDialog::closeEvent(QCloseEvent *event)
{
    bool a = ui->stopCompressor1->isChecked();
    bool b = ui->stopCompressor2->isChecked();
    bool c = ui->stopCompressor3->isChecked();
    bool d = ui->stopDryer1->isChecked();
    bool e = ui->stopDryer2->isChecked();
    bool f = ui->stopDryer3->isChecked();
    emit closeCurrentDialog(a,b,c,d,e,f);
}
void WarningHintDialog::init()
{
    ui->textEdit->setText(operationType);
    ui->stopCompressor1->setChecked(a);
    ui->stopCompressor2->setChecked(b);
    ui->stopCompressor3->setChecked(c);
    ui->stopDryer1->setChecked(d);
    ui->stopDryer2->setChecked(e);
    ui->stopDryer3->setChecked(f);
}

void WarningHintDialog::on_exitBtn_clicked()
{
    qApp->exit();
}
