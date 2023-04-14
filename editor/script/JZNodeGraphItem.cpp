#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include "JZNodeGraphItem.h"
#include "JZNodeView.h"

DispWidget::DispWidget()
{
    QFormLayout *layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    this->setMaximumWidth(90);       

    m_item = nullptr;
}

DispWidget::~DispWidget()
{
}

void DispWidget::setItem(JZNodeGraphItem *item)
{
    m_item = item;
}

JZNodeGraphItem *DispWidget::item()
{
    return m_item;
}

void DispWidget::clear()
{
    QFormLayout *l = (QFormLayout *)layout();
    if(l){
        delete l;
    }

    l = new QFormLayout();
    l->setContentsMargins(0, 0, 0, 0);
    this->setLayout(l);
    m_widgets.clear();
}

void DispWidget::addVariable(const JZNodePin &prop)
{
    QFormLayout *l = (QFormLayout *)layout();
    QLineEdit *line = new QLineEdit();
    l->addRow(new QLabel(prop.name()), line);
    connect(line, &QLineEdit::editingFinished, this, &DispWidget::onValueChanged);

    int param_id = m_item->editor()->paramId(m_item->id(),prop.id());
    m_widgets[param_id] = line;

    update();
}

QVariant DispWidget::getVariable(int id)
{
    if(!m_widgets.contains(id))
        return QVariant();

    auto widget = m_widgets[id];
    return ((QLineEdit *)widget)->text();
}

void DispWidget::setVariable(int id, QVariant value)
{
    if(!m_widgets.contains(id))
        return;

    auto widget = m_widgets[id];
    ((QLineEdit *)widget)->setText(value.toString());
}

void DispWidget::onValueChanged()
{
    QLineEdit *widget = (QLineEdit *)sender();
    int id = m_widgets.key(widget);
    QVariant value = widget->text();
    sigValueChanged(id, value);
}

// JZNodeGraphItem
JZNodeGraphItem::JZNodeGraphItem(JZNode *node)
{
    m_type = Item_node;
    m_node = node;
    m_id = node->id();

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);    

    m_proxy = new QGraphicsProxyWidget(this);
    m_dispWidget = new DispWidget();
    m_dispWidget->setItem(this);
    m_proxy->setWidget(m_dispWidget);
    m_proxy->setVisible(false);
}

JZNodeGraphItem::~JZNodeGraphItem()
{
}

void JZNodeGraphItem::setValue(int prop,QVariant value)
{
    m_dispWidget->setVariable(prop,value);
    update();
}

QRectF JZNodeGraphItem::boundingRect() const
{
    QRectF rc(0, 0, m_size.width(), m_size.height());
    return rc;
}

void JZNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{
    QRectF rc = boundingRect();
    QRectF title_rc = QRectF(rc.left(), rc.top(), rc.width(), 20);
    painter->fillRect(rc, Qt::red);

    QString title = m_node->name() + "(" + QString::number(m_node->id()) + ")";
    painter->fillRect(title_rc, QColor(192,192,192));
    painter->drawText(title_rc, title, QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));

    for (int i = 0; i < m_in.size(); i++)
    {
        QRectF icon_rc = QRectF(rc.left() + 4, title_rc.bottom() + i * 20 + 4, 16, 16);
        QRectF text_rc = QRectF(rc.left() + 20, title_rc.bottom() + i * 20, rc.width() - 20, 20);
        painter->fillRect(icon_rc, Qt::yellow);
        painter->drawText(text_rc, m_in[i], QTextOption(Qt::AlignVCenter | Qt::AlignLeft));
    }
    for (int i = 0; i < m_out.size(); i++)
    {
        QRectF icon_rc = QRectF(rc.right() - 20, title_rc.bottom() + i * 20 + 4, 16, 16);
        QRectF text_rc = QRectF(rc.left(), title_rc.bottom() + i * 20, rc.width() - 20, 20);
        painter->fillRect(icon_rc, Qt::yellow);
        painter->drawText(text_rc, m_out[i], QTextOption(Qt::AlignVCenter | Qt::AlignRight));
    }

    if(isSelected()){
        painter->setPen(QPen(Qt::yellow,4));
        painter->drawRect(rc);
    }
}

JZNode *JZNodeGraphItem::node()
{
    return m_node;
}

QRectF JZNodeGraphItem::propRect(int prop, int type)
{
    int index = m_node->indexOfPropByType(prop, type);
    QRectF rc = JZNodeGraphItem::boundingRect();
    int y = rc.top() + 20;
    if (type == Prop_in)
        return QRectF(rc.left() + 4, y + index * 20 + 4, 16, 16);
    else
        return QRectF(rc.right() - 20, y + index * 20 + 4, 16, 16);
}

DispWidget *JZNodeGraphItem::widget()
{
    return m_dispWidget;
}

void JZNodeGraphItem::updateNode()
{
    m_in.clear();
    m_out.clear();
    m_dispWidget->clear();
    m_proxy->setVisible(false);

    auto &list = m_node->propList();
    int disp_h = 0;
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i].flag() & Prop_in)
            m_in.push_back(list[i].name());
        if (list[i].flag() & Prop_out)
            m_out.push_back(list[i].name());
        if (list[i].flag() & Prop_disp)
        {
            disp_h += 20;                            
            m_dispWidget->addVariable(list[i]);
        }
    }

    int h = 20 + qMax(m_in.size(), m_out.size()) * 20;
    if (disp_h > 0)
    {        
        m_proxy->setGeometry(QRectF(5.0, h + 5.0, 90.0, disp_h));
        m_proxy->setVisible(true);
        h += 10 + disp_h;
    }

    h = qMax(h, 100);
    prepareGeometryChange();
    m_size = QSize(100, h);
}

void JZNodeGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    return JZNodeBaseItem::mouseMoveEvent(event);
}

void JZNodeGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto list = m_node->propList();
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i].flag() & Prop_out)
        {
            if (propRect(list[i].id(), Prop_out).contains(event->pos()))
            {
                event->ignore();

                JZNodeGemo gemo(m_node->id(), list[i].id());
                editor()->startLine(gemo);
                return;
            }
        }
    }
    return JZNodeBaseItem::mousePressEvent(event);
}

void JZNodeGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    return JZNodeBaseItem::mouseReleaseEvent(event);
}
