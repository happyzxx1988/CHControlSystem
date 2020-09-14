#ifndef PRESSURESETDIALOG_H
#define PRESSURESETDIALOG_H

#include <QDialog>
#include "appcore.h"

namespace Ui {
class PressureSetDialog;
}

class PressureSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PressureSetDialog(QString user, AppCore *core = 0, QWidget *parent = 0);
    ~PressureSetDialog();

signals:
    void closeCurrentDialog(float maxPressure,float minPressure);

private slots:
    void on_setPressureBtn_clicked();

private:
    Ui::PressureSetDialog *ui;
    QString currentUser;
    AppCore *appcore;
};

#endif // PRESSURESETDIALOG_H
