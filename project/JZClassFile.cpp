#include "JZClassFile.h"
#include "JZParamFile.h"
#include "JZProject.h"
#include "JZUiFile.h"
#include "JZNodeEvent.h"

//JZScriptClassFile
JZScriptClassFile::JZScriptClassFile()
    :JZProjectItem(ProjectItem_class)
{
    m_classId = -1;
}

JZScriptClassFile::~JZScriptClassFile()
{

}

void JZScriptClassFile::setClass(QString className, QString super)
{
    m_className = className;
    m_super = super;
}

QString JZScriptClassFile::className() const
{
    return m_className;
}

int JZScriptClassFile::classType() const
{
    return JZNodeObjectManager::instance()->getClassId(m_className);
}

void JZScriptClassFile::setClassType(int classId)
{
    m_classId = classId;
}

QString JZScriptClassFile::superClass() const
{
    return m_super;
}

void JZScriptClassFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);
    s << m_className;
    s << m_super;
    s << m_classId;
}

void JZScriptClassFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);
    s >> m_className;
    s >> m_super;
    s >> m_classId;
}

bool JZScriptClassFile::addMemberVariable(QString name,int dataType,const QVariant &v)
{    
    getParamFile()->addVariable(name,dataType,v);    
    regist();
    return true;
}

void JZScriptClassFile::removeMemberVariable(QString name)
{
    getParamFile()->removeVariable(name);    
    regist();
}

JZParamDefine *JZScriptClassFile::memberVariableInfo(QString name)
{
    return getParamFile()->getVariable(name);
}

JZScriptFile *JZScriptClassFile::addMemberFunction(FunctionDefine func)
{
    Q_ASSERT(func.paramIn.size() > 0 && func.paramIn[0].name == "this");

    JZScriptFile *file = new JZScriptFile(ProjectItem_scriptFunction);
    func.className = m_className;
    file->setFunction(func);
    m_project->addItem(itemPath(), file);        
    return file;
}

JZScriptFile *JZScriptClassFile::getMemberFunction(QString func)
{
    auto list = itemList(ProjectItem_scriptFunction);
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i]->name() == func)
            return (JZScriptFile *)list[i];
    }
    return nullptr;
}

void JZScriptClassFile::removeMemberFunction(QString func)
{
    JZScriptFile *item = getMemberFunction(func);
    m_project->removeItem(item->itemPath());    
}

QList<JZParamDefine> JZScriptClassFile::uiWidgets()
{
    QList<JZParamDefine> list;
    auto item_list = itemList(ProjectItem_ui);
    if (item_list.size() > 0)
    {
        auto ui_item = dynamic_cast<JZUiFile*>(item_list[0]);
        list = ui_item->widgets();
    }
    return list;
}

JZParamFile *JZScriptClassFile::getParamFile()
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        auto item = m_childs[i].data();
        if(item->itemType() == ProjectItem_param)
            return dynamic_cast<JZParamFile*>(item);
    }
    Q_ASSERT(0);
    return nullptr;
}

JZNodeObjectDefine JZScriptClassFile::objectDefine()
{    
    JZNodeObjectDefine define;
    define.className = m_className;
    define.superName = m_super;
    define.id = m_classId;

    auto item_list = itemList(ProjectItem_any);
    for(int i = 0; i < item_list.size(); i++)
    {
        auto item = item_list[i];
        if(item->itemType() == ProjectItem_param)
        {
            auto param_item = dynamic_cast<JZParamFile*>(item);
            define.params = param_item->variables();
        }
        else if(item->itemType() == ProjectItem_scriptFunction)
        {
            auto function_item = dynamic_cast<JZScriptFile*>(item);
            define.addFunction(function_item->function());
        }
        else if(item->itemType() == ProjectItem_ui)
        {
            auto ui_item = dynamic_cast<JZUiFile*>(item);
            define.isUiWidget = true;
            define.widgteXml = ui_item->xml();
            
            QList<JZParamDefine> widget_list = ui_item->widgets();
            for (int i = 0; i < widget_list.size(); i++)
            {
                define.params[widget_list[i].name] = widget_list[i];
            }
        }
    }
    return define;
}

