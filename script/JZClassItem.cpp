#include "JZClassItem.h"
#include "JZParamItem.h"
#include "JZProject.h"
#include "JZUiItem.h"
#include "JZNodeEvent.h"

//JZScriptClassItem
JZScriptClassItem::JZScriptClassItem()
    :JZProjectItem(ProjectItem_class)
{
}

JZScriptClassItem::~JZScriptClassItem()
{

}

void JZScriptClassItem::saveToStream(QDataStream &s) const   
{
    s << m_name;    
    s << m_super;
}

bool JZScriptClassItem::loadFromStream(QDataStream &s)
{
    s >> m_name;
    s >> m_super;
    return true;
}


void JZScriptClassItem::setClass(QString className, QString super)
{
    m_name = className;    
    m_super = super;    
}

QString JZScriptClassItem::className() const
{
    return m_name;
}


JZUiItem *JZScriptClassItem::ui()
{
    auto list = itemList(ProjectItem_ui);
    if (list.size() == 0)
        return nullptr;
    
    return dynamic_cast<JZUiItem*>(list[0]);
}

bool JZScriptClassItem::hasUi()
{
    return ui();
}

void JZScriptClassItem::addUi(JZUiItem *item)
{
    Q_ASSERT(!ui());
    addItem(item);
}

void JZScriptClassItem::removeUi()
{
    auto ui_item = ui();
    if(ui_item)
        removeItem(ui_item);
}

int JZScriptClassItem::classType()
{
    return project()->environment()->objectManager()->getClassId(m_name);
}

QString JZScriptClassItem::superClass() const
{
    return m_super;
}

bool JZScriptClassItem::addMemberVariable(QString name,int dataType,const QString &v)
{    
    auto env = project()->environment();
    return addMemberVariable(name, env->typeToName(dataType), v);
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
    auto ui_item = ui();
    if (hasUi && ui_item)
    {
        auto widgets = ui_item->widgets();
        for (int i = 0; i < widgets.size(); i++)
            list << widgets[i].name;
    }

    return list;
}

const JZParamDefine *JZScriptClassItem::memberVariable(QString name, bool hasUi)
{
    auto def = paramFile()->variable(name);
    if (def)
        return def;

    auto ui_item = ui();
    if (hasUi && ui_item)
    {
        return ui_item->widgetVariable(name);
    }
    
    return nullptr;
}

const JZParamDefine *JZScriptClassItem::memberThis()
{
    m_this.name = "this";
    m_this.type = JZNodeType::pointerType(m_name);
    return &m_this;
}

JZScriptItem *JZScriptClassItem::addMemberFunction(JZFunctionDefine func)
{
    Q_ASSERT(!func.isCFunction && !func.name.isEmpty() && func.className == m_name);

    JZScriptItem *file = new JZScriptItem(JZScriptItem::Function);
    file->setFunction(func);
    project()->addItem(itemPath(), file);
    return file;
}

QStringList JZScriptClassItem::memberFunctionList()
{
    QStringList list;
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->itemType() == ProjectItem_scriptItem && isFunctionScriptItem(m_childs[i]))
            list << m_childs[i]->name();
    }
    return list;
}

JZScriptItem *JZScriptClassItem::memberFunction(QString func)
{    
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->name() == func && isFunctionScriptItem(m_childs[i]))
            return (JZScriptItem *)m_childs[i];
    }
    return nullptr;
}

void JZScriptClassItem::removeMemberFunction(QString func)
{
    JZScriptItem *item = memberFunction(func);
    project()->removeItem(item->itemPath());
}

JZScriptItem* JZScriptClassItem::addFlow(QString name)
{
    JZScriptItem* file = new JZScriptItem(JZScriptItem::Flow);
    file->setName(name);
    bool ret = project()->addItem(itemPath(), file);
    Q_ASSERT(ret);
    return file;
}

void JZScriptClassItem::removeFlow(QString name)
{
    auto file = flow(name);
    if (file)
        project()->removeItem(file->itemPath());
}

JZScriptItem* JZScriptClassItem::flow(QString name)
{
    auto item = getItem(name);
    if (item && isFlowScriptItem(item))
        return (JZScriptItem*)item;
    else
        return nullptr;
}

QStringList JZScriptClassItem::flowList()
{
    QStringList list;
    for (int i = 0; i < m_childs.size(); i++)
    {
        if (m_childs[i]->itemType() == ProjectItem_scriptItem && isFlowScriptItem(m_childs[i]))
            list << m_childs[i]->name();
    }
    return list;
}

QList<JZParamDefine> JZScriptClassItem::uiWidgets()
{
    QList<JZParamDefine> list;

    auto ui_item = ui();
    if (ui_item)
    {
        list = ui_item->widgets();
    }
    return list;
}

JZParamItem *JZScriptClassItem::paramFile()
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        auto item = m_childs[i];
        if(item->itemType() == ProjectItem_param)
            return dynamic_cast<JZParamItem*>(item);
    }
    Q_ASSERT(0);
    return nullptr;
}

JZNodeObjectDefine JZScriptClassItem::objectDefine()
{    
    JZNodeObjectManager *obj_inst = nullptr;
    if(project())
        obj_inst = project()->environment()->objectManager();

    JZNodeObjectDefine define;
    define.className = m_name;
    define.superName = m_super;
    define.id = obj_inst->getClassId(m_name);
    define.manager = obj_inst;

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
        else if (item->itemType() == ProjectItem_scriptItem)
        {
            auto function_item = dynamic_cast<JZScriptItem*>(item);
            if(isFunctionScriptItem(function_item))
                define.addFunction(function_item->function());
        }
    }
     
    JZUiItem* ui_item = ui();
    if(ui_item)
    {        
        define.isUiWidget = true;
        define.widgetXml = ui_item->xml();
        define.widgetParams = ui_item->widgets();
            
        QList<JZParamDefine> widget_list = ui_item->widgets();
        for (int i = 0; i < widget_list.size(); i++)
        {
            define.params[widget_list[i].name] = widget_list[i];
        }

        auto param_file = paramFile();
        auto bind_list = param_file->bindVariableList();
        for (int i = 0; i < bind_list.size(); i++)
        {
            auto bind = param_file->bindVariable(bind_list[i]);
            define.widgetBind[bind->widget] = *bind;
        }       
    }    
    return define;
}

