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
}

void JZVisionNodeItem::updateNode()
{
    QString name = m_node->name();
    int name_min_width = 100;
    int name_max_width = 200;

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

void JZVisionNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto pin_id = pinAt(event->pos());
    if ((event->modifiers() & Qt::ControlModifier))
        pin_id = m_node->flowOut();

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

    JZNodeBaseItem::mousePressEvent(event);
    if (no_move)
        setFlag(QGraphicsItem::ItemIsMovable, false);
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
    QRectF rc = boundingRect();
    painter->fillRect(rc, Qt::white);
    painter->fillRect(rc, Qt::white);

    painter->drawText(rc, m_node->name(), QTextOption(Qt::AlignCenter));
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