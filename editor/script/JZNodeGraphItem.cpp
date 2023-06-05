#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include "JZNodeGraphItem.h"
#include "JZNodeView.h"
#include <math.h>

DispWidget::DispWidget()
{

}

DispWidget::~DispWidget()
{
}

void DispWidget::onValueChanged()
{
    QLineEdit *widget = (QLineEdit *)sender();    
    QVariant value;
    sigValueChanged(0, value);
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
}

JZNodeGraphItem::~JZNodeGraphItem()
{
}

void JZNodeGraphItem::setValue(int prop,QVariant value)
{    
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
    painter->fillRect(rc, QColor(192,192,192));

    QString title = m_node->name();
    painter->fillRect(title_rc, QColor(220,220,220));
    painter->drawText(title_rc, title, QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));

    auto in_list = m_node->propList();
    for (int i = 0; i < in_list.size(); i++)
        drawProp(painter,in_list[i]);   

    if(isSelected()){
        painter->setPen(QPen(Qt::yellow,4));
        painter->drawRect(rc);
    }
}

JZNode *JZNodeGraphItem::node()
{
    return m_node;
}

JZNodePin *JZNodeGraphItem::propAt(QPointF pos)
{    
    auto list = m_node->propList();
    for (int i = 0; i < list.size(); i++)
    {                    
        auto rc = propRect(list[i]);
        if (rc.contains(pos))
            return m_node->prop(list[i]);            
    }
    return nullptr;    
}

QRectF JZNodeGraphItem::propRect(int prop)
{
    return m_propRects[prop];
}

void JZNodeGraphItem::updateNode()
{        
    m_proxy->setVisible(false);

    int in_y = 24,out_y = 24;
    int y_gap = 28;
    m_propRects.clear();
    int flow_in = m_node->flowIn();
    if(flow_in != -1)
    {
        m_propRects[flow_in] = QRectF(4,in_y,24,24);
        in_y += y_gap;
    }
    QVector<int> in_list = m_node->paramInList();
    for(int i = 0; i < in_list.size(); i++)
    {
        m_propRects[in_list[i]] = QRectF(4,in_y,24,24);
        in_y += y_gap;
    }

    int right_x = 100 - 4 - 24;
    QVector<int> sub_flow_list = m_node->subFlowList();
    for(int i = 0; i < sub_flow_list.size(); i++)
    {
        m_propRects[sub_flow_list[i]] = QRectF(right_x,out_y,24,24);
        out_y += y_gap;
    }
    QVector<int> flow_list = m_node->flowOutList();
    for(int i = 0; i < flow_list.size(); i++)
    {
        m_propRects[flow_list[i]] = QRectF(right_x,out_y,24,24);
        out_y += y_gap;
    }
    QVector<int> out_list = m_node->paramOutList();
    for(int i = 0; i < out_list.size(); i++)
    {
        m_propRects[out_list[i]] = QRectF(right_x,out_y,24,24);
        out_y += y_gap;
    }
    
    int h = qMax(in_y, out_y);
    prepareGeometryChange();
    m_size = QSize(100, qMax(h,100));
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
        if (m_node->prop(list[i])->isOutput() && propRect(list[i]).contains(event->pos()))
        {
            event->ignore();

            JZNodeGemo gemo(m_node->id(), list[i]);
            editor()->startLine(gemo);
            return;
        }
    }
    return JZNodeBaseItem::mousePressEvent(event);
}

void JZNodeGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    return JZNodeBaseItem::mouseReleaseEvent(event);
}

void JZNodeGraphItem::drawProp(QPainter *painter,int prop_id)
{
    JZNodePin *pin = m_node->prop(prop_id);
    QRectF rc = m_propRects[prop_id];

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
    else
    {
        color = QColor(220,  48,  48);
        type = IconType::Circle;
    }
    QColor innerColor = QColor(40,40,40,80);
    drawIcon(painter,rc,type,false,color,innerColor);
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
    auto AddConvexPolyFilled = [painter](const QPainterPath &path,QColor c)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->drawPath(path);
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
