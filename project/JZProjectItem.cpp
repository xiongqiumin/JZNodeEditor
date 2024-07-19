#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZUiFile.h"
#include "JZParamItem.h"
#include "JZProject.h"

JZProjectItem::JZProjectItem(int itemType)
{
    m_parent = nullptr;
    m_project = nullptr;
    m_editor = nullptr;
    m_itemType = itemType;    
    m_pri = 0;
}

JZProjectItem::~JZProjectItem()
{
}

void JZProjectItem::setProject(JZProject *project)
{
    m_project = project;
}

JZProject *JZProjectItem::project() 
{
    return m_project;
}

void JZProjectItem::setEditor(JZEditor *editor)
{
    m_editor = editor;
}

JZEditor *JZProjectItem::editor() 
{
    return m_editor;
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
    if (!m_project)
        return nullptr;

    return m_project->getItemClass(this);
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

void JZProjectItem::regist()
{
    if (m_project)
        m_project->regist(this);
}

//JZProjectItemRoot
JZProjectItemRoot::JZProjectItemRoot()
    :JZProjectItem(ProjectItem_root)
{
}

JZProjectItemRoot::~JZProjectItemRoot()
{
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