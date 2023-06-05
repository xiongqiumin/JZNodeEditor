#include "JZProjectItem.h"
#include "JZScriptFile.h"

JZProjectItem::JZProjectItem(int itemType)
{
    m_parent = nullptr;
    m_project = nullptr;
    m_itemType = itemType;    
    m_folder = false;
}

JZProjectItem::~JZProjectItem()
{
}

void JZProjectItem::setProject(JZProject *project)
{
    m_project = project;
}

JZProject *JZProjectItem::project() const
{
    return m_project;
}

void JZProjectItem::saveToStream(QDataStream &s)
{
    s << m_name;
    s << m_folder;
    s << m_childs.size();
    for(int i = 0; i < m_childs.size(); i++)
    {
        s << m_childs[i]->itemType();
        m_childs[i]->saveToStream(s);
    }
}

void JZProjectItem::loadFromStream(QDataStream &s)
{
    s >> m_name;
    s >> m_folder;
    int size = 0;
    s >> size;
    for(int i = 0; i < size; i++)
    {
        int type;
        s >> type;
        auto ptr = JZProjectItemFactory::create(type);
        ptr->loadFromStream(s);
        m_childs.push_back(JZProjectItemPtr(ptr));
    }
}

bool JZProjectItem::isFolder()
{
    return m_folder;        
}

void JZProjectItem::sort()
{
    std::sort(m_childs.begin(),m_childs.end(),[](const JZProjectItemPtr &i1_ptr,const JZProjectItemPtr &i2_ptr){
        auto i1 = i1_ptr.data();
        auto i2 = i2_ptr.data();
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

void JZProjectItem::addItem(JZProjectItemPtr child)
{
    Q_ASSERT(child->parent() == nullptr);
    child->m_parent = this;
    m_childs.push_back(child);
}

void JZProjectItem::removeItem(int index)
{
    m_childs.removeAt(index);
}

JZProjectItem *JZProjectItem::getItem(QString name)
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        if(m_childs[i]->name() == name)
            return m_childs[i].data();
    }
    return nullptr;
}

QList<JZProjectItem *> JZProjectItem::childs()
{
    QList<JZProjectItem *> result;
    for(int i = 0; i < m_childs.size(); i++)
        result.push_back(m_childs[i].data());
    return result;
}

void JZProjectItem::removeChlids()
{
    m_childs.clear();
}

int JZProjectItem::indexOfItem(JZProjectItem *item)
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        if(m_childs[i].data() == item)
            return i;
    }
    return -1;
}

//JZProjectItemFolder
JZProjectItemFolder::JZProjectItemFolder()
    :JZProjectItem(ProjectItem_folder)
{
    m_folder = true;
}

JZProjectItemFolder::~JZProjectItemFolder()
{

}

JZProjectItem *JZProjectItemFactory::create(int itemType)
{
    if(itemType == ProjectItem_folder)
        return new JZProjectItem(ProjectItem_folder);
    else if(itemType >= ProjectItem_scriptParam && itemType <= ProjectItem_scriptFlow)
        return new JZScriptFile(itemType);

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
