#ifndef COMPRESSORSETDIALOG_H
#define COMPRESSORSETDIALOG_H

#include <QDialog>
#include "appcore.h"
#include "dataoper.h"

namespace Ui {
class CompressorSetDialog;
}

class CompressorSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompressorSetDialog(QString user,int type,AppCore *core = 0,QWidget *parent = 0);
    ~CompressorSetDialog();

signals:
    void closeCurrentDialog(int compressorNo,float uninstallPressure,float pressureDiff);

private slots:
    void on_setCompressorBtn_clicked();

private:
    Ui::CompressorSetDialog *ui;
    QString currentUser;
    int compressorNo;
    AppCore *appcore;
    DataOper dataOper;
};

#endif // COMPRESSORSETDIALOG_H
