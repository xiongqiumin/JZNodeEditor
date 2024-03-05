#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "JZNodeViewMap.h"
#include "JZNodeView.h"

//DrawInfo
QPoint JZNodeViewMap::DrawInfo::toImage(QPointF scene_pt)
{
    scene_pt -= top;
    scene_pt.rx() *= wRatio;
    scene_pt.ry() *= hRatio;
    scene_pt.rx() += xOffset;
    scene_pt.ry() += yOffset;
    return scene_pt.toPoint();
}

QRect JZNodeViewMap::DrawInfo::toImage(QRectF scene_rc)
{
    return QRect(toImage(scene_rc.topLeft()), toImage(scene_rc.bottomRight()));
}

QPointF JZNodeViewMap::DrawInfo::toScene(QPoint image_pt)
{
    QPointF ret = image_pt;
    ret -= imageRect.topLeft();
    ret.rx() /= wRatio;
    ret.ry() /= hRatio;
    ret += top;
    return ret;
}

//JZNodeViewMap
JZNodeViewMap::JZNodeViewMap(QWidget *parent)
    :QWidget(parent)
{
    m_view = nullptr;
    m_down = false;
    m_mapCache = false;
}

JZNodeViewMap::~JZNodeViewMap()
{

}

void JZNodeViewMap::setView(JZNodeView *view)
{
    m_view = view;
    updateMap();
}

void JZNodeViewMap::updateMap()
{
    m_mapCache = false;
    update();
}

void JZNodeViewMap::updateMapCache()
{
    if (!m_view->scene())
    {
        m_imageBg = QImage();
        m_imageFg = QImage();        
        return;
    }
    
    QRect scene_rc = m_view->scene()->itemsBoundingRect().toRect();
    QRectF view_rc = m_view->mapToScene(m_view->rect()).boundingRect();
    scene_rc.adjust(-20, -20, 20, 20);
    
    double view_gap = 1.6;
    if (scene_rc.width() < view_rc.width() * view_gap)
    {
        double gap = (view_rc.width() * view_gap - scene_rc.width()) / 2;
        scene_rc.adjust(-gap, -0, gap, 0);
    }
    if (scene_rc.height() < view_rc.height() * view_gap)
    {
        double gap = (view_rc.height() * view_gap - scene_rc.height()) / 2;
        scene_rc.adjust(0, -gap, 0, gap);
    }
    
    QSize img_size = scene_rc.size().scaled(size(),Qt::KeepAspectRatio);
    m_drawInfo.top = scene_rc.topLeft();
    m_drawInfo.wRatio = (double)img_size.width() / scene_rc.width();
    m_drawInfo.hRatio = (double)img_size.height() / scene_rc.height();
    m_drawInfo.xOffset = (width() - img_size.width()) / 2;
    m_drawInfo.yOffset = (height() - img_size.height()) / 2;
    m_drawInfo.imageRect = QRect(m_drawInfo.xOffset, m_drawInfo.yOffset, img_size.width(), img_size.height());

    m_imageBg = QImage(size(), QImage::Format_ARGB32);
    m_imageBg.fill(Qt::gray);    
    
    QPainter pt(&m_imageBg);
    pt.fillRect(m_drawInfo.imageRect,Qt::white);
    
    auto items = m_view->scene()->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_group)
        {
            QRectF rc = items[i]->sceneBoundingRect();
            rc = m_drawInfo.toImage(rc);
            pt.fillRect(rc, Qt::lightGray);
        }
    }

    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node)
        {
            auto graph_item = dynamic_cast<JZNodeGraphItem*>(items[i]);
            QRectF rc = graph_item->sceneBoundingRect();
            rc = m_drawInfo.toImage(rc);
            if(graph_item->isError())
                pt.fillRect(rc, Qt::red);
            else
                pt.fillRect(rc, Qt::black); 
        }
    }

    m_imageFg = m_imageBg.copy();    

    QImage image = QImage(size(), QImage::Format_ARGB32);
    image.fill(QColor(0, 0, 0, 50));
    pt.drawImage(0, 0, image);
}

void JZNodeViewMap::paintEvent(QPaintEvent *event)
{
    if (!m_mapCache)
    {
        updateMapCache();
        m_mapCache = true;
    }

    QRect back_rc = rect();

    QPainter painter(this);    
    if (m_imageBg.isNull())
        return;
        
    painter.drawImage(0,0, m_imageBg);
        
    QRectF view_rc = m_view->mapToScene(m_view->rect()).boundingRect();
    view_rc = m_drawInfo.toImage(view_rc);
    painter.drawImage(view_rc, m_imageFg, view_rc);
    
    painter.setPen(QPen(Qt::green,2));
    painter.drawRect(view_rc.adjusted(0, 0, -1, -1));

    painter.setPen(Qt::black);
    painter.drawRect(back_rc.adjusted(0,0,-1,-1));
}

void JZNodeViewMap::mousePressEvent(QMouseEvent *event)
{
    m_downPoint = event->pos();

    QRectF view_rc = m_view->mapToScene(m_view->rect()).boundingRect();
    view_rc = m_drawInfo.toImage(view_rc);
    if (view_rc.contains(m_downPoint))
    {
        m_down = true;
        m_downViewCenter = view_rc.center().toPoint();
    }
}

void JZNodeViewMap::wheelEvent(QWheelEvent *event)
{
    emit mapSceneScaled(event->angleDelta().y() > 0);
    event->accept();
}

void JZNodeViewMap::mouseMoveEvent(QMouseEvent *event)
{
    if (m_down)
    {
        QRectF view_rc = m_view->mapToScene(m_view->rect()).boundingRect();
        int w = view_rc.width();
        int h = view_rc.height();

        QPoint offset = m_downViewCenter - m_downPoint;
        QPointF pt = m_drawInfo.toScene(event->pos() + offset);
        QRectF rc(pt.x() - w / 2, pt.y() - h / 2, w, h);
        mapSceneChanged(rc);
    }
}

void JZNodeViewMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_drawInfo.imageRect.contains(m_downPoint) && 
        (m_downPoint - event->pos()).manhattanLength() < 10)
    {
        QRectF view_rc = m_view->mapToScene(m_view->rect()).boundingRect();
        int w = view_rc.width();
        int h = view_rc.height();

        QPointF pt = m_drawInfo.toScene(m_downPoint);
        QRectF rc(pt.x() - w/2,pt.y() - h/2,w,h);
        mapSceneChanged(rc);
    }
    m_downPoint = QPoint();
    m_downViewCenter = QPoint();
    m_down = true;
}