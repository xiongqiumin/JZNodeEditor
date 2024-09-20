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
    QFile sub_file(filepath);
    if (!sub_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    
    QList<QByteArray> class_buffer;           
    auto all_class = itemList(ProjectItem_class);
    for(int i = 0; i < all_class.size(); i++)    
        class_buffer << getClassData((JZScriptClassItem*)all_class[i]);

    QDataStream s(&sub_file);
    s << class_buffer;
    saveScript(s, this);
    return true;
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
        || item->itemType() == ProjectItem_scriptParamBinding)
    {
        auto script = dynamic_cast<JZScriptItem*>(item);
        buffer = script->toBuffer();
    }
    
    return buffer;
}

void JZScriptFile::setItemData(JZProjectItem *item, const QByteArray &data)
{
    if (item->itemType() == ProjectItem_class)
    {
        auto class_item = dynamic_cast<JZScriptClassItem*>(item);
        class_item->fromBuffer(data);
    }
    else if (item->itemType() == ProjectItem_param)
    {
        auto param_item = dynamic_cast<JZParamItem*>(item);
        param_item->fromBuffer(data);
    }
    else if (item->itemType() == ProjectItem_scriptFunction
        || item->itemType() == ProjectItem_scriptParamBinding)
    {
        auto script = dynamic_cast<JZScriptItem*>(item);
        script->fromBuffer(data);
    }
}

void JZScriptFile::saveScript(QDataStream &s, JZProjectItem *parent)
{        
    QList<QByteArray> function_list, param_list;
    auto items = parent->childs();
    for (int i = 0; i < items.size(); i++)
    {
        auto item = items[i];

        if (item->itemType() == ProjectItem_param)
            param_list << getItemData(item);        
        else if (item->itemType() == ProjectItem_scriptFunction)
            function_list << getItemData(item);
    }
    
    s << param_list;
    s << function_list;
}

void JZScriptFile::loadScript(QDataStream &s, JZProjectItem *parent)
{    
    Q_ASSERT(parent->project());

    QList<QByteArray> function_list, param_list;
    s >> param_list;
    s >> function_list;

    for (int i = 0; i < param_list.size(); i++)
    {
        JZParamItem *item = new JZParamItem();
        item->fromBuffer(param_list[i]);
        m_project->addItem(parent->itemPath(),item);
    }

    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *item = new JZScriptItem(ProjectItem_scriptFunction);
        item->fromBuffer(function_list[i]);
        m_project->addItem(parent->itemPath(), item);
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

QStringList JZScriptFile::functionList() const
{
    QStringList functions;
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->itemType() == ProjectItem_scriptFunction)
            functions << m_childs[i]->name();
    }
    return functions;
}

JZScriptItem *JZScriptFile::addFunction(const JZFunctionDefine &define)
{    
    JZScriptItem *file = new JZScriptItem(ProjectItem_scriptFunction);    
    file->setFunction(define);
    m_project->addItem(itemPath(), file);
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
    data_page->setName("param");
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

QSharedPointer<JZScriptFile> createTempFile(JZProject *project)
{
    if (!project)
        project = JZProject::active();

    auto ptr = QSharedPointer<JZScriptFile>(new JZScriptFile(), [](JZScriptFile *file){
        if (file->project())
            file->project()->removeItem(file->itemPath());
        else
            delete file;

    });
    return ptr;
}