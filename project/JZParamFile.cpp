#include "JZParamFile.h"
#include "JZNodeType.h"

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
    Q_ASSERT(type != Type_none);

    JZParamDefine info;
    info.name = name;
    info.dataType = type;
    info.value = v;
    m_variables[name] = info;
}

void JZParamFile::removeVariable(QString name)
{
    m_variables.remove(name);
}

QMap<QString,JZParamDefine> JZParamFile::variables()
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
