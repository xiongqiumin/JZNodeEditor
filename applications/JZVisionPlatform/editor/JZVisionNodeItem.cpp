#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>
#include "JZVisionNodeItem.h"
#include "JZNodeAbstractView.h"

JZVisionNodeItem::JZVisionNodeItem(JZNode *node)
    :JZAbstractNodeItem(node)
{
    m_downPin = -1;
    m_hovered = false;
}

void JZVisionNodeItem::updateNode()
{
    QString name = m_node->name();
    int name_min_width = 160;
    int name_max_width = 320;

    QFontMetrics ft(scene()->font());
    int w = qMin(name_max_width, ft.horizontalAdvance(name) + 20);
    w = qMax(name_min_width, w);
    
    m_size = QSize(w, 40);
}

int JZVisionNodeItem::pinAt(QPointF pos)
{
    return -1;
}

QRectF JZVisionNodeItem::pinRect(int pin)
{
    return QRect();
}

QString JZVisionNodeItem::getTip(QPointF pt)
{
    int w = (int)boundingRect().height();
    QRectF icon_rc(0, 0, w, w);

    if (isError() && icon_rc.contains(pt))
        return m_error;
    else
        return QString();
}

QList<QRect> JZVisionNodeItem::hoverList()
{
    int icon_w = 10;

    QRect rc = boundingRect().toRect();
    int w = rc.width();
    int h = rc.height();
    int x = (w - icon_w) / 2;
    int y = (h - icon_w) / 2;

    QList<QRect> ret;
    ret << QRect(0, y, icon_w, icon_w);
    ret << QRect(x, 0, icon_w, icon_w);
    ret << QRect(w - icon_w, y, icon_w, icon_w);
    ret << QRect(x, h - icon_w, icon_w, icon_w);

    return ret;
}

void JZVisionNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverEnterEvent(event);
    setZValue(m_baseZValue + 0.5);

    m_hovered = true;
    update(); // 触发重绘    
}

void JZVisionNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverLeaveEvent(event);
    setZValue(m_baseZValue);

    m_hovered = false;
    update(); // 触发重绘    
}

void JZVisionNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto pin_id = pinAt(event->pos());
    QList<QRect> hover_list = hoverList();
    for (int i = 0; i < hover_list.size(); i++)
    {
        if (hover_list[i].contains(event->pos().toPoint()))
        {
            pin_id = m_node->flowOut();
            break;
        }
    }
   
    bool no_move = false;
    if (pin_id >= 0)
    {
        auto pin = m_node->pin(pin_id);
        if ((event->buttons() & Qt::LeftButton) && pin->isOutput() &&
            (pin->isParam() || pin->isFlow() || pin->isSubFlow()))
        {
            m_downPin = pin_id;
            m_downPoint = event->pos();
            no_move = true;
        }
    }
    else
    {
        m_downPin = -1;
    }

    m_hovered = false;
    JZNodeBaseItem::mousePressEvent(event);
    if (no_move)
        setFlag(QGraphicsItem::ItemIsMovable, false);

    update();
}

void JZVisionNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{    
    if (event->buttons() & Qt::LeftButton)
    {
        if (m_downPin != -1 && (event->pos() - m_downPoint).manhattanLength() > 10)
        {
            JZNodeGemo gemo(m_node->id(), m_downPin);
            editor()->startLine(gemo);
            m_downPoint = QPointF();
            m_downPin = -1;
        }
    }

    if (isSelected() && m_downPin == -1)
        JZNodeBaseItem::mouseMoveEvent(event);
}

void JZVisionNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_downPin = -1;    
    m_downPoint = QPointF();
    JZNodeBaseItem::mouseReleaseEvent(event);
    setFlag(QGraphicsItem::ItemIsMovable, true);    
}

void JZVisionNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRect rc = boundingRect().toRect();
    painter->fillRect(rc, Qt::white);
    painter->fillRect(rc, Qt::white);

    int w = (int)rc.width();
    int h = (int)rc.height();    

    int icon_w = h;
    if (isError())
    {
        painter->fillRect(QRect(0, 0, icon_w, (int)rc.height()), Qt::red);
    }
    else
    {
        painter->fillRect(QRect(0, 0, icon_w, (int)rc.height()), Qt::green);
    }

    if (m_hovered)
    {
        QList<QRect> list = hoverList();
        for (int i = 0; i < list.size(); i++)
            painter->fillRect(list[i], QColor(250, 156, 62));
    }

    QRect text_rc(icon_w, 0,(int)(rc.width() - icon_w),(int)rc.height());
    painter->drawText(text_rc, m_node->name(), QTextOption(Qt::AlignCenter));
    if (isSelected())
    {
        painter->setPen(QPen(Qt::yellow, 3));
        painter->drawRect(rc);
    }
    else
    {
        painter->setPen(QPen(Qt::black, 3));
        painter->drawRect(rc);
    }    
}