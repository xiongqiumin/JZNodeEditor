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

//JZNodeTraceTimeLine
class JZNodeTraceView;
class JZNodeTraceTimeLine : public QWidget
{
    Q_OBJECT

public:
    JZNodeTraceTimeLine(JZNodeTraceView *view);
    ~JZNodeTraceTimeLine();

    void setTraceData(const JZNodeTraceRecord *context);
    void clear();
    void resetView();
    void updateSize();

    double scale();
    qint64 maxTime();

    void setStartTime(qint64 x);
    void setStartY(qint64 y);

protected:
    struct ThreadInfo
    {
        int y;
        int level;
    };

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

    QMap<int, ThreadInfo> m_threadInfo;  //每个线程起始
    const JZNodeTraceRecord * m_record;
    qint64 m_minTime;
    JZNodeTraceView* m_view;
};

//JZNodeTraceTree
class JZNodeTraceTree : public QWidget
{
    Q_OBJECT

public:
    JZNodeTraceTree();
    ~JZNodeTraceTree();

    void setStartY(int y);

protected:
    int m_startY;
};

//JZNodeTraceOuputWidget
class JZNodeTraceOuputWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeTraceOuputWidget();
    ~JZNodeTraceOuputWidget();
};

//JZNodeTraceView
class JZNodeTraceView : public QWidget
{
    Q_OBJECT

public:
    explicit JZNodeTraceView(QWidget *parent = nullptr);
    ~JZNodeTraceView();

    void setTraceData(const JZNodeTraceRecord& context);
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
    JZNodeTraceTree* m_tree;
    JZNodeTraceOuputWidget* m_output;
    JZNodeTraceTimeLine* m_timeLine;

    JZNodeTraceRecord m_record;
};

#endif // JZNODETRACEVIEW_H