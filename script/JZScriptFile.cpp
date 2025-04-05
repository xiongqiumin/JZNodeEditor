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


void JZScriptFile::saveToStream(QDataStream &s) const
{

}

bool JZScriptFile::loadFromStream(QDataStream &s)
{
    return true;
}

bool JZScriptFile::save(QString filepath)
{  
    QFile sub_file(filepath);
    if (!sub_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QDataStream s(&sub_file);
    s << toBuffer();
    return true;
}

bool JZScriptFile::load(QString filepath)
{
    QFile sub_file(filepath);
    if (!sub_file.open(QIODevice::ReadOnly))
        return false;

    QFileInfo info(filepath);

    QByteArray buffer;
    QDataStream s(&sub_file);
    s >> buffer;
    fromBuffer(buffer);
    m_name = info.fileName();
    return true;
}

JZParamItem *JZScriptFile::addParamDefine(QString name)
{
    JZParamItem *def = new JZParamItem();
    def->setName(name);
    project()->addItem(itemPath(), def);
    return def;
}

void JZScriptFile::removeParamDefine(QString name)
{
    auto path = getItem(name)->itemPath();
    project()->removeItem(path);
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
    project()->addItem(itemPath(), file);
    return file;
}

void JZScriptFile::removeFunction(QString name)
{
    auto func = getFunction(name);
    if (func)
        project()->removeItem(func->itemPath());
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
    project()->addItem(itemPath(), class_file);

    JZParamItem *data_page = new JZParamItem();            
    data_page->setName("param");
    project()->addItem(class_file->itemPath(), data_page);

    return class_file;
}

void JZScriptFile::removeClass(QString name)
{
    project()->removeItem(getItem(name)->itemPath());
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