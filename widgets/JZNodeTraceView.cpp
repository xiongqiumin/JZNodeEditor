#include "JZNodeTraceView.h"
#include <QPainter>
#include <QPainterPath>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QHboxLayout>
#include <QVboxLayout>
#include <QSplitter>

//JZNodeTraceTree
JZNodeTraceTree::JZNodeTraceTree()
{
    m_startY = 0;
}

JZNodeTraceTree::~JZNodeTraceTree()
{
}

void JZNodeTraceTree::setStartY(int y)
{
    m_startY = y;
}

//JZNodeTraceOuputWidget
JZNodeTraceOuputWidget::JZNodeTraceOuputWidget()
{
}

JZNodeTraceOuputWidget::~JZNodeTraceOuputWidget()
{
}


//JZNodeTraceTimeLine
// 默认颜色映射函数
QColor defaultColorMapper(int level) {
    // 根据层级生成不同的颜色
    static const QList<QColor> baseColors = {
        QColor(240, 163, 255), QColor(0, 117, 220), QColor(153, 63, 0),
        QColor(76, 0, 92), QColor(25, 25, 25), QColor(0, 92, 49),
        QColor(43, 206, 72), QColor(255, 204, 153), QColor(128, 128, 128),
        QColor(148, 255, 181), QColor(143, 124, 0), QColor(157, 204, 0),
        QColor(194, 0, 136), QColor(0, 51, 128), QColor(255, 164, 5),
        QColor(255, 168, 187), QColor(66, 102, 0), QColor(255, 0, 16),
        QColor(94, 241, 242), QColor(0, 153, 143), QColor(224, 255, 102),
        QColor(116, 10, 255), QColor(153, 0, 0), QColor(255, 255, 128),
        QColor(255, 255, 0), QColor(255, 80, 5)
    };
    
    // 基础颜色 + 层级变化
    return baseColors[level % baseColors.size()].lighter(100 + (level / baseColors.size()) * 20);
}

//JZNodeTraceTimeLine
JZNodeTraceTimeLine::JZNodeTraceTimeLine(JZNodeTraceView* view)
{
    m_view = view;
    m_isDragging = false;
    m_scale = 1000;
    m_startTime = 0;
    m_maxTime = 0;
    m_startY = 0;
}

JZNodeTraceTimeLine::~JZNodeTraceTimeLine()
{
}

void JZNodeTraceTimeLine::setTraceData(const JZNodeTraceRecord* context)
{
    m_record = context;

    qint64 max = 0, min = INT64_MAX;
    for (int i = 0; i < m_record->items.size(); i++)
    {
        auto &t = m_record->items[i];
        min = qMin(t.start, min);
        max = qMax(t.start + t.duration, max);

        if (!m_threadInfo.contains(t.thread))
        {
            ThreadInfo thread_info;
            thread_info.y = 0;
            thread_info.level = 0;
            m_threadInfo[t.thread] = thread_info;
        }

        m_threadInfo[t.thread].level = qMax(m_threadInfo[t.thread].level, t.level);
    }
    m_minTime = min;
    m_maxTime = max - min;
    updateSize();
}

void JZNodeTraceTimeLine::clear()
{
    m_minTime = 0;
    resetView();
}

void JZNodeTraceTimeLine::resetView()
{
    m_scale = 1000;
    update();
}

void JZNodeTraceTimeLine::updateSize()
{
    QList<int> pre_thread;
    auto it = m_threadInfo.begin();
    while (it != m_threadInfo.end())
    {
        int y = 0;
        for (int i = 0; i < pre_thread.size(); i++)
        {
            int pre_id = pre_thread[i];
            y += m_threadInfo[pre_id].level * 30;
        }
        it->y = y;
        pre_thread.push_back(it.key());
        it++;
    }

    qint64 endTime = m_maxTime - qint64(width() * m_scale);
    if(endTime > 0)
        m_view->horizontalScroll()->setRange(0, (int)endTime);
    else
        m_view->horizontalScroll()->setRange(0, 0);
    update();
}

void JZNodeTraceTimeLine::setStartTime(qint64 x)
{
    m_startTime = x;
    update();
}

void JZNodeTraceTimeLine::setStartY(qint64 y)
{
    m_startY = y;
    update();
}

void JZNodeTraceTimeLine::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_downPos = event->pos();
        m_downTime = m_startTime;
        m_isDragging = true;
    }
}

void JZNodeTraceTimeLine::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        QPoint delta = event->pos() - m_downPos;
        qint64 cur_time = m_downTime - delta.x() * m_scale;
        m_view->horizontalScroll()->setValue(cur_time);
        qDebug() << cur_time;
    }
}

void JZNodeTraceTimeLine::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        m_downTime = -1;
    }
}

void JZNodeTraceTimeLine::wheelEvent(QWheelEvent* event)
{
    double new_scale = m_scale;
    if (event->angleDelta().y() >= 0)
        new_scale *= 1.05;
    else
        new_scale /= 1.05;

    new_scale = qBound(MIN_SCALE, new_scale, MAX_SCALE);
    if (new_scale == m_scale)
        return;

    m_scale = new_scale;
    updateSize();
}

void JZNodeTraceTimeLine::contextMenuEvent(QContextMenuEvent* event)
{
}

QColor JZNodeTraceTimeLine::getForegroundColor(const QColor& color, int threshold)
{
    int weightedLum = 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();
    return weightedLum < threshold ? Qt::white : Qt::black;
}

void JZNodeTraceTimeLine::drawScale(QPainter &painter) 
{
    QStringList unit_list = { "1us","10us","100us","1ms","10ms","100ms","1s","10s","100s" };
    double unit_len = 0;
    int uint_idx = -1;
    qint64 time = 1;
    for (int i = 0; i < unit_list.size(); i++)
    {
        unit_len = time / m_scale;
        if (unit_len > 10)
        {
            uint_idx = i;
            break;
        }
        time *= 10;
    }
    if (uint_idx == -1)
        uint_idx = unit_list.size() - 1;
    
    QString scaleText = unit_list[uint_idx];
    int lineLength = (int)unit_len;

    int scale_w = 120;
    int scale_h = 30;
    QRect scale_rc(width() - scale_w - 10, height() - scale_h - 10, scale_w, scale_h);
    painter.fillRect(scale_rc,QColor(255,255,255,150));
    
    painter.setPen(QPen(Qt::black,2));
    int x = scale_rc.right() - 2 - lineLength, y = scale_rc.bottom() - 2;
    painter.drawLine(x, y, x + lineLength, y);

    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(x-1, y - 2, x - 1, y + 2);
    painter.drawLine(x + lineLength+1, y - 2, x + lineLength + 1, y + 2);

    scale_rc.setBottom(height() - 5 - 10);
    painter.drawText(scale_rc, scaleText, QTextOption(Qt::AlignVCenter | Qt::AlignRight));
}

void JZNodeTraceTimeLine::drawItem(QPainter &painter, const JZNodeTraceItem &item) 
{
    double itemStart = item.start;
    int x = static_cast<int>((itemStart - m_startTime - m_minTime) / m_scale);
    int width = static_cast<int>(item.duration / m_scale);
    int y = m_threadInfo[item.thread].y + item.level * 20;
    int height = 15;

    QColor bg_color = defaultColorMapper(item.level);
    QRect rc = QRect(x, y, width, height);
    painter.setBrush(bg_color);
    painter.drawRect(rc);

    if (width > 20) {
        painter.setPen(getForegroundColor(bg_color));
        QString text = item.name;
        if (painter.fontMetrics().width(text) > width - 10) 
        {
            text = painter.fontMetrics().elidedText(text, Qt::ElideRight, width - 10);
        }
        if (rc.left() < 0)
            rc.setLeft(0);
        if (rc.right() > this->width())
            rc.setRight(this->width());
        painter.drawText(rc, text, QTextOption(Qt::AlignCenter));
    }
}

void JZNodeTraceTimeLine::resizeEvent(QResizeEvent* event)
{
    updateSize();
}

void JZNodeTraceTimeLine::paintEvent(QPaintEvent *event) 
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    double visibleStart = m_startTime;
    double visibleEnd = m_startTime + (width() * m_scale);
    int visibleStartY = m_startY;
    int visibleEndY = m_startY + height();

    // 计算总时间范围
    qint64 minTime = std::numeric_limits<qint64>::max();
    qint64 maxTime = std::numeric_limits<qint64>::min();
    for (const auto &item : m_record->items) {
        minTime = qMin(minTime, item.start);
        maxTime = qMax(maxTime, item.start + item.duration);
    }

    for (const auto &item : m_record->items) {
        double itemStart = item.start;
        double itemEnd = item.start + item.duration;
        int itemStartY = m_threadInfo[item.thread].y + item.level * 20;
        int itemEndY = itemStartY + 20;

        if (itemEnd < visibleStart || itemStart > visibleEnd)
            continue;
        if (itemEndY < visibleStartY || itemStartY < visibleEndY)
            continue;

        drawItem(painter, item);
    }

    drawScale(painter);
}

//JZNodeTraceView
JZNodeTraceView::JZNodeTraceView(QWidget *parent)
{
    QSplitter* splitter_top = new QSplitter();
    QSplitter* splitter_main = new QSplitter();

    m_tree = new JZNodeTraceTree();
    m_output = new JZNodeTraceOuputWidget();

    m_timeLine = new JZNodeTraceTimeLine(this);
    setMinimumSize(400, 300);

    QWidget* time_widget = new QWidget();

    QVBoxLayout* v = new QVBoxLayout();
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    QHBoxLayout* h1 = new QHBoxLayout();
    QHBoxLayout* h2 = new QHBoxLayout();
    h1->setContentsMargins(0, 0, 0, 0);
    h1->setSpacing(0);
    h2->setContentsMargins(0, 0, 0, 0);
    h2->setSpacing(0);

    m_hScollBar = new QScrollBar(Qt::Horizontal);
    m_vScollBar = new QScrollBar(Qt::Vertical);
    connect(m_hScollBar, &QScrollBar::valueChanged, this, &JZNodeTraceView::onHorizontalScroll);
    connect(m_vScollBar, &QScrollBar::valueChanged, this, &JZNodeTraceView::onVerticalScroll);

    m_hScollBar->setRange(0,0);
    m_vScollBar->setRange(0,0);

    h1->addWidget(m_timeLine);
    h1->addWidget(m_vScollBar);
    h2->addWidget(m_hScollBar);
    //QWidget* coner = new QWidget();
    //h2->addWidget(coner);
    v->addLayout(h1);
    v->addLayout(h2);
    time_widget->setLayout(v);

    splitter_top->addWidget(m_tree);
    splitter_top->addWidget(time_widget);
    splitter_top->setSizes({ 200,600 });
    splitter_top->setChildrenCollapsible(false);

    splitter_main->addWidget(splitter_top);
    splitter_main->addWidget(m_output);
    splitter_main->setSizes({ 600,200 });
    splitter_main->setChildrenCollapsible(false);

    QVBoxLayout* self_layout = new QVBoxLayout();
    self_layout->addWidget(splitter_main);
    setLayout(self_layout);
}

JZNodeTraceView::~JZNodeTraceView()
{
}

QScrollBar* JZNodeTraceView::verticalScroll()
{
    return m_vScollBar;
}

QScrollBar* JZNodeTraceView::horizontalScroll()
{
    return m_hScollBar;
}

void JZNodeTraceView::onHorizontalScroll(int value)
{
    m_timeLine->setStartTime(value);
}

void JZNodeTraceView::onVerticalScroll(int value)
{
    m_tree->setStartY(value);
    m_timeLine->setStartY(value);
}

void JZNodeTraceView::setTraceData(const JZNodeTraceRecord& context)
{
    m_timeLine->setTraceData(&context);
}

void JZNodeTraceView::clear()
{
    m_timeLine->clear();
}

void JZNodeTraceView::resetView()
{
    m_timeLine->resetView();
}
