#include <QPainter>
#include "JZNodeBaseItem.h"
#include "JZNodeAbstractView.h"

JZNodeBaseItem::JZNodeBaseItem()
{
    m_type = Item_none;
    m_id = INVALID_ID;
}

JZNodeBaseItem::~JZNodeBaseItem()
{
}

int JZNodeBaseItem::id() const
{
    return m_id;
}

void JZNodeBaseItem::setId(int id)
{
    m_id = id;
}

JZNodeAbstractView *JZNodeBaseItem::editor() const
{
    if (this->scene() && this->scene()->views().size() > 0)
        return dynamic_cast<JZNodeAbstractView*>(this->scene()->views()[0]);
    else
        return nullptr;
}

int JZNodeBaseItem::type() const
{
    return m_type;
}

QVariant JZNodeBaseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    auto edit = editor();
    if (!edit)
        return QGraphicsItem::itemChange(change, value);

    return edit->onItemChange(this, change, value);
}

//JZAbstractLineItem
JZAbstractLineItem::JZAbstractLineItem(JZNodeGemo from)
{
    m_type = Item_line;
    m_from = from;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(-1);
}

JZAbstractLineItem::~JZAbstractLineItem()
{
}

JZNodeGemo JZAbstractLineItem::startTraget()
{
    return m_from;
}

JZNodeGemo JZAbstractLineItem::endTraget()
{
    return m_to;
}

void JZAbstractLineItem::setEndPoint(QPointF point)
{
    prepareGeometryChange();
    m_endPoint = point;
}

void JZAbstractLineItem::setEndTraget(JZNodeGemo to)
{
    prepareGeometryChange();
    m_to = to;
}

//JZAbstractNodeItem
JZAbstractNodeItem::JZAbstractNodeItem(JZNode *node)    
{
    m_type = Item_node;
    m_node = node;
    m_id = node->id();

    setOpacity(0.9);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);    
    m_baseZValue = 0;

    setAcceptHoverEvents(true);
}

JZAbstractNodeItem::~JZAbstractNodeItem()
{
}

QRectF JZAbstractNodeItem::boundingRect() const
{
    QRectF rc(0, 0, m_size.width(), m_size.height());
    return rc;
}

void JZAbstractNodeItem::setBaseZValue(int value)
{
    m_baseZValue = value;
    setZValue(value);
}

JZNode* JZAbstractNodeItem::node()
{
    return m_node;
}

QSize JZAbstractNodeItem::size() const
{
    return m_size;
}

bool JZAbstractNodeItem::isError()
{
    return !m_error.isEmpty();
}

void JZAbstractNodeItem::clearError()
{
    m_error.clear();
    update();
}

void JZAbstractNodeItem::setError(QString error)
{
    m_error = error;
    update();
}


void JZAbstractNodeItem::drawIcon(QPainter *painter, QRectF rect, IconType type, bool filled, QColor color, QColor innerColor)
{
    auto rect_x = rect.x();
    auto rect_y = rect.y();
    auto rect_w = rect.width();
    auto rect_h = rect.height();
    auto rect_center_x = rect.center().x();
    auto rect_center_y = rect.center().y();
    auto rect_center = QPoint(rect_center_x, rect_center_y);
    const auto outline_scale = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

    QPainterPath path;
    auto AddCircle = [painter](QPointF center, double r, QColor c, int flag, double thickness)
    {
        painter->setPen(QPen(c, thickness));
        painter->drawEllipse(center, r, r);
    };
    auto AddCircleFilled = [painter](QPointF center, double r, QColor c, int)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->drawEllipse(center, r, r);
    };
    auto AddRect = [painter](QPointF p1, QPointF p2, QColor c, double round, int flag, double thickness)
    {
        painter->setPen(QPen(c, thickness));
        painter->drawRect(QRectF(p1, p2));
    };
    auto AddRectFilled = [painter](QPointF p1, QPointF p2, QColor c, int a = 0, int b = 0)
    {
        painter->fillRect(QRectF(p1, p2), c);
    };
    auto AddConvexPolyFilled = [painter](const QPainterPath &poly_path, QColor c)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->drawPath(poly_path);
    };
    auto AddTriangleFilled = [painter](QPointF p1, QPointF p2, QPointF p3, QColor c)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        QPolygonF polygon;
        polygon << p1 << p2 << p3;
        painter->drawPolygon(polygon);
    };

    auto PathBezierCubicCurveTo = [&path](QPointF p1, QPointF p2, QPointF p3) { path.cubicTo(p1, p2, p3); };
    auto PathLineTo = [&path](QPointF pt)
    {
        if (path.elementCount() == 0)
            path.moveTo(pt);
        else
            path.lineTo(pt);
    };
    auto PathFillConvex = [painter, &path](QColor c)
    {
        painter->fillPath(path, c);
    };
    auto PathStroke = [painter, &path](QColor c, bool flag, double lineSize)
    {
        path.closeSubpath();
        painter->setPen(QPen(c, lineSize));
        painter->drawPath(path);
    };

    painter->save();
    if (type == IconType::Flow)
    {
        const auto origin_scale = rect_w / 24.0f;

        const auto offset_x = 1.0f * origin_scale;
        const auto offset_y = 0.0f * origin_scale;
        const auto margin = (filled ? 2.0f : 2.0f) * origin_scale;
        const auto rounding = 0.1f * origin_scale;
        const auto tip_round = 0.7f; // percentage of triangle edge (for tip)
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

        const auto left = canvas_x + canvas_w            * 0.5f * 0.3f;
        const auto right = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
        const auto top = canvas_y + canvas_h            * 0.5f * 0.2f;
        const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
        const auto center_y = (top + bottom) * 0.5f;
        //const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

        const auto tip_top = QPointF(canvas_x + canvas_w * 0.5f, top);
        const auto tip_right = QPointF(right, center_y);
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
        rect_x += rect_offset;
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
                const auto r = 0.5f * rect_w / 2.0f;
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
                const auto r = 0.5f * rect_w / 2.0f;
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

                PathLineTo(c + QPointF(0, -r));
                PathLineTo(c + QPointF(r, 0));
                PathLineTo(c + QPointF(0, r));
                PathLineTo(c + QPointF(-r, 0));
                PathFillConvex(color);
            }
            else
            {
                const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                const auto c = rect_center;

                PathLineTo(c + QPointF(0, -r));
                PathLineTo(c + QPointF(r, 0));
                PathLineTo(c + QPointF(0, r));
                PathLineTo(c + QPointF(-r, 0));

                if (innerColor.rgb() & 0xFF000000)
                    AddConvexPolyFilled(path, innerColor);

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