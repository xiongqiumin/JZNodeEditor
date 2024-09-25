#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZUiFile.h"
#include "JZParamItem.h"
#include "JZProject.h"

JZProjectItem::JZProjectItem(int itemType)
{
    m_parent = nullptr;    
    m_itemType = itemType;    
    m_pri = 0;
}

JZProjectItem::~JZProjectItem()
{
}

QByteArray JZProjectItem::toBuffer() const
{
    QByteArray ret;
    QDataStream s(&ret, QIODevice::WriteOnly);
    s << m_itemType;
    s << m_name;
    s << m_pri;
    saveToStream(s);
    
    QList<QByteArray> sub_list;
    for (int i = 0; i < m_childs.size(); i++)
    {
        QByteArray sub = m_childs[i]->toBuffer();
        sub_list << sub;
    }
    s << sub_list;
    return ret;
}

void JZProjectItem::fromBuffer(const QByteArray &buffer)
{    
    int itemType;
    QDataStream s(buffer);
    s >> itemType;
    Q_ASSERT(itemType == m_itemType);
    s >> m_name;
    s >> m_pri;
    loadFromStream(s);

    QList<QByteArray> sub_list;
    s >> sub_list;
    for (int i = 0; i < sub_list.size(); i++)
    {
        QDataStream sub_s(sub_list[i]);
        int type = ProjectItem_none;
        sub_s >> type;

        auto sub_item = JZProjectItemManager::instance()->create(type);
        JZProjectItemPtr child = JZProjectItemPtr(sub_item);
        addItem(child);
        child->fromBuffer(sub_list[i]);        
    }
}

void JZProjectItem::saveToStream(QDataStream &s) const
{
}

bool JZProjectItem::loadFromStream(QDataStream &s)
{
    return true;
}

const JZProject *JZProjectItem::project() const
{
    auto item = const_cast<JZProjectItem*>(this);
    return item->project();
}

JZProject *JZProjectItem::project() 
{
    if (m_itemType == ProjectItem_root)
    {
        JZProjectItemRoot *root = (JZProjectItemRoot *)this;
        return root->rootProject();
    }
    else
    {
        if (parent())
            return parent()->project();
        else
            return nullptr;
    }
}

void JZProjectItem::sort()
{
    std::sort(m_childs.begin(),m_childs.end(),[]( JZProjectItemPtr &i1_ptr, JZProjectItemPtr &i2_ptr){
        auto i1 = i1_ptr.data();
        auto i2 = i2_ptr.data();
        if(i1->m_pri != i2->m_pri)
            return i1->m_pri < i2->m_pri;
        return i1->m_name < i2->m_name;
    });
}

QString JZProjectItem::name() const
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

int JZProjectItem::itemType() const
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

JZScriptClassItem *JZProjectItem::getClassFile() 
{
    if (!project())
        return nullptr;

    return project()->getItemClass(this);
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

bool JZProjectItem::hasItem(QString name) 
{
    return getItem(name) != nullptr;
}

QList<JZProjectItem *> JZProjectItem::childs() 
{
    QList<JZProjectItem *> result;
    for(int i = 0; i < m_childs.size(); i++)
        result.push_back(m_childs[i].data());
    return result;
}

int JZProjectItem::childCount()
{
    return m_childs.size();
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

QList<JZProjectItem *> JZProjectItem::itemList(int type) 
{
    QList<int> type_list = { type };
    return itemList(type_list);
}

QList<JZProjectItem *> JZProjectItem::itemList(QList<int> type_list)
{
    QList<JZProjectItem *> result;
    if(type_list.contains(this->itemType()) || type_list.contains(ProjectItem_any))
        result << this;

    auto chlid = this->childs();
    for(int i = 0; i < chlid.size(); i++)
        result << chlid[i]->itemList(type_list);
        
    return result;
}

//JZProjectItemRoot
JZProjectItemRoot::JZProjectItemRoot()
    :JZProjectItem(ProjectItem_root)
{
    m_project = nullptr;
}

JZProjectItemRoot::~JZProjectItemRoot()
{
}

JZProject *JZProjectItemRoot::rootProject()
{
    return m_project;
}

void JZProjectItemRoot::setRootProject(JZProject *project)
{
    m_project = project;
}

//JZProjectItemFolder
JZProjectItemFolder::JZProjectItemFolder()
    :JZProjectItem(ProjectItem_folder)
{
}

JZProjectItemFolder::~JZProjectItemFolder()
{

}

int JZProjectItemIsScript(JZProjectItem *item)
{
    auto type = item->itemType();
    if (type == ProjectItem_scriptParamBinding
        || type == ProjectItem_scriptFunction)
        return true;

    return false;
}

//JZProjectItemManager
JZProjectItemManager *JZProjectItemManager::instance()
{    
    static JZProjectItemManager inst;
    return &inst;
}

void JZProjectItemManager::registItem(int item_type,JZProjectItemCreateFunc func)
{
    m_funcs[item_type] = func;
}

JZProjectItem *JZProjectItemManager::create(int item_type)
{
    Q_ASSERT(m_funcs.contains(item_type));
    return m_funcs[item_type]();
}