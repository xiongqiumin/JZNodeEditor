#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>
#include "JZVisionNodeItem.h"
#include "JZVisionView.h"

JZVisionNodeItem::JZVisionNodeItem(JZNode *node)
    :JZAbstractNodeItem(node)
{
    m_downPin = -1;
    m_hovered = false;
    m_nodeName = node->name();
}

void JZVisionNodeItem::setNodeName(QString name)
{
    m_nodeName = name;
}

void JZVisionNodeItem::updateNode()
{
    m_blockList.clear();
    if (!m_node->isFlowNode())
    {
        m_size = QSize(0, 0);
        return;
    }

    JZVisionView* view = qobject_cast<JZVisionView*>(editor());
    
    //pin
    auto pin_list = m_node->pinList();
    for (int i = 0; i < pin_list.size(); i++)
    {
        auto pin = m_node->pin(pin_list[i]);
        if (pin->isFlow() || pin->isSubFlow())
        {
            Block block;
            block.id = pin_list[i];

            block.pri = pin->isSubFlow() ? 0 : 1;
            block.name = pin->name();
            m_blockList.push_back(block);
        }
    }
    std::sort(m_blockList.begin(), m_blockList.end(), [](const Block &b1, const Block &b2)
    {
        if (b1.pri != b2.pri)
            return b1.pri < b2.pri;
        else
            return b1.id < b2.id;
    });

    //calc gemo
    QString name = m_nodeName;
    int name_min_width = 160;
    int name_max_width = 320;

    QFontMetrics ft(scene()->font());
    int w = qMin(name_max_width, ft.horizontalAdvance(name) + 20);
    w = qMax(name_min_width, w);    

    int pin_name_max_width = 120;
    int x = 0;
    int in_y = 30;
    int out_y = 30;
    int y_gap = 30;    
    for (int i = 0; i < m_blockList.size(); i++)
    {
        int pin_id = m_blockList[i].id;
        auto pin = m_node->pin(pin_id);
        
        Block &block = m_blockList[i];
        block.id = pin_id;
        block.name = pin->name();

        QFontMetrics ft(scene()->font());
        int text_w = qMin(pin_name_max_width, ft.horizontalAdvance(block.name));

        if (pin->isInput())
        {
            x = 2;
            block.iconRect = QRect(x, in_y, 24, 24);
            block.nameRect = QRect(x + text_w + 4, in_y, text_w, 24);
            in_y += y_gap;
        }
        else
        {
            x = w - 2 - 24;
            block.iconRect = QRect(x, out_y, 24, 24);
            block.nameRect = QRect(x - text_w - 4, out_y, text_w, 24);
            out_y += y_gap;
        }
    }

    int h = qMax(in_y, out_y);
    m_size = QSize(w, h);
}

int JZVisionNodeItem::indexOfBlock(int pin_id)
{
    for (int i = 0; i < m_blockList.size(); i++)
    {
        if (m_blockList[i].id == pin_id)
            return i;
    }

    return -1;
}

int JZVisionNodeItem::pinAt(QPointF pos)
{
    for (int i = 0; i < m_blockList.size(); i++)
    {
        QRectF rc = m_blockList[i].iconRect;
        if (rc.contains(pos))
            return m_blockList[i].id;
    }

    return -1;
}

QRectF JZVisionNodeItem::pinRect(int pin_id)
{
    int index = indexOfBlock(pin_id);
    return m_blockList[index].iconRect;
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

void JZVisionNodeItem::drawBlock(QPainter *painter, const Block &block)
{    
    auto pin = m_node->pin(block.id);

    QColor color = QColor(255, 255, 255);
    QColor innerColor = QColor(40, 40, 40, 80);    
    if (pin->isSubFlow())
        innerColor = QColor(255, 223, 131, 80);

    drawIcon(painter, block.iconRect, IconType::Flow, false, color, innerColor);

    QTextOption text_opt;
    text_opt.setWrapMode(QTextOption::NoWrap);
    if (pin->isSubFlow())
    {
        auto opt = pin->isInput() ? Qt::AlignLeft : Qt::AlignRight;
        text_opt.setAlignment(Qt::AlignVCenter | opt);
        painter->drawText(block.nameRect, block.name, text_opt);
    }
}

void JZVisionNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!m_node->isFlowNode())
        return;

    JZVisionView *view = qobject_cast<JZVisionView*>(editor());

    QRect rc = boundingRect().toRect();
    painter->fillRect(rc, QColor(192, 192, 192));

    int w = (int)rc.width();
    int h = (int)rc.height();    

    int icon_w = 30;
    painter->fillRect(QRect(0, 0, rc.width(), icon_w), QColor(220, 220, 220));    
    if (isError())
    {
        painter->fillRect(QRect(0, 0, icon_w, icon_w), Qt::red);
    }
    else
    {
        painter->fillRect(QRect(0, 0, icon_w, icon_w), Qt::green);
    }   

    for (int i = 0; i < m_blockList.size(); i++)
        drawBlock(painter, m_blockList[i]);

    QRect text_rc(icon_w, 0,(int)(rc.width() - icon_w), icon_w);
    painter->drawText(text_rc, m_nodeName, QTextOption(Qt::AlignCenter));
    if (isSelected())
    {
        painter->setPen(QPen(QColor(250, 156, 62) , 2));
        painter->drawRect(rc);
    }
    else
    {
        painter->setPen(QPen(Qt::black, 2));
        painter->drawRect(rc);
    }    
}