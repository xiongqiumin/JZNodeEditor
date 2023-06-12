#include "JZParamFile.h"
#include "JZNodeType.h"

//JZScriptParamFile
JZScriptParamDefineFile::JZScriptParamDefineFile()
    :JZProjectItem(ProjectItem_param)
{

}

JZScriptParamDefineFile::~JZScriptParamDefineFile()
{

}

void JZScriptParamDefineFile::addVariable(QString name,int type,QVariant v)
{
    Q_ASSERT(type != Type_none);

    JZParamDefine info;
    info.name = name;
    info.dataType = type;
    info.value = v;
    m_variables[name] = info;
}

void JZScriptParamDefineFile::removeVariable(QString name)
{
    m_variables.remove(name);
}

JZParamDefine *JZScriptParamDefineFile::getVariable(QString name)
{
    if(m_variables.contains(name))
        return &m_variables[name];
    return nullptr;
}

QStringList JZScriptParamDefineFile::variableList()
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
