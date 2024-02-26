#include "JZScriptFile.h"
#include "JZProject.h"

JZScriptFile::JZScriptFile()
    :JZProjectItem(ProjectItem_scriptFile)
{
}

JZScriptFile::~JZScriptFile()
{

}

bool JZScriptFile::loadFromStream(JZProjectStream &s)
{
    return true;
}

bool JZScriptFile::saveToStream(JZProjectStream &s)
{
    return true;
}

JZParamDefine *JZScriptFile::addParamDefine(QString name)
{
    JZParamDefine *def = nullptr;
    return def;
}

void JZScriptFile::removeParamDefine(QString name)
{

}

JZScriptItem *JZScriptFile::addFlow(QString name)
{
    return nullptr;
}

void JZScriptFile::removeFlow(QString name)
{

}

JZScriptItem *JZScriptFile::addFunction(QString path, const FunctionDefine &define)
{
    Q_ASSERT(!define.name.isEmpty() && getItem(path));

    JZScriptItem *file = new JZScriptItem(ProjectItem_scriptFunction);
    file->setName(define.name);
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

    JZParamItem *data_page = new JZParamItem();
    JZScriptItem *script_flow = new JZScriptItem(ProjectItem_scriptFlow);
    JZProjectItemFolder *script_function = new JZProjectItemFolder();
    class_file->setName(name);
    data_page->setName("变量");
    script_flow->setName("事件");    
    

    return class_file;
}

void JZScriptFile::removeClass(QString name)
{
    m_project->removeItem(name);
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