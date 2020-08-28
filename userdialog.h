#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include "dataoper.h"

namespace Ui {
class UserDialog;
}

class UserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserDialog(QString currentUser,QString u, QString p, int flag, QWidget *parent = 0);
    ~UserDialog();

private slots:
    void on_saveUserBtn_clicked();

signals:
    void closeCurrentDialog(int userEditType);

private:
    Ui::UserDialog *ui;
    QString userName;
    QString pwd;
    int flag_;//1新增   2，修改
    QString currentUser_;
    DataOper dataOper;

};

#endif // USERDIALOG_H
