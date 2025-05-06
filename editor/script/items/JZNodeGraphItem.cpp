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
JZNodeGraphItem::JZNodeGraphItem()
{    
    m_type = Item_node;    
    m_longPress = false;
    m_downPin = -1;
    m_node = nullptr;
    m_id = -1;
    m_widgetIndex = 100;

    setOpacity(0.9);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);    
}

JZNodeGraphItem::~JZNodeGraphItem()
{
    clear();
}

void JZNodeGraphItem::init(JZNode *node)
{
    m_node = node;
    m_id = node->id();
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
        if (pin->isInput())
            block->pri = 0;
        else
        {
            if (m_node->subFlowCount() > 0)
                block->pri = pin->isSubFlow()? 0:2;
            else
                block->pri = 0;
        }
    }
    else
    {
        block->iconType = IconType::Circle;
        block->pri = 1;        
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
    block->id = m_widgetIndex++;
    m_blocks[block->id] = block;
    return block;
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

QRectF JZNodeGraphItem::boundingRect() const
{
    QRectF rc(0, 0, m_size.width(), m_size.height());
    return rc;
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
    if (editor()->runtimeNode() == m_id)
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

    if(editor()->isBreakPoint(m_id))
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

JZNode *JZNodeGraphItem::node()
{
    return m_node;
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
    bool flag;
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

QSize JZNodeGraphItem::size() const
{
    return m_size;
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
    else if (gemo->isPin() && pin(pin_id)->isParam())
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
    if (pin_id >= 0)
    {        
        auto pin = m_node->pin(pin_id);
        if ((event->buttons() & Qt::LeftButton) && pin->isOutput() && 
            (pin->isParam() || pin->isFlow() || pin->isSubFlow()))
            m_downPin = pin_id;
        else
            m_downPin = -1;
    }   
    else if (pin_value_id >= 0)
    {
        auto block = m_blocks[pin_value_id];
        if (isPinEditable(pin_value_id))
            editor()->editPinValue(m_id, pin_value_id);
    }
    else
    {
        m_longPress = 1;
        editor()->setNodeTimer(500,m_node->id(),Timer_longPress);
    }
    return JZNodeBaseItem::mousePressEvent(event);
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
    m_longPress = 0;
    m_downPin = -1;
    return JZNodeBaseItem::mouseReleaseEvent(event);
}

void JZNodeGraphItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setZValue(0.5);
    event->accept();
}

void JZNodeGraphItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{    
    event->accept();
}

void JZNodeGraphItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setZValue(0);
    event->accept();
}

void JZNodeGraphItem::notifyPropChanged(const QByteArray &buffer)
{
    editor()->onNodeChanged(m_id, buffer);
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

void JZNodeGraphItem::onTimerEvent(int event)
{
    if(event == Timer_longPress)
    {
        if (m_longPress == 1)
            m_longPress = 2;
    }
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
    if (block->isShowValue && pin && pin->isParam())
    {
        auto opt = block->isInput ? Qt::AlignLeft : Qt::AlignRight;
        text_opt.setAlignment(Qt::AlignVCenter | opt);
        painter->fillRect(block->valueRect, Qt::white);
        painter->drawText(block->valueRect, pin->value(), text_opt);
    }
}

void JZNodeGraphItem::drawIcon(QPainter *painter,QRectF rect, IconType type, bool filled, QColor color, QColor innerColor)
{    
    auto rect_x         = rect.x();
    auto rect_y         = rect.y();
    auto rect_w         = rect.width();
    auto rect_h         = rect.height();
    auto rect_center_x  = rect.center().x();
    auto rect_center_y  = rect.center().y();
    auto rect_center    = QPoint(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

    QPainterPath path;
    auto AddCircle = [painter](QPointF center,double r,QColor c,int flag,double thickness)
    {
        painter->setPen(QPen(c,thickness));
        painter->drawEllipse(center,r,r);
    };
    auto AddCircleFilled  = [painter](QPointF center,double r,QColor c,int)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->drawEllipse(center,r,r);
    };
    auto AddRect = [painter](QPointF p1,QPointF p2,QColor c,double round,int flag,double thickness)
    {
        painter->setPen(QPen(c,thickness));
        painter->drawRect(QRectF(p1,p2));
    };
    auto AddRectFilled = [painter](QPointF p1,QPointF p2,QColor c,int a=0,int b=0)
    {
        painter->fillRect(QRectF(p1,p2),c);
    };
    auto AddConvexPolyFilled = [painter](const QPainterPath &poly_path,QColor c)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->drawPath(poly_path);
    };
    auto AddTriangleFilled = [painter](QPointF p1,QPointF p2,QPointF p3,QColor c)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        QPolygonF polygon;
        polygon << p1 << p2 << p3;
        painter->drawPolygon(polygon);
    };

    auto PathBezierCubicCurveTo = [&path](QPointF p1,QPointF p2,QPointF p3){ path.cubicTo(p1,p2,p3); };
    auto PathLineTo = [&path](QPointF pt)
    {
        if(path.elementCount() == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    };
    auto PathFillConvex = [painter,&path](QColor c)
    {
        painter->fillPath(path,c);
    };
    auto PathStroke = [painter,&path](QColor c,bool flag,double lineSize)
    {
        path.closeSubpath();
        painter->setPen(QPen(c,lineSize));
        painter->drawPath(path);
    };

    painter->save();
    if (type == IconType::Flow)
    {
        const auto origin_scale = rect_w / 24.0f;

        const auto offset_x  = 1.0f * origin_scale;
        const auto offset_y  = 0.0f * origin_scale;
        const auto margin     = (filled ? 2.0f : 2.0f) * origin_scale;
        const auto rounding   = 0.1f * origin_scale;
        const auto tip_round  = 0.7f; // percentage of triangle edge (for tip)
        //const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
        const auto canvas = QRectF(
            QPointF(rect.x() + margin + offset_x,
                rect.y() + margin + offset_y),
            QPointF(rect.bottomRight().x() - margin + offset_x,
                rect.bottomRight().y() - margin + offset_y));
        const auto canvas_x = canvas.x();
        const auto canvas_y = canvas.y();
        const auto canvas_w = canvas.bottomRight().x() - canvas.x();
        const auto canvas_h = canvas.bottomRight().y() - canvas.y();

        const auto left   = canvas_x + canvas_w            * 0.5f * 0.3f;
        const auto right  = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
        const auto top    = canvas_y + canvas_h            * 0.5f * 0.2f;
        const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
        const auto center_y = (top + bottom) * 0.5f;
        //const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

        const auto tip_top    = QPointF(canvas_x + canvas_w * 0.5f, top);
        const auto tip_right  = QPointF(right, center_y);
        const auto tip_bottom = QPointF(canvas_x + canvas_w * 0.5f, bottom);

        PathLineTo(QPointF(left, top) + QPointF(0, rounding));
        PathBezierCubicCurveTo(
            QPointF(left, top),
            QPointF(left, top),
            QPointF(left, top) + QPointF(rounding, 0));
        PathLineTo(tip_top);
        PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
        PathBezierCubicCurveTo(
            tip_right,
            tip_right,
            tip_bottom + (tip_right - tip_bottom) * tip_round);
        PathLineTo(tip_bottom);
        PathLineTo(QPointF(left, bottom) + QPointF(rounding, 0));
        PathBezierCubicCurveTo(
            QPointF(left, bottom),
            QPointF(left, bottom),
            QPointF(left, bottom) - QPointF(0, rounding));

        if (!filled)
        {
            if (innerColor.rgb() & 0xFF000000)
                AddConvexPolyFilled(path, innerColor);

            PathStroke(color, true, 2.0f * outline_scale);
        }
        else
            PathFillConvex(color);
    }
    else
    {
        auto triangleStart = rect_center_x + 0.32f * rect_w;
        auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

        rect.moveLeft(rect.x() + rect_offset);
        rect_x        += rect_offset;
        rect_center_x += rect_offset * 0.5f;
        rect_center.rx() += rect_offset * 0.5f;

        if (type == IconType::Circle)
        {
            const auto c = rect_center;

            if (!filled)
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;

                if (innerColor.rgb() & 0xFF000000)
                    AddCircleFilled(c, r, innerColor, 12 + extra_segments);
                AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
            }
            else
            {
                AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
            }
        }

        if (type == IconType::Square)
        {
            if (filled)
            {
                const auto r  = 0.5f * rect_w / 2.0f;
                const auto p0 = rect_center - QPointF(r, r);
                const auto p1 = rect_center + QPointF(r, r);
                AddRectFilled(p0, p1, color, 0, 15);
            }
            else
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto p0 = rect_center - QPointF(r, r);
                const auto p1 = rect_center + QPointF(r, r);

                if (innerColor.rgb() & 0xFF000000)
                {
                    AddRectFilled(p0, p1, innerColor, 0, 15);
                }
                AddRect(p0, p1, color, 0, 15, 2.0f * outline_scale);
            }
        }

        if (type == IconType::Grid)
        {
            const auto r = 0.5f * rect_w / 2.0f;
            const auto w = ceilf(r / 3.0f);

            const auto baseTl = QPointF(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
            const auto baseBr = QPointF(floorf(baseTl.x() + w), floorf(baseTl.y() + w));

            auto tl = baseTl;
            auto br = baseBr;
            for (int i = 0; i < 3; ++i)
            {
                tl.rx() = baseTl.x();
                br.rx() = baseBr.x();
                AddRectFilled(tl, br, color);
                tl.rx() += w * 2;
                br.rx() += w * 2;
                if (i != 1 || filled)
                    AddRectFilled(tl, br, color);
                tl.rx() += w * 2;
                br.rx() += w * 2;
                AddRectFilled(tl, br, color);

                tl.ry() += w * 2;
                br.ry() += w * 2;
            }

            triangleStart = br.x() + w + 1.0f / 24.0f * rect_w;
        }

        if (type == IconType::RoundSquare)
        {
            if (filled)
            {
                const auto r  = 0.5f * rect_w / 2.0f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - QPointF(r, r);
                const auto p1 = rect_center + QPointF(r, r);

                AddRectFilled(p0, p1, color, cr, 15);
            }
            else
            {
                const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                const auto cr = r * 0.5f;
                const auto p0 = rect_center - QPointF(r, r);
                const auto p1 = rect_center + QPointF(r, r);

                if (innerColor.rgb() & 0xFF000000)
                {
                    AddRectFilled(p0, p1, innerColor, cr, 15);
                }
                AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
            }
        }
        else if (type == IconType::Diamond)
        {
            if (filled)
            {
                const auto r = 0.607f * rect_w / 2.0f;
                const auto c = rect_center;

                PathLineTo(c + QPointF( 0, -r));
                PathLineTo(c + QPointF( r,  0));
                PathLineTo(c + QPointF( 0,  r));
                PathLineTo(c + QPointF(-r,  0));
                PathFillConvex(color);
            }
            else
            {
                const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                const auto c = rect_center;

                PathLineTo(c + QPointF( 0, -r));
                PathLineTo(c + QPointF( r,  0));
                PathLineTo(c + QPointF( 0,  r));
                PathLineTo(c + QPointF(-r,  0));

                if (innerColor.rgb() & 0xFF000000)
                    AddConvexPolyFilled(path,innerColor);

                PathStroke(color, true, 2.0f * outline_scale);
            }
        }
        else
        {
            const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

            AddTriangleFilled(
                QPointF(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                QPointF(triangleStart, rect_center_y + 0.15f * rect_h),
                QPointF(triangleStart, rect_center_y - 0.15f * rect_h),
                color);
        }
    }
    painter->restore();
}

//JZNodeFunctionItem
JZNodeFunctionItem::JZNodeFunctionItem()
{
}

void JZNodeFunctionItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    JZNodeFunction *node_func = dynamic_cast<JZNodeFunction*>(m_node);
    m_title = node_func->function();

    auto env = m_node->environment();
    auto func_inst = env->functionManager();
    auto meta = func_inst->function(m_title);
    if (meta && meta->isMemberFunction() && !node_func->isDirectCall())
    {
        QString v = node_func->variable();
        if (v.isEmpty())
        {
            if (!node_func->isMemberCall())
                return;

            v = "this";
        }

        QString name = v + "." + meta->name;
        m_title = name;
    }
}