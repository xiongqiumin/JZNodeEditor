#include <QPainter>
#include "QImageLabel.h"

QImageLabel::QImageLabel(QWidget *parent)
    :QWidget(parent)
{

}

QImageLabel::~QImageLabel()
{

}

QSize QImageLabel::sizeHint() const
{
    return QSize(100, 100);
}

void QImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::red);
}