#ifndef JZNODE_VIEW_MAP_H_
#define JZNODE_VIEW_MAP_H_

#include <QWidget>
#include "JZScriptItem.h"

class JZNodeView;
class JZNodeViewMap : public QWidget
{
    Q_OBJECT

public:
    JZNodeViewMap(QWidget *parent);
    ~JZNodeViewMap();

    void setView(JZNodeView *view);
    void updateMap();

signals:
    void mapSceneChanged(QRectF rc);
    void mapSceneScaled(bool up);

protected:
    struct DrawInfo{
        QPoint toImage(QPointF scene_pt);
        QRect toImage(QRectF scene_rc);
        QPointF toScene(QPoint image_pt);

        QPointF top;
        double wRatio;
        double hRatio;
        int xOffset;
        int yOffset;
        QRect imageRect;
    };

    virtual void paintEvent(QPaintEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    void updateMapCache();

    JZNodeView *m_view;
    QImage m_imageBg;
    QImage m_imageFg;
    DrawInfo m_drawInfo;
    QPoint m_downPoint;
    QPoint m_downViewCenter;
    bool m_down;
    bool m_mapCache;
};










#endif // ! JZNODE_VIEW_MAP_H_
