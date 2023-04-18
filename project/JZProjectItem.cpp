#include "JZProjectItem.h"
#include "JZScriptFile.h"

JZProjectItem::JZProjectItem(int itemType,bool folder)
{
    m_parent = nullptr;
    m_itemType = itemType;
    m_folder = folder;
}

JZProjectItem::~JZProjectItem()
{
}

void JZProjectItem::saveToStream(QDataStream &s)
{
    
}

void JZProjectItem::loadFromStream(QDataStream &s)
{

}

bool JZProjectItem::isFolder()
{
    return m_folder;        
}

void JZProjectItem::sort()
{
    std::sort(m_childs.begin(),m_childs.end(),[](const JZProjectItem *i1,const JZProjectItem *i2){
        if(i1->m_folder != i2->m_folder)
            return (int)i1->m_folder > (int)i2->m_folder;
        return i1->m_name < i2->m_name;
    });
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
        return m_parent->itemPath();
    else
        return QString();
}

QString JZProjectItem::itemPath()
{
    if(m_parent)
        return m_parent->itemPath() + "/" + m_name;
    else
        return m_name;
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
    child->m_parent = this;
    m_childs.push_back(child);
}

void JZProjectItem::removeItem(JZProjectItem *child)
{
    if(m_childs.removeOne(child))
        child->m_parent = nullptr;
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

int JZProjectItem::indexOfItem(JZProjectItem *item)
{
    return m_childs.indexOf(item);
}

//JZProjectRoot
JZProjectRoot::JZProjectRoot()
    :JZProjectItem(ProjectItem_root,true)
{
    m_name = ".";
}

JZProjectRoot::~JZProjectRoot()
{

}

JZProjectItem *JZProjectItemFactory::create(int itemType,bool folder)
{
    if(itemType >= ProjectItem_scriptParam && itemType <= ProjectItem_scriptFlow)
        return new JZScriptFile(itemType,folder);

    Q_ASSERT(0);
    return nullptr;
}

QString JZProjectItemFactory::itemTypeName(int itemType)
{
    if(itemType == ProjectItem_scriptParam)
        return "变量绑定";
    else if(itemType == ProjectItem_scriptFlow)
        return "流程";

    return QString();
}
