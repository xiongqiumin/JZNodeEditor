#include "JZClassItem.h"
#include "JZParamItem.h"
#include "JZProject.h"
#include "JZUiFile.h"
#include "JZNodeEvent.h"

//JZScriptClassItem
JZScriptClassItem::JZScriptClassItem()
    :JZProjectItem(ProjectItem_class)
{
    m_classId = -1;
}

JZScriptClassItem::~JZScriptClassItem()
{

}

QByteArray JZScriptClassItem::toBuffer()
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << m_name;    
    s << m_super;
    s << m_uiFile;
    s << m_classId;
    return buffer;
}

bool JZScriptClassItem::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);
    s >> m_name;
    s >> m_super;
    s >> m_uiFile;
    s >> m_classId;
    return true;
}


void JZScriptClassItem::setClass(QString className, QString super)
{
    m_name = className;    
    m_super = super;
    itemChangedNotify();
}

QString JZScriptClassItem::className() const
{
    return m_name;
}

void JZScriptClassItem::setUiFile(QString uiFile)
{
    m_uiFile = uiFile;
    itemChangedNotify();
}

QString JZScriptClassItem::uiFile() const
{
    return m_uiFile;
}

int JZScriptClassItem::classType() const
{
    return JZNodeObjectManager::instance()->getClassId(m_name);
}

void JZScriptClassItem::setClassType(int classId)
{
    m_classId = classId;
}

QString JZScriptClassItem::superClass() const
{
    return m_super;
}

bool JZScriptClassItem::addMemberVariable(QString name,int dataType,const QString &v)
{    
    return addMemberVariable(name, JZNodeType::typeToName(dataType), v);    
}

bool JZScriptClassItem::addMemberVariable(QString name, QString dataType, const QString &v)
{
    paramFile()->addVariable(name, dataType, v);    
    return true;
}

bool JZScriptClassItem::addMemberVariable(JZParamDefine param)
{
    paramFile()->addVariable(param);
    return true;
}

void JZScriptClassItem::removeMemberVariable(QString name)
{
    paramFile()->removeVariable(name);       
}

QStringList JZScriptClassItem::memberVariableList(bool hasUi)
{
    QStringList list = paramFile()->variableList();
    if (hasUi && !m_uiFile.isEmpty())
    {
        auto ui_item = dynamic_cast<JZUiFile*>(m_project->getItem(m_uiFile));
        if (ui_item)
        {
            auto widgets = ui_item->widgets();
            for (int i = 0; i < widgets.size(); i++)
                list << widgets[i].name;
        }
    }

    return list;
}

const JZParamDefine *JZScriptClassItem::memberVariable(QString name, bool hasUi)
{
    auto def = paramFile()->variable(name);
    if (def)
        return def;

    if (hasUi && !m_uiFile.isEmpty())
    {
        auto ui_item = dynamic_cast<JZUiFile*>(m_project->getItem(m_uiFile));
        if (ui_item)        
            return ui_item->widgetVariable(name);
    }
    
    return nullptr;
}

JZScriptItem *JZScriptClassItem::addMemberFunction(JZFunctionDefine func)
{
    Q_ASSERT(!func.isCFunction && !func.name.isEmpty());

    JZScriptItem *file = new JZScriptItem(ProjectItem_scriptFunction);
    func.className = m_name;
    file->setFunction(func);
    m_project->addItem(itemPath(), file);        
    return file;
}

QStringList JZScriptClassItem::memberFunctionList()
{
    QStringList list;
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->itemType() == ProjectItem_scriptFunction)
            list << m_childs[i]->name();
    }
    return list;
}

JZScriptItem *JZScriptClassItem::memberFunction(QString func)
{    
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->name() == func && m_childs[i]->itemType() == ProjectItem_scriptFunction)
            return (JZScriptItem *)m_childs[i].data();
    }
    return nullptr;
}

void JZScriptClassItem::removeMemberFunction(QString func)
{
    JZScriptItem *item = memberFunction(func);
    m_project->removeItem(item->itemPath());    
}

QList<JZParamDefine> JZScriptClassItem::uiWidgets()
{
    QList<JZParamDefine> list;
    if (!m_uiFile.isEmpty())
    {
        auto ui_item = dynamic_cast<JZUiFile*>(m_project->getItem(m_uiFile));
        if(ui_item)
            list = ui_item->widgets();
    }
    return list;
}

JZParamItem *JZScriptClassItem::paramFile()
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        auto item = m_childs[i].data();
        if(item->itemType() == ProjectItem_param)
            return dynamic_cast<JZParamItem*>(item);
    }
    Q_ASSERT(0);
    return nullptr;
}

JZNodeObjectDefine JZScriptClassItem::objectDefine()
{    
    JZNodeObjectDefine define;
    define.className = m_name;
    define.superName = m_super;
    define.id = m_classId;

    auto item_list = itemList(ProjectItem_any);
    for (int item_idx = 0; item_idx < item_list.size(); item_idx++)
    {
        auto item = item_list[item_idx];
        if (item->itemType() == ProjectItem_param)
        {
            auto param_item = dynamic_cast<JZParamItem*>(item);
            auto var_list = param_item->variableList();
            for (int i = 0; i < var_list.size(); i++)
                define.params[var_list[i]] = *param_item->variable(var_list[i]);
        }
        else if (item->itemType() == ProjectItem_scriptFunction)
        {
            auto function_item = dynamic_cast<JZScriptItem*>(item);
            define.addFunction(function_item->function());
        }
    }
        
    if(!m_uiFile.isEmpty())
    {        
        auto ui_item = dynamic_cast<JZUiFile*>(m_project->getItem(m_uiFile));
        define.isUiWidget = true;
        define.widgetXml = ui_item->xml();
        define.widgetParams = ui_item->widgets();
            
        QList<JZParamDefine> widget_list = ui_item->widgets();
        for (int i = 0; i < widget_list.size(); i++)
        {
            define.params[widget_list[i].name] = widget_list[i];
        }
    }    
    return define;
}

