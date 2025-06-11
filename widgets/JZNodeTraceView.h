#ifndef JZNODETRACEVIEW_H
#define JZNODETRACEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <QTimer>
#include <QDebug>
#include <QScrollBar>
#include <QScrollArea>
#include <QScrollBar>
#include "JZNodeTrace.h"

class JZNodeTraceView;
class JZNodeTraceScene : public QWidget
{
    Q_OBJECT

public:
    JZNodeTraceScene(JZNodeTraceView *view);
    ~JZNodeTraceScene();

    void setTraceData(const QList<JZNodeTraceItem>& data);
    void clear();
    void resetView();
    void updateSize();

    double scale();
    qint64 maxTime();

    void setStartTime(qint64 x);
    void setStartY(qint64 y);

protected:
    // 鼠标事件
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

    void drawItem(QPainter& painter, const JZNodeTraceItem& item);
    void drawScale(QPainter& painter);
    QColor getForegroundColor(const QColor& backgroundColor, int threshold = 128);

    // 缩放相关
    static constexpr double MIN_SCALE = 1;
    static constexpr double MAX_SCALE = 1e6;

    qint64 m_downTime;
    QPoint m_downPos;
    bool m_isDragging;

    qint64 m_startTime;
    qint64 m_maxTime;
    qint64 m_startY;
    double m_scale;     //一个像素等于多少us
    QList<JZNodeTraceItem> m_traceData;
    JZNodeTraceView* m_view;
};


class JZNodeTraceView : public QWidget
{
    Q_OBJECT

public:
    explicit JZNodeTraceView(QWidget *parent = nullptr);
    ~JZNodeTraceView();

    void setTraceData(const QList<JZNodeTraceItem>& data);
    void clear();
    void resetView();

    QScrollBar* horizontalScroll();  //水平
    QScrollBar* verticalScroll();

protected slots:
    void onHorizontalScroll(int value);
    void onVerticalScroll(int value);

protected:
    QScrollBar* m_hScollBar;
    QScrollBar* m_vScollBar;
    JZNodeTraceScene* m_scene;
};

#endif // JZNODETRACEVIEW_H