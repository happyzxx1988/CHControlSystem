#ifndef SHOWIMGWIDGET_H
#define SHOWIMGWIDGET_H

#include <QWidget>

class ShowImgWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShowImgWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *);
    void drawMainImage(QPainter *painter);
    void drawCompressorImage(QPainter *painter);
    void drawDryerImage(QPainter *painter);


signals:

public slots:
};

#endif // SHOWIMGWIDGET_H
