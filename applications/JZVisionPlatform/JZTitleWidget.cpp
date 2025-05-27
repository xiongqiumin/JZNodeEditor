#include "jztitlewidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

JZTitleWidget::JZTitleWidget(QWidget *parent) 
    :QWidget(parent)
{    
    m_image = QImage(":/JZMonitorSystem/Resources/icons/MonitorSystem.png");
}

JZTitleWidget::~JZTitleWidget()
{
}

void JZTitleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置字体
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    // 绘制背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    int h = height();
    painter.drawImage(QRect(0,0,h, h), m_image);

    QRect text_rc(h, 0, width() - h, height());
    painter.drawText(text_rc, "橘子低代码视觉平台", QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
}