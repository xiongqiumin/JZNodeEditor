#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include "JZPanelWidget.h"

JZPanelWidget::JZPanelWidget(QWidget *parent) 
    : QWidget(parent)
{
    m_handleIndex = -1;
    m_preHeight = m_curHeight = 0;
    m_handleTimer = new QTimer(this);    
    connect(m_handleTimer, &QTimer::timeout, this, &JZPanelWidget::onHandleTimer);
    
    setMouseTracking(true);
}

JZPanelWidget::~JZPanelWidget()
{
}

void JZPanelWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QTimer::singleShot(0, [this] {
        updateLayout();
    });
}

void JZPanelWidget::enterEvent(QEvent *event)
{    
    QWidget::enterEvent(event);
    m_handleTimer->start(50);
}

void JZPanelWidget::leaveEvent(QEvent *event)
{    
    m_handleTimer->stop();
    setCursor(Qt::ArrowCursor);
    QWidget::leaveEvent(event);    

}
void JZPanelWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = resizeHandleIndex(event->y());
        if (idx != -1)
        {
            m_handleIndex = idx;
            m_preHeight = m_panelList[m_handleIndex - 1].height;
            m_curHeight = m_panelList[m_handleIndex].height;
            m_downPoint = event->pos();
        }
    }
}

void JZPanelWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 检查是否在调整区域内    
    if (m_handleIndex != -1)
    {
        int gap = event->pos().y() - m_downPoint.y();
        auto pre = &m_panelList[m_handleIndex - 1];
        auto cur = &m_panelList[m_handleIndex];
        int w = width();   
        int total = m_preHeight + m_curHeight;
        pre->height = m_preHeight + gap;
        cur->height = m_curHeight - gap;
        if (pre->height < 50)
        {
            pre->height = 50;
            cur->height = total - pre->height;
        }
        if (cur->height < 50)
        {
            cur->height = 50;
            pre->height = total - cur->height;
        }

        updateLayout();
    } 
}

void JZPanelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_handleIndex = -1;    
}

void JZPanelWidget::addTab(const QString &title, QWidget *insert_widget)
{
    // 创建切换按钮
    auto btn = new QToolButton();
    btn->setToolTip("隐藏/显示面板");
    btn->setArrowType(Qt::DownArrow);    
    connect(btn, &QToolButton::clicked, this, &JZPanelWidget::onToggleButtonClicked);

    QLabel *list_title = new QLabel(title);    

    QWidget *panel_widget = new QWidget();
    QWidget *panel_title_widget = new QWidget();
    panel_title_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panel_title_widget->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");

    QHBoxLayout *title_layout = new QHBoxLayout(panel_title_widget);
    QVBoxLayout *panel_layout = new QVBoxLayout(panel_widget);
    title_layout->setContentsMargins(0, 0, 0, 0);
    title_layout->setSpacing(2);
    title_layout->addWidget(btn);
    title_layout->addWidget(list_title);

    panel_layout->setContentsMargins(0, 0, 0, 0);
    panel_layout->setSpacing(0);
    panel_layout->addWidget(panel_title_widget);
    panel_layout->addWidget(insert_widget);    

    panel_widget->setParent(this);
    panel_widget->show();    

    PanelInfo info;
    info.panel = panel_widget;
    info.btn = btn;    
    info.widget = insert_widget;    
    info.height = 200;
    m_panelList.push_back(info);
    updateLayout();
}

void JZPanelWidget::onToggleButtonClicked()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    int idx = -1;
    for (int i = 0; i < m_panelList.size(); i++)
    {
        if (m_panelList[i].btn == btn)
        {
            idx = i;
            break;
        }
    }

    auto &panel = m_panelList[idx];
    panel.widget->setVisible(!panel.widget->isVisible());
    if (panel.widget->isVisible()) 
    {
        int h = height();
        h -= (30 + 4) * m_panelList.size();

        QList<int> visible_idx;
        for (int i = 0; i < m_panelList.size(); i++)
        {
            if (m_panelList[i].widget->isVisible())
                visible_idx << i;
        }

        for (int i = 0; i < visible_idx.size(); i++)
        {
            int p_idx = visible_idx[i];
            m_panelList[i].height = h / visible_idx.size();
        }
        panel.btn->setArrowType(Qt::DownArrow);
    }
    else 
    {        
        panel.btn->setArrowType(Qt::RightArrow);
    }
    updateLayout();
}

void JZPanelWidget::onHandleTimer()
{
    if (m_handleIndex != -1)
        return;

    QPoint p = mapFromGlobal(QCursor::pos());
    int idx = resizeHandleIndex(p.y());
    if (idx != -1)
        setCursor(Qt::SizeVerCursor);
    else
        setCursor(Qt::ArrowCursor);
}

int JZPanelWidget::resizeHandleIndex(int y)
{
    for (int i = 1; i < m_panelList.size(); i++)
    {
        PanelInfo *pre_p = &m_panelList[i - 1];
        PanelInfo *p = &m_panelList[i];        
        if (pre_p->widget->isVisible() && p->widget->isVisible()
            && abs(y - p->panel->y()) < 5)
            return i;
    }
    return -1;
}

void JZPanelWidget::updateLayout()
{
    int title_h = 30;
    int gap = 4;

    int y = 0;
    int w = width();
    int last_visible = -1;
    int last_y = height();

    for (int i = m_panelList.size() - 1; i >= 0; i--)
    {
        PanelInfo *p = &m_panelList[i];
        if (p->widget->isVisible())
        {
            last_visible = i;
            break;
        }
        last_y -= (title_h + gap);
    }

    for (int i = 0; i < m_panelList.size(); i++)
    {
        PanelInfo *p = &m_panelList[i];

        int h = title_h;
        if (p->widget->isVisible())
            h += p->height;
        if (i == last_visible)
        {
            h = last_y - y;
            p->height = h;
        }
        
        p->panel->setGeometry(0, y, w, h);        
        y += h + gap;        
    }
}