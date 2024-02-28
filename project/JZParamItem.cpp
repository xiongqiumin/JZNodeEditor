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

QByteArray JZParamItem::toBuffer()
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << m_name;    
    s << m_variables;
    return buffer;
}

bool JZParamItem::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);
    s >> m_name;
    s >> m_variables;
    return true;
}

void JZParamItem::addVariable(QString name, QString type, const QString &v)
{    
    JZParamDefine info;
    info.name = name;
    info.type = type;
    info.value = v;
    m_variables[name] = info;
    regist();
}

void JZParamItem::addVariable(QString name,int type, const QString &v)
{
    QString type_name = JZNodeType::typeToName(type);
    addVariable(name, type_name, v);
}

void JZParamItem::removeVariable(QString name)
{
    m_variables.remove(name);
    regist();
}

const JZParamDefine *JZParamItem::variable(QString name) const
{
    auto it = m_variables.find(name);
    if (it == m_variables.end())
        return nullptr;

    return &it.value();
}

QStringList JZParamItem::variableList()
{    
    return m_variables.keys();
}

void JZParamItem::bindVariable(QString name, QString widget)
{        
    m_binds[name] = widget;
}

void JZParamItem::unbindVariable(QString name)
{
    m_binds.remove(name);
}