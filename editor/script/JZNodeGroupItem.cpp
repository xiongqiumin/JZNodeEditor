#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "JZNodeGroupItem.h"
#include "JZNodeView.h"

// JZNodeGroupItem
JZNodeGroupItem::JZNodeGroupItem(int id)
{
    m_type = Item_group;
    m_id = id;
    setZValue(-2);
    setOpacity(0.3);
    m_text = new QGraphicsTextItem(this);
    m_text->setFlag(ItemIgnoresParentOpacity);
    m_text->setPos(5, 0);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

JZNodeGroupItem::~JZNodeGroupItem()
{

}

QRectF JZNodeGroupItem::boundingRect() const
{
    return QRectF(QPointF(0,0),m_rect.size());
}

void JZNodeGroupItem::updateNode()
{
    prepareGeometryChange();

    auto group = editor()->file()->getGroup(m_id);
    m_text->setPlainText(group->memo);
    QSizeF text_size = m_text->document()->size();

    QList<int> node_list = editor()->file()->groupNodeList(m_id);
    if (node_list.size() == 0)
    {
        m_rect = QRectF();
        return;
    }

    for (int i = 0; i < node_list.size(); i++)
    {
        QRectF rc = editor()->getNodeItem(node_list[i])->sceneBoundingRect();
        if (i == 0)
            m_rect = rc;
        else
            m_rect = m_rect | rc;
    }

    double hMax = qMax(20.0, text_size.height());
    m_rect.adjust(-20, -hMax, 20, 20);

    setFlag(QGraphicsItem::ItemSendsGeometryChanges,false);
    setPos(m_rect.topLeft());
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,true);
}

void JZNodeGroupItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{    
    QRectF rc = boundingRect();

    painter->setPen(QPen(Qt::white));
    painter->setBrush(QBrush(Qt::green));
    painter->drawRoundRect(rc);
}
