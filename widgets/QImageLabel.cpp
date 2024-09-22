#include <QPainter>
#include "QImageLabel.h"

QImageLabel::QImageLabel(QWidget *parent)
    :QWidget(parent)
{

}

QImageLabel::~QImageLabel()
{

}

QImage QImageLabel::image()
{
    return m_image;
}

void QImageLabel::setImage(QImage image)
{
    m_image = image;
}

QSize QImageLabel::sizeHint() const
{
    return QSize(100, 100);
}

void QImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    if(!m_image.isNull())
    {
        auto draw_size = m_image.size().scaled(size(), Qt::KeepAspectRatio);
        QRect rc((width() - draw_size.width()) / 2, (height() - draw_size.height()) / 2, draw_size.width(), draw_size.height());
        painter.drawImage(rc, m_image);
    }
}