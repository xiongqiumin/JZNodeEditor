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
#include "JZNodeGraphItem.h"
#include "JZNodeView.h"

enum {
    Widget_none,
    Widget_lineEdit,    
    Widget_comboBox,
};

constexpr int name_max_width = 120;

//JZNodeGraphItem::PropGemo
JZNodeGraphItem::PropGemo::PropGemo()
{
    widgetType = Widget_none;
    proxy = nullptr;
    widget = nullptr;    
}

JZNodeGraphItem::PropGemo::~PropGemo()
{
    Q_ASSERT(!proxy && !widget);
}

void JZNodeGraphItem::PropGemo::clear()
{
    if (proxy)
    {
        delete proxy;
    }
    widgetType = 0;
    proxy = nullptr;
    widget = nullptr;
}

int JZNodeGraphItem::PropGemo::width()
{    
    return iconRect.width() + nameRect.width() + valueRect.width();
}

int JZNodeGraphItem::PropGemo::height()
{
    int h = qMax(iconRect.height(), nameRect.height());
    h = qMax(h, valueRect.height());
    return h;
}

// JZNodeGraphItem
JZNodeGraphItem::JZNodeGraphItem(JZNode *node)
{
    m_type = Item_node;
    m_node = node;
    m_id = node->id();    
    m_longPress = false;
    m_downPin = -1;

    setOpacity(0.9);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    
    updatePropGemo();
}

JZNodeGraphItem::~JZNodeGraphItem()
{
    auto it = m_pinRects.begin();
    while (it != m_pinRects.end())
    {
        it->clear();
        it++;
    }
    m_pinRects.clear();
}

void JZNodeGraphItem::clear()
{
    auto it = m_pinRects.begin();
    while (it != m_pinRects.end())
    {
        if(it->widget)
            it->widget->disconnect(it->widget, SIGNAL(sigValueChanged(QString)), editor(), SLOT(onItemPropChanged()));

        it++;
    }
}

void JZNodeGraphItem::setRunningMode(ProcessStatus mode)
{
    updateRuntimeStatus();
}

void JZNodeGraphItem::updatePropGemo()
{
    //remove
    auto it = m_pinRects.begin();
    while (it != m_pinRects.end())
    {
        if (!m_node->hasPin(it.key()))
        {
            it->clear();
            it = m_pinRects.erase(it);
        }
        else
            it++;
    }

    //add
    auto list = m_node->pinList();
    for (int i = 0; i < list.size(); i++)
    {
        int pin = list[i];
        if (!m_pinRects.contains(pin))
            m_pinRects[pin] = PropGemo();
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
    QString title = m_node->name();
    painter->fillRect(title_rc, QColor(220,220,220));
    painter->drawText(title_rc, title, text_opt);

    auto in_list = m_node->pinList();
    for (int i = 0; i < in_list.size(); i++)
        drawProp(painter,in_list[i]);   

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

    if(editor()->isBreakPoint(m_id))
    {
        QRect bt_rc = QRect(5,5,15,15);
        painter->fillRect(bt_rc,Qt::green);
    }

    if(!m_error.isEmpty())
    {
        painter->fillRect(m_errorRect,Qt::red);
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

QRectF JZNodeGraphItem::pinRect(int pin)
{
    return m_pinRects[pin].iconRect;
}

QRectF JZNodeGraphItem::pinNameRect(int pin)
{
    return m_pinRects[pin].nameRect;
}

QSize JZNodeGraphItem::size() const
{
    return m_size;
}

void JZNodeGraphItem::calcGemo(int pin_id, int x, int y, PropGemo *gemo)
{
    auto pin = m_node->pin(pin_id);
    gemo->iconRect = QRect(x, y, 24, 24);

    x = gemo->iconRect.right() + 5;
    if (pin->isDispName())
    {
        QFontMetrics ft(scene()->font());
        int w = qMin(name_max_width, ft.horizontalAdvance(pin->name()));
        gemo->nameRect = QRect(x, y, w, 24);
        x = gemo->nameRect.right() + 5;
    }

    if (!gemo->widget)
    {
        JZNodePinWidget *widget = nullptr;
        if (pin->isWidget())
        {
            QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
            if (!gemo->widget)
                widget = m_node->createWidget(pin_id);
        }
        else if(pin->isDispValue())
        {            
            JZNodeParamValueWidget *param_widget = new JZNodeParamValueWidget();
            int up_type = JZNodeType::upType(pin->dataTypeId());
            param_widget->initWidget(up_type);            

            widget = param_widget;                        
        }

        if (widget)
        {
            QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();   
            widget->connect(widget, SIGNAL(sigValueChanged(QString)), editor(), SLOT(onItemPropChanged()));
            widget->setProperty("node_id", m_id);
            widget->setProperty("prop_id", pin_id);

            gemo->widget = widget;
            gemo->proxy = proxy;           
            auto w = widget->sizeHint().width();
            auto h = widget->sizeHint().height();            
            w = qMax(w, 60);
            h = qMax(h, 24);
            gemo->valueRect = QRect(x, y, w, h);
            
            proxy->setWidget(widget);                      
            proxy->setParentItem(this);
            proxy->resize(w, h);
        }        
    }

    if (gemo->widget)
    {
        gemo->valueRect.moveTo(QPoint(x, y));
        if (pin->isEditValue())
        {
            bool editable = editor()->isPropEditable(m_id, pin_id);
            gemo->widget->setEnabled(editable);            
        }
    }
}

void JZNodeGraphItem::setWidgetValue(int prop_id, const QString &value)
{    
    auto *widget = qobject_cast<JZNodePinWidget*>(m_pinRects[prop_id].widget);
    widget->setValue(value);
}

QString JZNodeGraphItem::getWidgetValue(int prop_id)
{
    auto *widget = qobject_cast<JZNodePinWidget*>(m_pinRects[prop_id].widget);
    return widget->value();
}

void JZNodeGraphItem::updateNode()
{
    auto cmp = [this](int i,int j)->bool{        
        int flag1 = m_node->pinPri(i);
        int flag2 = m_node->pinPri(j);
        if(flag1 != flag2)
            return flag1 < flag2;
        else
            return i < j;
    };
    updatePropGemo();

    QString title = m_node->name();
    QFontMetrics title_ft(scene()->font());
    int title_w = title_ft.horizontalAdvance(title) + 20;

    int in_x = 0, out_x = 0;
    int in_y = 24,out_y = 24;
    int y_gap = 4;    

    auto in_list = m_node->pinInList(Pin_none);
    std::stable_sort(in_list.begin(),in_list.end(),cmp);
    for(int i = 0; i < in_list.size(); i++)
    {
        auto &gemo = m_pinRects[in_list[i]];
        calcGemo(in_list[i],4,in_y,&gemo);
        in_x = qMax(in_x,gemo.width());
        in_y += gemo.height() + y_gap;
    }

    QList<int> out_list = m_node->pinOutList(Pin_none);
    std::stable_sort(out_list.begin(),out_list.end(),cmp);
    for(int i = 0; i < out_list.size(); i++)
    {        
        auto &gemo = m_pinRects[out_list[i]];
        calcGemo(out_list[i],4,out_y,&gemo);
        out_x = qMax(out_x,gemo.width());
        out_y += gemo.height() + y_gap;
    }
    
    QList<int> pinList = m_node->pinList();
    for (int i = 0; i < pinList.size(); i++)
    {
        auto pin = m_node->pin(pinList[i]);
        setPinValue(pin->id(), pin->value());
    }

    int w = qMax(100,in_x + out_x + 30);
    w = qMax(title_w, w);

    int h = qMax(in_y, out_y);
    prepareGeometryChange();
    for(int i = 0; i < out_list.size(); i++)
    {
        auto &info = m_pinRects[out_list[i]];
        info.iconRect.moveRight(w - 4);
        QRectF last = info.iconRect;
        if(!info.nameRect.isEmpty())
        {
            info.nameRect.moveRight(last.left() - 5);
            last = info.nameRect;
        }
        if (!info.valueRect.isEmpty())
        {
            info.valueRect.moveRight(last.left() - 5);               
        }
    }
    for (int i = 0; i < pinList.size(); i++)
    {
        auto &info = m_pinRects[pinList[i]];
        if (info.proxy)        
            info.proxy->setPos(info.valueRect.topLeft());                    
    }
    
    m_size = QSize(w, qMax(h,50));    
    updateErrorGemo();
}

void JZNodeGraphItem::updateRuntimeStatus()
{
    ProcessStatus status = editor()->runningMode();

    auto it = m_pinRects.begin();
    while(it != m_pinRects.end())
    {
        if (it->widget)
        {
            int pin_id = it.key();
            auto pin = m_node->pin(pin_id);
            if(status == Process_none)
            {
                if (pin->isEditValue())
                {
                    bool editable = editor()->isPropEditable(m_id, pin_id);                    
                    it->widget->setEnabled(editable);
                }
                else
                {
                    it->widget->setEnabled(true);
                }
            }
            else
            {
                it->widget->setEnabled(false);
                if (pin->isEditValue())
                {   
                    if(editor()->runtimeNode() == m_id)
                    {                        
                        it->widget->setEnabled(true);
                    }                    
                }
            }
        }
        it++;
    }
}

void JZNodeGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{    
    auto pin_id = pinAt(event->pos());
    if (pin_id >= 0)
    {        
        auto pin = m_node->pin(pin_id);
        if ((event->buttons() & Qt::LeftButton) && pin->isOutput() && 
            (pin->isParam() || pin->isFlow() || pin->isSubFlow()))
            m_downPin = pin_id;
        else
            m_downPin = -1;
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
        if (m_downPin != -1 && (event->pos() - m_pinRects[m_downPin].iconRect.center()).manhattanLength() > 10)
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

void JZNodeGraphItem::setPinValue(int prop_id, const QString &value)
{
    auto widget = m_pinRects[prop_id].widget;
    if (!widget)
        return;

    widget->blockSignals(true);
    setWidgetValue(prop_id, value);
    widget->blockSignals(false);
}

QString JZNodeGraphItem::pinValue(int prop_id)
{
    auto widget = m_pinRects[prop_id].widget;
    if (!widget)
        return QString();

    return getWidgetValue(prop_id);
}

void JZNodeGraphItem::setPinRuntimeValue(int pin_id,const JZNodeDebugParamValue &value)
{
    auto widget = m_pinRects[pin_id].widget;
    if (!widget)
        return;

    widget->setValue(value.value);
}

void JZNodeGraphItem::resetPropValue()
{
    QList<int> pinList = m_node->pinList();
    for (int i = 0; i < pinList.size(); i++)
    {
        auto pin = m_node->pin(pinList[i]);
        if (pin->isParam())
            setPinValue(pin->id(), pin->value());
    }
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
    JZNodePin *pin = m_node->pin(prop_id);
    PropGemo &info = m_pinRects[prop_id];

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
    IconType type;
    if(pin->isFlow() || pin->isSubFlow())
    {
        color = QColor(255, 255, 255);
        type = IconType::Flow;
    }
    else if(pin->isParam())
    {
        color = QColor(220,  48,  48);
        type = IconType::Circle;
    }
    else
    {
        type = IconType::Diamond;
    }

    QColor innerColor = QColor(40,40,40,80);
    drawIcon(painter,info.iconRect,type,false,color,innerColor);

    QTextOption text_opt;
    text_opt.setWrapMode(QTextOption::NoWrap);
    if(!pin->name().isEmpty())
    {        
        auto opt = pin->isInput()? Qt::AlignLeft : Qt::AlignRight;
        text_opt.setAlignment(Qt::AlignVCenter | opt);
        painter->drawText(info.nameRect,pin->name(), text_opt);
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
