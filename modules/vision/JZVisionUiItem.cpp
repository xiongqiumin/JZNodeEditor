#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "JZVisionUiItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZModuleVision.h"
#include "JZNodeUtils.h"

JZVisionUiItem::JZVisionUiItem()
{
    m_itemType = ProjectItem_visionUi;
    m_name = "visionUi";
}

JZVisionUiItem::~JZVisionUiItem()
{
    
}

JZNodeObjectWidgetDefine JZVisionUiItem::define()
{
    JZNodeObjectWidgetDefine define;
    define.type = Widget_Vision;
    define.buffer = JZNodeUtils::toBuffer(m_config);
    return define;
}

void JZVisionUiItem::setConfig(JZVisionWindowConfig config)
{
    m_config = config;
}

JZVisionWindowConfig JZVisionUiItem::config()
{
    return m_config;
}

void JZVisionUiItem::saveToStream(QDataStream &s) const
{
    s << m_config;
}

bool JZVisionUiItem::loadFromStream(QDataStream &s)
{
    s >> m_config;
    return true;
}