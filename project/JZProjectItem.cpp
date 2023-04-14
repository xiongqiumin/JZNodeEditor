#include "JZProjectItem.h"

JZProjectItem::JZProjectItem(int itemType,bool folder)
{
    m_parent = nullptr;
    m_itemType = ProjectItem_none;
    m_folder = folder;
}

JZProjectItem::~JZProjectItem()
{
}

bool JZProjectItem::isFolder()
{
    return m_folder;        
}

QString JZProjectItem::name()
{
    return m_name;        
}

void JZProjectItem::setName(QString name)
{
    m_name = name;
}

QString JZProjectItem::path()
{
    if(m_parent)
        return m_parent->filePath();
    else
        return QString();
}

QString JZProjectItem::itemPath()
{
    return path() + "/" + m_name;
}

int JZProjectItem::itemType()
{
    return m_itemType;
}

void JZProjectItem::setItemType(int type)
{
    m_itemType = type;
}

JZProjectItem *JZProjectItem::parent()
{
    return m_parent;
}

void JZProjectItem::addItem(JZProjectItem *child)
{
    m_childs.push_back(child);
}

void JZProjectItem::removeItem(JZProjectItem *child)
{
    m_childs.remove(child);
}

JZProjectItem *JZProjectItem::getItem(QString name)
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        if(m_childs[i]->name() == name)
            return m_childs[i];
    }
    return nullptr;
}

QList<JZProjectItem *> JZProjectItem::items()
{
    return m_childs;
}