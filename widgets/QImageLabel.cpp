#include <QPainter>
#include "QImageLabel.h"

QImageLabel::QImageLabel(QWidget *parent)
    :QWidget(parent)
{

}

QImageLabel::~QImageLabel()
{

}

void QImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::red);
}