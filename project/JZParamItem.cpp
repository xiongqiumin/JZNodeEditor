#include "JZParamItem.h"
#include "JZNodeType.h"
#include "JZProject.h"

//JZScriptParamFile
JZParamItem::JZParamItem()
    :JZProjectItem(ProjectItem_param)
{

}

JZParamItem::~JZParamItem()
{

}

void JZParamItem::addVariable(QString name,int type,QVariant v)
{
    Q_ASSERT(!getVariable(name) && type != Type_none);

    JZParamDefine info;
    info.name = name;
    info.dataType = type;
    info.value = v;
    m_variables[name] = info;
    regist();
}

void JZParamItem::removeVariable(QString name)
{
    m_variables.remove(name);
    regist();
}

void JZParamItem::renameVariable(QString oldName, QString newName)
{
    Q_ASSERT(m_variables.contains(oldName));
    auto def = m_variables[oldName];
    def.name = newName;
    m_variables.remove(oldName);
    m_variables[newName] = def;
    regist();
}

void JZParamItem::setVariableType(QString name, int dataType)
{
    Q_ASSERT(m_variables.contains(name));
    m_variables[name].dataType = dataType;
    if (JZNodeType::isEnum(dataType))
    {
        auto meta = JZNodeObjectManager::instance()->enumMeta(dataType);
        m_variables[name].value = meta->value(0);
    }
    else
    {
        m_variables[name].value.clear();
    }
    regist();
}

void JZParamItem::setVariableValue(QString name, const QVariant &value)
{
    Q_ASSERT(m_variables.contains(name));
    m_variables[name].value = value;
    regist();
}

const QMap<QString,JZParamDefine> &JZParamItem::variables()
{
    return m_variables;
}

JZParamDefine *JZParamItem::getVariable(QString name)
{
    if(m_variables.contains(name))
        return &m_variables[name];
    return nullptr;
}

QStringList JZParamItem::variableList()
{
    QStringList list;
    auto it = m_variables.begin();
    while(it != m_variables.end())
    {
        list << it.key();
        it++;
    }
    return list;
}

void JZParamItem::bindVariable(QString name, QString widget)
{    
    Q_ASSERT(getClassFile());
    m_binds[name] = widget;
}

void JZParamItem::unbindVariable(QString name)
{
    m_binds.remove(name);
}

const QMap<QString, QString> &JZParamItem::binds()
{
    return m_binds;
}