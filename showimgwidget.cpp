#include "showimgwidget.h"
#include "qpainter.h"
#include "qevent.h"
#include "qtimer.h"
#include "qdebug.h"

ShowImgWidget::ShowImgWidget(QWidget *parent) : QWidget(parent)
{

}


void ShowImgWidget::paintEvent(QPaintEvent *)
{
    int width = this->width();
    int height = this->height();

    //绘制准备工作,启用反锯齿,平移坐标轴中心,等比例缩放
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//    painter.translate(width / 2, height / 2);

//    int side = qMin(width, height);
//    painter.scale(side / 200.0, side / 200.0);

    //绘制主背景图片
    drawMainImage(&painter);
    //绘制空压机
    drawCompressorImage(&painter);
    //绘制冷干机
    drawDryerImage(&painter);

}
void ShowImgWidget::drawMainImage(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    QImage img(":/images/images/g22.png");
    img = img.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //按照比例自动居中绘制
    int pixX = rect().x();
    int pixY = rect().y();
    QPoint point(pixX, pixY);
    painter->drawImage(point, img);
    painter->restore();
}

void ShowImgWidget::drawCompressorImage(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    QImage img(":/images/images/kyj.png");
    img = img.scaled(this->size() / 9, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //按照比例自动居中绘制
    int pixX = rect().center().x() - img.width() * 8.5;
    int pixY = rect().center().y() - img.height() * 2;
    QPoint point(pixX, pixY);
    painter->drawImage(point, img);
    painter->restore();
}
void ShowImgWidget::drawDryerImage(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    QImage img(":/images/images/lgj.png");
    img = img.scaled(this->size() / 9, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //按照比例自动居中绘制
    int pixX = rect().center().x() - img.width() / 2;
    int pixY = rect().center().y() - img.height() / 2;
    QPoint point(pixX, pixY);
    painter->drawImage(point, img);
    painter->restore();
}








