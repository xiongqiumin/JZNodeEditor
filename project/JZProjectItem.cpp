#include "JZProjectItem.h"
#include "JZScriptFile.h"
#include "JZUiFile.h"
#include "JZParamFile.h"
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

JZProject *JZProjectItem::project() const
{
    return m_project;
}

void JZProjectItem::setEditor(JZEditor *editor)
{
    m_editor = editor;
}

JZEditor *JZProjectItem::editor() const
{
    return m_editor;
}

void JZProjectItem::save()
{
    if(m_project)
        m_project->saveItem(this);
}

void JZProjectItem::load()
{
    if(m_project)
        m_project->loadItem(this);
}

void JZProjectItem::saveToStream(QDataStream &s)
{
    s << m_name;    
}

void JZProjectItem::loadFromStream(QDataStream &s)
{
    s >> m_name;
}

void JZProjectItem::sort()
{
    std::sort(m_childs.begin(),m_childs.end(),[](const JZProjectItemPtr &i1_ptr,const JZProjectItemPtr &i2_ptr){
        auto i1 = i1_ptr.data();
        auto i2 = i2_ptr.data();
        if(i1->m_pri != i2->m_pri)
            return i1->m_pri < i2->m_pri;
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

JZScriptClassFile *JZProjectItem::getClassFile()
{
    if (!m_project)
        return nullptr;

    return m_project->getClassFile(this);
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
    QList<JZProjectItem *> result;
    if(this->itemType() & type)
        result << this;

    auto chlid = this->childs();
    for(int i = 0; i < chlid.size(); i++)
        result << chlid[i]->itemList(type);
        
    return result;
}

void JZProjectItem::regist()
{
    if (m_project)
        m_project->regist(this);
}


//JZProjectItemFolder
JZProjectItemFolder::JZProjectItemFolder()
    :JZProjectItem(ProjectItem_folder)
{
}

JZProjectItemFolder::~JZProjectItemFolder()
{

}

QByteArray JZProjectItemFactory::save(JZProjectItem *item)
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    s << item->itemType();
    item->saveToStream(s);
    return buffer;
}

JZProjectItem *JZProjectItemFactory::load(const QByteArray &buffer)
{
    QDataStream s(buffer);
    int itemType;
    s >> itemType;

    JZProjectItem *item = nullptr;
    if(itemType == ProjectItem_folder)
        item = new JZProjectItem(ProjectItem_folder);
    else if(itemType == ProjectItem_ui)
        item = new JZUiFile();
    else if(itemType == ProjectItem_class)
        item = new JZScriptClassFile();
    else if(itemType == ProjectItem_library)
        item = new JZScriptLibraryFile();
    else if(itemType == ProjectItem_param)
        item = new JZParamFile();
    else if(itemType >= ProjectItem_scriptParamBinding && itemType <= ProjectItem_scriptFunction)
        item = new JZScriptFile(itemType);
    else
    {
        Q_ASSERT(0);
    }
    item->loadFromStream(s);
    return item;
}

QString JZProjectItemFactory::itemTypeName(int itemType)
{
    if(itemType == ProjectItem_scriptParamBinding)
        return "变量绑定";
    else if(itemType == ProjectItem_scriptFlow)
        return "流程";

    return QString();
}
