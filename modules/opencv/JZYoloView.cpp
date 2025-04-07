#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include "JZYoloView.h"

JZYoloView::JZYoloView()
{
    m_colorList.push_back(QColor(255, 0, 0));    // 蓝色
    m_colorList.push_back(QColor(0, 255, 0));    // 绿色
    m_colorList.push_back(QColor(0, 0, 255));   // 红色
    m_colorList.push_back(QColor(255, 255, 0));  // 青色
    m_colorList.push_back(QColor(255, 0, 255));  // 品红色
    m_colorList.push_back(QColor(0, 255, 255));  // 黄色
    m_colorList.push_back(QColor(128, 0, 0));    // 深蓝色
    m_colorList.push_back(QColor(0, 128, 0));    // 深绿色
    m_colorList.push_back(QColor(0, 0, 128));   // 深红色
    m_colorList.push_back(QColor(128, 128, 0));   // 深青色);
}

JZYoloView::~JZYoloView()
{
}

void JZYoloView::setYoloResult(QImage image,const QList<JZYoloResult> &m_lists)
{
    m_scene->clear();
    setImage(image);

    QMap<QString,QColor> color_map; 
    int color_idx = 0;
    for(int i = 0; i < m_lists.size(); i++)
    {
        auto rc_item = new QGraphicsRectItem();
        auto &ret = m_lists[i];

        if(!color_map.contains(ret.name))
        {
            color_map[ret.name] = m_colorList[color_idx];
            color_idx = (color_idx + 1)%m_colorList.size();
        }

        QPen pen(color_map[ret.name]);
        QRectF rc(0,0,ret.rect.width(),ret.rect.height());
        rc_item->setPos(ret.rect.topLeft());
        rc_item->setRect(rc);
        rc_item->setPen(pen);
        m_scene->addItem(rc_item);

        QGraphicsSimpleTextItem *item_name = new QGraphicsSimpleTextItem(ret.name);
        QGraphicsSimpleTextItem *item_conf = new QGraphicsSimpleTextItem(QString::asprintf("%0.2f",ret.confidence));
        item_name->setPos(ret.rect.topLeft());
        item_name->setPen(pen);
        m_scene->addItem(item_name);

        item_conf->setPos(ret.rect.topLeft());
        item_conf->setPen(pen);
        m_scene->addItem(item_conf);
    }
}
