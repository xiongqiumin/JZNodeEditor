#include <QFileInfo>
#include "JZScriptFile.h"
#include "JZProject.h"

JZScriptFile::JZScriptFile()
    :JZProjectItem(ProjectItem_scriptFile)
{
}

JZScriptFile::~JZScriptFile()
{

}

bool JZScriptFile::save(QString filepath)
{
    QList<JZProjectItem*> list = itemList(ProjectItem_any);
    return save(filepath, list);
}

bool JZScriptFile::save(QString filepath, QList<JZProjectItem*> change_items)
{   
    QFile sub_file(filepath);
    if (!sub_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    
    for (int i = 0; i < change_items.size(); i++)            
        m_itemCache.remove(change_items[i]);    
    
    QList<QByteArray> class_buffer;           
    auto all_class = itemList(ProjectItem_class);
    for(int i = 0; i < all_class.size(); i++)    
        class_buffer << getClassData((JZScriptClassItem*)all_class[i]);

    QDataStream s(&sub_file);
    s << class_buffer;
    saveScript(s, this);
    return true;
}

void JZScriptFile::updateClass(JZScriptClassItem *item, JZNodeObjectDefine define)
{
    m_itemCache.remove(item);    
    m_project->saveItem(this);
}

void JZScriptFile::updateScriptName(JZScriptItem *item, QString name)
{
    QByteArray buffer = getItemData(item);    
    JZScriptItem tmp(item->itemType());
    tmp.fromBuffer(buffer);
    tmp.setName(name);
    item->setName(name);
    m_itemCache[item] = tmp.toBuffer();
    m_project->saveItem(this);
}

void JZScriptFile::updateScriptFunction(JZScriptItem *item, JZFunctionDefine define)
{
    QByteArray buffer = getItemData(item);
    JZScriptItem tmp(item->itemType());
    tmp.fromBuffer(buffer);
    tmp.setFunction(define);
    item->setFunction(define);
    m_itemCache[item] = tmp.toBuffer();
    m_project->saveItem(this);
}

bool JZScriptFile::load(QString filepath)
{
    QFile sub_file(filepath);
    if (!sub_file.open(QIODevice::ReadOnly))
        return false;

    QDataStream s(&sub_file);

    QList<QByteArray> class_list;
    s >> class_list;
    for (int i = 0; i < class_list.size(); i++)
    {
        QByteArray cls_buffer = class_list[i];
        QDataStream cls_s(&cls_buffer, QIODevice::ReadOnly);

        QByteArray head;
        cls_s >> head;

        JZScriptClassItem *class_item = new JZScriptClassItem();
        class_item->fromBuffer(head);
        m_itemCache[class_item] = head;
        m_project->addItem(itemPath(), class_item);                

        loadScript(cls_s, class_item);        
    }
    loadScript(s, this);
    return true;
}

QByteArray JZScriptFile::getClassData(JZScriptClassItem *item)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    QByteArray head = getItemData(item);
    s << head;
    saveScript(s, item);
    return buffer;
}

QByteArray JZScriptFile::getItemData(JZProjectItem *item)
{
    if (!m_itemCache.contains(item))
    {
        QByteArray buffer;        
        if (item->itemType() == ProjectItem_class)
        {
            auto class_item = dynamic_cast<JZScriptClassItem*>(item);
            buffer = class_item->toBuffer();
        }
        else if (item->itemType() == ProjectItem_param)
        {
            auto param_item = dynamic_cast<JZParamItem*>(item);
            buffer = param_item->toBuffer();
        }
        else if (item->itemType() == ProjectItem_scriptFunction
            || item->itemType() == ProjectItem_scriptFlow)
        {
            auto script = dynamic_cast<JZScriptItem*>(item);
            buffer = script->toBuffer();
        }
        m_itemCache[item] = buffer;
    }
    return m_itemCache[item];
}

void JZScriptFile::saveScript(QDataStream &s, JZProjectItem *parent)
{        
    QList<QByteArray> function_list, param_list, flow_list;
    auto items = parent->childs();
    for (int i = 0; i < items.size(); i++)
    {
        auto item = items[i];

        JZProjectStream s;
        if (item->itemType() == ProjectItem_param)
            param_list << getItemData(item);        
        else if (item->itemType() == ProjectItem_scriptFunction)
            function_list << getItemData(item);
        else if (item->itemType() == ProjectItem_scriptFlow)
            flow_list << getItemData(item);
    }
    
    s << param_list;
    s << function_list;
    s << flow_list;
}

void JZScriptFile::loadScript(QDataStream &s, JZProjectItem *parent)
{    
    Q_ASSERT(parent->project());

    QList<QByteArray> function_list, param_list, flow_list;
    s >> param_list;
    s >> function_list;    
    s >> flow_list;

    for (int i = 0; i < param_list.size(); i++)
    {
        JZParamItem *item = new JZParamItem();
        item->fromBuffer(param_list[i]);
        m_project->addItem(parent->itemPath(),item);
        m_itemCache[item] = param_list[i];
    }

    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *item = new JZScriptItem(ProjectItem_scriptFunction);
        item->fromBuffer(function_list[i]);
        m_project->addItem(parent->itemPath(), item);
        m_itemCache[item] = function_list[i];
    }

    for (int i = 0; i < flow_list.size(); i++)
    {
        JZScriptItem *item = new JZScriptItem(ProjectItem_scriptFlow);        
        item->fromBuffer(flow_list[i]);
        m_project->addItem(parent->itemPath(), item);
        m_itemCache[item] = flow_list[i];
    } 
}

JZParamItem *JZScriptFile::addParamDefine(QString name)
{
    JZParamItem *def = new JZParamItem();
    def->setName(name);
    m_project->addItem(itemPath(), def);
    return def;
}

void JZScriptFile::removeParamDefine(QString name)
{
    auto path = getItem(name)->itemPath();
    m_project->removeItem(path);
}

JZParamItem *JZScriptFile::paramDefine(QString name)
{
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->name() == name && m_childs[i]->itemType() == ProjectItem_param)
            return (JZParamItem *)m_childs[i].data();
    }
    return nullptr;
}

JZScriptItem *JZScriptFile::addFlow(QString name)
{
    JZScriptItem *item = new JZScriptItem(ProjectItem_scriptFlow);
    item->setName(name);
    m_project->addItem(itemPath(), item);
    return item;
}

void JZScriptFile::removeFlow(QString name)
{
    auto path = getItem(name)->itemPath();
    m_project->removeItem(path);
}

JZScriptItem *JZScriptFile::flow(QString name)
{
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->name() == name && m_childs[i]->itemType() == ProjectItem_scriptFlow)
            return (JZScriptItem *)m_childs[i].data();
    }
    return nullptr;
}

JZScriptItem *JZScriptFile::addFunction(QString path, const JZFunctionDefine &define)
{    
    JZScriptItem *file = new JZScriptItem(ProjectItem_scriptFunction);    
    file->setFunction(define);
    m_project->addItem(path, file);
    return file;
}

void JZScriptFile::removeFunction(QString name)
{
    auto func = getFunction(name);
    if (func)
        m_project->removeItem(func->itemPath());
}

JZScriptItem *JZScriptFile::getFunction(QString name)
{
    name.replace(".", "/");
    auto item = getItem(name);
    if (item && item->itemType() == ProjectItem_scriptFunction)
        return (JZScriptItem *)item;
    else
        return nullptr;
}

JZScriptClassItem *JZScriptFile::addClass(QString name, QString super)
{
    QString flow = name + ".jz";

    JZScriptClassItem *class_file = new JZScriptClassItem();
    class_file->setClass(name, super);
    m_project->addItem(itemPath(), class_file);

    JZParamItem *data_page = new JZParamItem();            
    data_page->setName("成员变量");
    m_project->addItem(class_file->itemPath(), data_page);

    return class_file;
}

void JZScriptFile::removeClass(QString name)
{
    m_project->removeItem(getItem(name)->itemPath());
}

JZScriptClassItem *JZScriptFile::getClass(QString name)
{
    QList<JZProjectItem*> list = itemList(ProjectItem_class);
    for (int i = 0; i < list.size(); i++)
    {
        JZScriptClassItem *file = dynamic_cast<JZScriptClassItem*>(list[i]);
        if (file->name() == name)
            return file;
    }
    return nullptr;
}