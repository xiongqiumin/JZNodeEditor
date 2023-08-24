#include "JZParamFile.h"
#include "JZNodeType.h"
#include "JZProject.h"

//JZScriptParamFile
JZParamFile::JZParamFile()
    :JZProjectItem(ProjectItem_param)
{

}

JZParamFile::~JZParamFile()
{

}

void JZParamFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);
    s << m_variables;
}

void JZParamFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);
    s >> m_variables;
}

void JZParamFile::addVariable(QString name,int type,QVariant v)
{
    Q_ASSERT(!getVariable(name) && type != Type_none);

    JZParamDefine info;
    info.name = name;
    info.dataType = type;
    info.value = v;
    m_variables[name] = info;
    regist();
}

void JZParamFile::removeVariable(QString name)
{
    m_variables.remove(name);
    regist();
}

void JZParamFile::renameVariable(QString oldName, QString newName)
{
    Q_ASSERT(m_variables.contains(oldName));
    auto def = m_variables[oldName];
    def.name = newName;
    m_variables.remove(oldName);
    m_variables[newName] = def;
    regist();
}

void JZParamFile::setVariableType(QString name, int dataType)
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

void JZParamFile::setVariableValue(QString name, const QVariant &value)
{
    Q_ASSERT(m_variables.contains(name));
    m_variables[name].value = value;
    regist();
}

const QMap<QString,JZParamDefine> &JZParamFile::variables()
{
    return m_variables;
}

JZParamDefine *JZParamFile::getVariable(QString name)
{
    if(m_variables.contains(name))
        return &m_variables[name];
    return nullptr;
}

QStringList JZParamFile::variableList()
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