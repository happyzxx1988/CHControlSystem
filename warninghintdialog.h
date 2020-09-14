#ifndef WARNINGHINTDIALOG_H
#define WARNINGHINTDIALOG_H

#include <QDialog>

namespace Ui {
class WarningHintDialog;
}

class WarningHintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarningHintDialog(int num,bool a_, bool b_, bool c_, bool d_, bool e_, bool f_, QString operationType_, QWidget *parent = 0);
    ~WarningHintDialog();

signals:
    void closeCurrentDialog(bool a, bool b, bool c, bool d, bool e, bool f);
protected:
    void closeEvent(QCloseEvent *event);


private slots:
    void on_exitBtn_clicked();

private:
    Ui::WarningHintDialog *ui;
    int device_num;

    QString operationType;
    void init();
    bool a;
    bool b;
    bool c;
    bool d;
    bool e;
    bool f;
};

#endif // WARNINGHINTDIALOG_H
