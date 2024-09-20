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
    s << m_binds;
    return buffer;
}

bool JZParamItem::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);
    s >> m_name;
    s >> m_variables;
    s >> m_binds;
    return true;
}

void JZParamItem::addVariable(QString name, QString type, const QString &v)
{    
    Q_ASSERT(!m_variables.contains(name));

    JZParamDefine info;
    info.name = name;
    info.type = type;
    info.value = v;
    m_variables[name] = info;    
}

void JZParamItem::addVariable(QString name,int type, const QString &v)
{
    QString type_name = JZNodeType::typeToName(type);
    addVariable(name, type_name, v);
}

void JZParamItem::addVariable(JZParamDefine define)
{
    addVariable(define.name, define.type, define.value);
}

void JZParamItem::removeVariable(QString name)
{
    m_variables.remove(name);        
}

void JZParamItem::setVariable(QString name, JZParamDefine define)
{
    Q_ASSERT(m_variables.contains(name));
    if (name != define.name)
    {
        Q_ASSERT(!m_variables.contains(define.name));
        m_variables.remove(name);        
    }
    m_variables[define.name] = define;    
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


void JZParamItem::addBind(JZNodeParamBind info)
{
    m_binds[info.widget] = info;
}

void JZParamItem::removeBind(QString name)
{
    m_binds.remove(name);
}

QStringList JZParamItem::bindVariableList()
{
    return m_binds.keys();
}

JZNodeParamBind *JZParamItem::bindVariable(QString name)
{
    auto it = m_binds.find(name);
    if (it == m_binds.end())
        return nullptr;

    return &it.value();
}