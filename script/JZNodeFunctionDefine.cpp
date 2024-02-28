﻿#include "JZNodeFunctionDefine.h"
#include "JZEvent.h"

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

//FunctionDefine
FunctionDefine::FunctionDefine()
{
    isCFunction = false;
    isFlowFunction = false;          
}

bool FunctionDefine::isNull() const
{
    return name.isEmpty();
}

QString FunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "." + result;
    return result;
}

bool FunctionDefine::isMemberFunction() const
{
    return !className.isEmpty();
}

QDataStream &operator<<(QDataStream &s, const FunctionDefine &param)
{    
    s << param.name;
    s << param.className;
    s << param.isCFunction;
    s << param.isFlowFunction;
    s << param.paramIn;
    s << param.paramOut;         
    return s;
}

QDataStream &operator>>(QDataStream &s, FunctionDefine &param)
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
    cfunc = nullptr;
}

JZFunction::~JZFunction()
{

}

bool JZFunction::isCFunction() const
{
    return cfunc;
}

FunctionDefine JZFunction::define() const
{
    FunctionDefine define;
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
    return !className.isEmpty();
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
    s >> param.file;
    s >> param.localVariables;
    return s;
}