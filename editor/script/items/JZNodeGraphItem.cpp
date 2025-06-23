#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <math.h>
#include <QPixmap>
#include <QPushButton>
#include "JZNodeGraphItem.h"
#include "JZNodeView.h"
#include "JZScriptEnvironment.h"
#include "JZIconManager.h"
#include "JZNodeFunction.h"

constexpr int name_max_width = 120;

JZNodeGraphItem::Block::Block(JZNodeGraphItem *item)
{
    this->item = item;
    iconType = Diamond;    
    proxy = nullptr;
    widget = nullptr;    
    clear();
}

JZNodeGraphItem::Block::~Block()
{
    clear();
}

bool JZNodeGraphItem::Block::isPin()
{
    return (id >= 0 && id < 100);
}

void JZNodeGraphItem::Block::setWidget(QWidget *w)
{
    Q_ASSERT(!widget);

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    proxy->setWidget(w);
    proxy->setParentItem(item);
    this->widget = w;
    this->proxy = proxy;    
}


void JZNodeGraphItem::Block::clearWidget()
{
    if (proxy)
    {
        delete proxy;
    }
    proxy = nullptr;
    widget = nullptr;
}

void JZNodeGraphItem::Block::clear()
{    
    clearWidget();
    iconRect = QRect();
    nameRect = QRect();
    valueRect = QRect(); //valueRect 就是 widget 显示范围

    pri = 0;
    id = -1;    
    isInput = false;
    isShowName = true;
    isShowValue = false;
    isEditable = false;
}

int JZNodeGraphItem::Block::width()
{    
    return iconRect.width() + nameRect.width() + valueRect.width();
}

int JZNodeGraphItem::Block::height()
{
    int h = qMax(iconRect.height(), nameRect.height());
    h = qMax(h, valueRect.height());
    return h;
}

// JZNodeGraphItem
JZNodeGraphItem::JZNodeGraphItem(JZNode *node)
    :JZAbstractNodeItem(node)
{    
    m_type = Item_node;
    m_node = node;
    m_id = node->id();
        
    m_downPin = -1;    
    m_blockExtId = 100;    
}

JZNodeGraphItem::~JZNodeGraphItem()
{
    clear();
}

JZNodeView *JZNodeGraphItem::nodeView()
{
    return qobject_cast<JZNodeView*>(editor());
}

JZNodeGraphItem::BlockPtr JZNodeGraphItem::createPinBlock(JZNodePin *pin)
{
    BlockPtr block = BlockPtr(new Block(this));
    block->id = pin->id();
    block->isInput = pin->isInput();
    block->isShowValue = pin->isParam() && pin->isInput();

    if (pin->isFlow() || pin->isSubFlow())
    {
        block->iconType = IconType::Flow;
        if (pin->isFlow())
            block->pri = Pri_flow;
        else
            block->pri = Pri_subFlow;
    }
    else
    {
        block->iconType = IconType::Circle;
        block->pri = Pri_flowParam;
    }
    if (pin->isInput())
        block->isEditable = true;

    block->name = pin->name();
    m_blocks[block->id] = block;
    return block;
}

JZNodeGraphItem::BlockPtr JZNodeGraphItem::createWidgetBlock(QWidget *widget, bool isInput)
{
    BlockPtr block = BlockPtr(new Block(this));
    block->isInput = isInput;

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    proxy->setWidget(widget);
    proxy->setParentItem(this);
    block->widget = widget;
    block->proxy = proxy;
    block->pri = Pri_widget;
    block->id = m_blockExtId++;
    m_blocks[block->id] = block;    
    return block;
}

JZNodeGraphItem::BlockPtr JZNodeGraphItem::createEditBlock(QString name, JZParamEditInfo info, bool isInput)
{
    BlockPtr block = BlockPtr(new Block(this));
    block->isInput = isInput;
    
    block->name = name;
    block->edit = info;
    block->isShowValue = true;
    block->isShowName = true;
    block->isEditable = true;
    block->pri = Pri_flowParam;
    block->id = m_blockExtId++;
    m_blocks[block->id] = block;
    return block;
}

JZNodeGraphItem::BlockPtr JZNodeGraphItem::createButtonBlock(QString name, std::function<void()> func, bool isInput)
{
    QPushButton *btnSet = new QPushButton(name);
    btnSet->connect(btnSet, &QPushButton::clicked, func);
    return createWidgetBlock(btnSet, isInput);    
}

void JZNodeGraphItem::clear()
{
    auto it = m_blocks.begin();
    while (it != m_blocks.end())
    {        
        it->clear();
        it++;
    }
    m_blocks.clear();
}

void JZNodeGraphItem::updatePin()
{
    m_title = m_node->name();

    //remove
    auto it = m_blocks.begin();
    while (it != m_blocks.end())
    {
        auto block = it->data();
        if (block->isPin() && !m_node->hasPin(it.key()) )
        {
            it->clear();
            it = m_blocks.erase(it);
        }
        else
            it++;
    }

    //add
    auto list = m_node->pinList();
    for (int i = 0; i < list.size(); i++)
    {
        int pin_id = list[i];
        if (!m_blocks.contains(pin_id))
            createPinBlock(m_node->pin(pin_id));
    }
}

void JZNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{    
    QRectF rc = boundingRect();
    QRectF title_rc = QRectF(rc.left(), rc.top(), rc.width(), 20);
    painter->fillRect(rc, QColor(192,192,192));        

    QTextOption text_opt;
    text_opt.setWrapMode(QTextOption::NoWrap);
    text_opt.setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    QString title = m_title;
    painter->fillRect(title_rc, QColor(220,220,220));
    painter->drawText(title_rc, title, text_opt);

    auto block_list = m_blocks.keys();
    for (int i = 0; i < block_list.size(); i++)
        drawProp(painter, block_list[i]);

    painter->save();
    if (nodeView()->runtimeNode() == m_id)
    {
        painter->setPen(QPen(Qt::green, 4));
        painter->drawRect(rc);
    }
    else if(isSelected())
    {
        painter->setPen(QPen(Qt::yellow,4));
        painter->drawRect(rc);
    }
    else
    {
        painter->setPen(QPen(QColor(128,128,128),1));
        painter->drawRect(rc.adjusted(0,0,-1,-1));
    }
    painter->restore();

    if(nodeView()->isBreakPoint(m_id))
    {
        QRect bt_rc = QRect(5,5,15,15);
        painter->save();
        painter->setBrush(Qt::red);
        painter->drawEllipse(bt_rc);
        painter->restore();
    }

    if(!m_error.isEmpty())
    {
        auto icon = JZIconManager::instance()->icon("iconExcl");
        painter->drawPixmap(m_errorRect, icon.pixmap(m_errorRect.size().toSize()), QRectF());
    }
}

int JZNodeGraphItem::pinAt(QPointF pos)
{    
    auto list = m_node->pinList();
    for (int i = 0; i < list.size(); i++)
    {                    
        auto rc = pinRect(list[i]);
        if (rc.contains(pos))
            return list[i];
    }
    return -1;    
}

int JZNodeGraphItem::pinAtInName(QPointF pos)
{
    auto list = m_node->pinList();
    for (int i = 0; i < list.size(); i++)
    {
        auto rc = pinRect(list[i]) | pinNameRect(list[i]);
        if (rc.contains(pos))
            return list[i];
    }
    return -1;
}

int JZNodeGraphItem::pinAtInValueRect(QPointF pos)
{
    auto it = m_blocks.begin();
    while (it != m_blocks.end())
    {
        QRectF rc = it->data()->valueRect;
        if (rc.contains(pos))
            return it.key();

        it++;
    }
    return -1;
}

QRectF JZNodeGraphItem::pinRect(int pin)
{
    return m_blocks[pin]->iconRect;
}

QRectF JZNodeGraphItem::pinNameRect(int pin)
{
    return m_blocks[pin]->nameRect;
}

bool JZNodeGraphItem::isPinEditable(int pin)
{    
    if (pin < 100 && !editor()->isPropEditable(m_id, pin))
        return false;

    return m_blocks[pin]->isEditable;
}

JZNodeGraphItem::Block *JZNodeGraphItem::block(int id)
{
    if (!m_blocks.contains(id))
        return nullptr;

    return m_blocks[id].data();
}

QByteArray JZNodeGraphItem::saveNode()
{
    auto node_factory = m_node->environment()->nodeFactory();
    return node_factory->saveNode(m_node);
}

QList<int> JZNodeGraphItem::blockList(bool isInput)
{
    QList<int> list;
    for (auto block : m_blocks)
    {
        if (block->isInput == isInput)
            list << block->id;
    }

    auto cmp = [this](int i, int j)->bool {
        if(m_blocks[i]->pri == m_blocks[j]->pri)
        {
            return m_blocks[i]->id < m_blocks[j]->id;
        }
        else
        {
            return m_blocks[i]->pri < m_blocks[j]->pri;
        }
    };
    std::sort(list.begin(), list.end(), cmp);
    return list;
}


JZNodePin *JZNodeGraphItem::pin(int pin_id)
{
    if (pin_id >= 100)
        return nullptr;

    return m_node->pin(pin_id);
}

void JZNodeGraphItem::calcGemo(int pin_id, int x, int y, Block *gemo)
{    
    gemo->iconRect = QRect(x, y, 24, 24);

    x = gemo->iconRect.right() + 5;
    if (gemo->isShowName)
    {
        QFontMetrics ft(scene()->font());
        int w = qMin(name_max_width, ft.horizontalAdvance(gemo->name));
        gemo->nameRect = QRect(x, y, w, 24);
        x = gemo->nameRect.right() + 5;
    }        
    if(gemo->widget)
    {
        gemo->valueRect = QRect(x, y, gemo->widget->width(), gemo->widget->height());
    }
    else if (gemo->isShowValue)
    {
        gemo->valueRect = QRect(x, y, 80, 24);
    }
}

void JZNodeGraphItem::updateNode()
{        
    updatePin();
    updateSize();    
}

void JZNodeGraphItem::setPinValue(int pin, QString value)
{
    if (pin < MAX_PIN_ID)
        update();
    else
        setBlockValue(pin, value);
}

QString JZNodeGraphItem::pinValue(int pin)
{
    if (pin < MAX_PIN_ID)
        return m_node->pinValue(pin);
    else
        return blockValue(pin);
}

void JZNodeGraphItem::setBlockValue(int pin, QString value)
{
    Q_ASSERT(0);
}

QString JZNodeGraphItem::blockValue(int pin)
{
    Q_ASSERT(0);
    return QString();
}

void JZNodeGraphItem::updateSize()
{        
    QFontMetrics title_ft(scene()->font());
    int title_w = title_ft.horizontalAdvance(m_title) + 20;

    int in_x = 0, out_x = 0;
    int in_y = 24, out_y = 24;
    int y_gap = 4;

    auto in_list = blockList(true);
    for (int i = 0; i < in_list.size(); i++)
    {
        auto &gemo = m_blocks[in_list[i]];
        calcGemo(in_list[i], 4, in_y, gemo.data());
        in_x = qMax(in_x, gemo->width());
        in_y += gemo->height() + y_gap;
    }

    QList<int> out_list = blockList(false);
    for (int i = 0; i < out_list.size(); i++)
    {
        auto &gemo = m_blocks[out_list[i]];
        calcGemo(out_list[i], 4, out_y, gemo.data());
        out_x = qMax(out_x, gemo->width());
        out_y += gemo->height() + y_gap;
    }

    int w = qMax(100, in_x + out_x + 30);
    w = qMax(title_w, w);

    int h = qMax(in_y, out_y);
    prepareGeometryChange();
    for (int i = 0; i < out_list.size(); i++)
    {
        auto &info = m_blocks[out_list[i]];
        info->iconRect.moveRight(w - 4);
        QRectF last = info->iconRect;
        if (!info->nameRect.isEmpty())
        {
            info->nameRect.moveRight(last.left() - 5);
            last = info->nameRect;
        }
        if (!info->valueRect.isEmpty())
        {
            info->valueRect.moveRight(last.left() - 5);
        }
    }

    for(auto b : m_blocks)
    {
        if (b->widget)
            b->widget->move(b->valueRect.topLeft());
    }

    m_size = QSize(w, qMax(h, 50));
    updateErrorGemo();
}

void JZNodeGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{    
    auto pin_id = pinAt(event->pos());
    auto pin_value_id = pinAtInValueRect(event->pos());

    bool no_move = false;
    if (pin_id >= 0)
    {        
        auto pin = m_node->pin(pin_id);
        if ((event->buttons() & Qt::LeftButton) && pin->isOutput() &&
            (pin->isParam() || pin->isFlow() || pin->isSubFlow()))
        {
            m_downPin = pin_id;
            no_move = true;
        }
        else
            m_downPin = -1;
    }   
    else if (pin_value_id >= 0)
    {
        auto block = m_blocks[pin_value_id];
        if (isPinEditable(pin_value_id))
        {
            no_move = true;
            nodeView()->editPinValue(m_id, pin_value_id);
        }
    }

    JZNodeBaseItem::mousePressEvent(event);

    if(no_move)
        setFlag(QGraphicsItem::ItemIsMovable, false);
}

void JZNodeGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{    
    if (event->buttons() & Qt::LeftButton)
    {               
        if (m_downPin != -1 && (event->pos() - m_blocks[m_downPin]->iconRect.center()).manhattanLength() > 10)
        {
            JZNodeGemo gemo(m_node->id(), m_downPin);
            editor()->startLine(gemo);
            m_downPin = -1;
        }
    }

    if(isSelected() && m_downPin == -1)
        JZNodeBaseItem::mouseMoveEvent(event);
}

void JZNodeGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{    
    m_downPin = -1;    
    
    JZNodeBaseItem::mouseReleaseEvent(event);

    setFlag(QGraphicsItem::ItemIsMovable, true);
}

void JZNodeGraphItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setZValue(m_baseZValue + 0.5);
    event->accept();
}

void JZNodeGraphItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{    
    event->accept();
}

void JZNodeGraphItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setZValue(m_baseZValue);
    event->accept();
}

void JZNodeGraphItem::notifyPropChanged(const QByteArray &buffer)
{
    editor()->addNodeChangedCommand(m_id, buffer);
}

void JZNodeGraphItem::updateErrorGemo()
{
    m_errorRect = QRectF();
    if(!m_error.isEmpty())
        m_errorRect = QRect(m_size.width() - 15 - 5,5,15,15);
}

QString JZNodeGraphItem::getTip(QPointF pt)
{
    if (m_errorRect.contains(pt))
        return m_error;

    int id = pinAtInName(pt);
    if (id != -1)
    {
        auto pin = m_node->pin(id);
        if(pin->isParam())
        {
            QString tips;
            if(pin->name().isEmpty())
                tips += "name: " + pin->name() + "\n";
            tips += "type: ";
            QStringList type_list;
            auto dataTypes = pin->dataType();
            for (int i = 0; i < dataTypes.size(); i++)
                type_list << dataTypes[i];
            tips += type_list.join(",");
            return tips;
        }
    }

    return QString();
}

void JZNodeGraphItem::setPinRuntimeValue(int pin_id, const QString &value)
{
    m_blocks[pin_id]->runTimeValue = value;
}

void JZNodeGraphItem::clearRuntimeValue()
{    
    m_runtimeValue.clear();
}

bool JZNodeGraphItem::isError() const
{
    return !m_error.isEmpty();
}

void JZNodeGraphItem::setError(const QString &error)
{
    m_error = error;
    updateErrorGemo();
    update();
}

void JZNodeGraphItem::clearError()
{
    m_error.clear();
    updateErrorGemo();
    update();
}

void JZNodeGraphItem::drawProp(QPainter *painter,int prop_id)
{        
    const Block *block = m_blocks[prop_id].data();
    auto pin = this->pin(block->id);

    /*
    case PinType::Flow:     return ImColor(255, 255, 255);
    case PinType::Bool:     return ImColor(220,  48,  48);
    case PinType::Int:      return ImColor( 68, 201, 156);
    case PinType::Float:    return ImColor(147, 226,  74);
    case PinType::String:   return ImColor(124,  21, 153);
    case PinType::Object:   return ImColor( 51, 150, 215);
    case PinType::Function: return ImColor(218,   0, 183);
    case PinType::Delegate: return ImColor(255,  48,  48);
    */
    QColor color;
    QColor innerColor = QColor(40, 40, 40, 80);
    IconType type = block->iconType;
    if(type == IconType::Flow)
    {
        color = QColor(255, 255, 255);        
        if (pin && pin->isSubFlow())
            innerColor = QColor(255, 223, 131, 80);
    }
    else if(type == IconType::Circle)
    {
        color = QColor(220,  48,  48);        
    }
    else
    {
        color = QColor(120, 48, 128);
    }
    
    drawIcon(painter, block->iconRect,type,false,color,innerColor);

    QTextOption text_opt;
    text_opt.setWrapMode(QTextOption::NoWrap);
    if(block->isShowName)
    {        
        auto opt = block->isInput? Qt::AlignLeft : Qt::AlignRight;
        text_opt.setAlignment(Qt::AlignVCenter | opt);
        painter->drawText(block->nameRect, block->name, text_opt);
    }
    bool show_value = (block->isShowValue || !block->runTimeValue.isEmpty());
    if (block->widget)
        show_value = false;
    if (show_value)
    {
        auto opt = block->isInput ? Qt::AlignLeft : Qt::AlignRight;
        text_opt.setAlignment(Qt::AlignVCenter | opt);
        painter->fillRect(block->valueRect, Qt::white);
        if(block->runTimeValue.size() > 0)
            painter->drawText(block->valueRect, block->runTimeValue, text_opt);
        else if(isPinEditable(prop_id))
            painter->drawText(block->valueRect, pinValue(prop_id), text_opt);
    }
}