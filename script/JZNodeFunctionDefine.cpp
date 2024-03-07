#include "JZNodeFunctionDefine.h"
#include "JZEvent.h"
#include "JZNodeObject.h"

//JZParamDefine
JZParamDefine::JZParamDefine()
{    
}

JZParamDefine::JZParamDefine(QString name, int dataType, const QString &v)
{
    this->name = name;
    this->type = JZNodeType::typeToName(dataType);
    this->value = v;
}

JZParamDefine::JZParamDefine(QString name, QString dataType, const QString &v)
{
    this->name = name;
    this->type = dataType;
    this->value = v;
}

int JZParamDefine::dataType() const
{
    return JZNodeType::nameToType(type);
}

QString JZParamDefine::initValue() const
{
    if (value.isEmpty())
    {
        int t = dataType();
        if (t == Type_bool)
            return "false";
        else if (JZNodeType::isNumber(t))
            return "0";
        else if (JZNodeType::isEnum(t))
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(t);
            return meta->defaultValue();
        }
        else if (JZNodeType::isObject(t))
            return "null";
        else if (t == Type_string)
            return "\"\"";
        else
            return QString();
    }
    else
    {
        return value;
    }
}

QDataStream &operator<<(QDataStream &s, const JZParamDefine &param)
{
    s << param.name;
    s << param.type;
    s << param.value;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZParamDefine &param)
{
    s >> param.name;
    s >> param.type;
    s >> param.value;
    return s;
}

//CFunction
CFunction::CFunction()
{

}

CFunction::~CFunction()
{

}

//JZFunctionDefine
JZFunctionDefine::JZFunctionDefine()
{
    isCFunction = false;
    isFlowFunction = false;          
}

bool JZFunctionDefine::isNull() const
{
    return name.isEmpty();
}

QString JZFunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "." + result;
    return result;
}

bool JZFunctionDefine::isMemberFunction() const
{
    return (paramIn.size() > 0 && paramIn[0].name == "this");
}

void JZFunctionDefine::updateParam(CFunction *func)
{
    paramIn.clear();
    paramOut.clear();
    for (int i = 0; i < func->args.size(); i++)
    {
        QString name = "input" + QString::number(i);
        int dataType = JZNodeType::typeidToType(func->args[i]);
        Q_ASSERT(dataType != Type_none);

        paramIn.push_back(JZParamDefine(name, dataType));
    }
    if (func->result != typeid(void).name())
    {
        QString name = "output";
        int dataType = JZNodeType::typeidToType(func->result);
        Q_ASSERT(dataType != Type_none);

        paramOut.push_back(JZParamDefine(name, dataType));
    }
}

void JZFunctionDefine::setDefaultValue(int index, QStringList values)
{
    for (int i = 0; i < values.size(); i++)
    {
        if (paramIn[i + index].dataType() == Type_string)
            paramIn[i + index].value = JZNodeType::addMark(values[i]);
        else
            paramIn[i + index].value = values[i];
    }
}

QDataStream &operator<<(QDataStream &s, const JZFunctionDefine &param)
{    
    s << param.name;
    s << param.className;
    s << param.isCFunction;
    s << param.isFlowFunction;
    s << param.paramIn;
    s << param.paramOut;         
    return s;
}

QDataStream &operator>>(QDataStream &s, JZFunctionDefine &param)
{
    s >> param.name;
    s >> param.className;
    s >> param.isCFunction;
    s >> param.isFlowFunction;     
    s >> param.paramIn;
    s >> param.paramOut;          
    return s;
}

//EventDefine
EventDefine::EventDefine()
{
    eventType = Event_none;
}

QDataStream &operator<<(QDataStream &s, const EventDefine &param)
{
    s << param.eventType;
    s << param.name;
    return s;
}
QDataStream &operator >> (QDataStream &s, EventDefine &param)
{
    s >> param.eventType;
    s >> param.name;
    return s;
}

//JZParam
JZParam JZParam::formDefine(const JZParamDefine &def)
{
    JZParam param;
    param.name = def.name;
    param.dataType = def.dataType();
    return param;
}

JZParam::JZParam()
{
    dataType = Type_none;
}

JZParam::JZParam(const QString &name, int type)
{
    this->name = name;
    this->dataType = type;
}

JZParamDefine JZParam::define() const
{
    JZParamDefine def;
    def.name = name;
    def.type = JZNodeType::typeToName(dataType);    
    return def;
}

QDataStream &operator<<(QDataStream &s, const JZParam &param)
{
    s << param.name;
    s << param.dataType;    
    return s;
}

QDataStream &operator>>(QDataStream &s, JZParam &param)
{
    s >> param.name;
    s >> param.dataType;    
    return s;
}

//JZFunction
JZFunction::JZFunction()
{
    addr = -1;
    addrEnd = -1;    
}

JZFunction::~JZFunction()
{

}

bool JZFunction::isCFunction() const
{
    return cfunc;
}

JZFunctionDefine JZFunction::define() const
{
    JZFunctionDefine define;
    define.name = this->name;
    define.className = this->className;
    define.isFlowFunction = this->flow;
    define.isCFunction = this->isCFunction();

    for (int i = 0; i < this->paramIn.size(); i++)
        define.paramIn << this->paramIn[i].define();
     
    for (int i = 0; i < this->paramOut.size(); i++)
        define.paramOut << this->paramOut[i].define();

    return define;
}

QString JZFunction::fullName() const
{
    if (className.isEmpty())
        return name;
    else
        return className + "." + name;
}

bool JZFunction::isMemberFunction() const
{
    return paramIn.size() > 0 && paramIn[0].name == "this";
}

bool JZFunction::isFlowFunction() const
{
    return flow;
}

QDataStream &operator<<(QDataStream &s, const JZFunction &param)
{
    s << param.name;
    s << param.className;
    s << param.paramIn;
    s << param.paramOut;
    s << param.flow;
   
    s << param.addr;
    s << param.addrEnd;
    s << param.file;
    s << param.localVariables;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZFunction &param)
{
    s >> param.name;
    s >> param.className;
    s >> param.paramIn;
    s >> param.paramOut;
    s >> param.flow;

    s >> param.addr;
    s >> param.addrEnd;
    s >> param.file;
    s >> param.localVariables;
    return s;
}