#include <QDebug>
#include "JZNodeFunctionDefine.h"
#include "JZEvent.h"
#include "JZNodeObject.h"

//JZParamDefine
JZParamDefine::JZParamDefine()
{    
}

JZParamDefine::JZParamDefine(QString param_name, int dataType, const QString &v)
{
    this->name = param_name;
    this->type = JZNodeType::typeToName(dataType);
    this->value = v;
}

JZParamDefine::JZParamDefine(QString param_name, QString dataType, const QString &v)
{
    this->name = param_name;
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

//JZNodeParamBind
JZNodeParamBind::JZNodeParamBind()
{
    dir = UiToData;
}

QDataStream &operator<<(QDataStream &s, const JZNodeParamBind &param)
{
    s << param.variable << param.widget << param.dir;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeParamBind &param)
{
    s >> param.variable >> param.widget >> param.dir;
    return s;
}

//CFunction
CFunction::CFunction()
{

}

CFunction::~CFunction()
{

}

//BuiltInFunction
BuiltInFunction::BuiltInFunction()
{    
}

BuiltInFunction::~BuiltInFunction()
{
}

//JZFunctionDefine
JZFunctionDefine::JZFunctionDefine()
{
    isCFunction = false;
    isFlowFunction = false;    
    isVirtualFunction = false;
    isProtected = false;  
    isSlot = false;
    isSingle = false;
}

bool JZFunctionDefine::isNull() const
{
    return name.isEmpty();
}

void JZFunctionDefine::setFullName(const QString &full_name)
{
    int idx = full_name.lastIndexOf(".");
    if(idx >= 0)
    {
        className = full_name.left(idx);
        name = full_name.mid(idx + 1);
    }
    else
    {
        className.clear();
        name = full_name;
    }
}

QString JZFunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "." + result;
    return result;
}

QString JZFunctionDefine::delcare() const
{
    QString text;
    QStringList in_list,out_list;
    for(int i = 0; i < paramIn.size(); i++)
        in_list << paramIn[i].type;
    for(int i = 0; i < paramOut.size(); i++)
        out_list << paramOut[i].type;
    if(out_list.size() == 0)
        text = "void";
    else
        text = out_list.join(",");

    text += " " + fullName() + "(" + in_list.join(",") + ")";
    return text;
}

bool JZFunctionDefine::isMemberFunction() const
{
    return (paramIn.size() > 0 && paramIn[0].name == "this");
}

bool JZFunctionDefine::isVariadicFunction() const
{
    return (paramIn.size() > 0 && paramIn.back().dataType() == Type_args);
}

void JZFunctionDefine::updateParam(CFunction *func)
{
    paramIn.clear();
    paramOut.clear();
    for (int i = 0; i < func->args.size(); i++)
    {
        QString param_name = "input" + QString::number(i);
        int dataType = JZNodeType::typeidToType(func->args[i]);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->args[i]));

        paramIn.push_back(JZParamDefine(param_name, dataType));
    }
    if (func->result != typeid(void).name())
    {
        QString param_name = "output";
        int dataType = JZNodeType::typeidToType(func->result);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->result));

        paramOut.push_back(JZParamDefine(param_name, dataType));
    }
}

void JZFunctionDefine::setDefaultValue(int index, QString text)
{
    paramIn[index].value = text;
}

void JZFunctionDefine::setDefaultValue(int index, QStringList values)
{
    for (int i = 0; i < values.size(); i++)
    {
        paramIn[i + index].value = values[i];
    }
}

QDataStream &operator<<(QDataStream &s, const JZFunctionDefine &param)
{    
    s << param.name;
    s << param.className;
    s << param.isCFunction;
    s << param.isFlowFunction;
    s << param.isVirtualFunction;
    s << param.isProtected;
    s << param.isSingle;
    s << param.isSlot;
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
    s >> param.isVirtualFunction;
    s >> param.isProtected;
    s >> param.isSingle;
    s >> param.isSlot;
    s >> param.paramIn;
    s >> param.paramOut;          
    return s;
}

//CSingle
CSingle::CSingle()
{    
}

CSingle::~CSingle()
{

}

//JZSingleDefine
JZSingleDefine::JZSingleDefine()
{
    csingle = nullptr;
}

QString JZSingleDefine::fullName() const
{
    return className + "." + name;
}

QString JZSingleDefine::delcare() const
{
    QString text;
    QStringList out_list;
    for(int i = 0; i < paramOut.size(); i++)
        out_list << paramOut[i].type;
    text += "void " + fullName() + "(" + out_list.join(",") + ")";
    return text;
}

bool JZSingleDefine::isCSingle() const
{
    return (csingle != nullptr);
}

QDataStream &operator<<(QDataStream &s, const JZSingleDefine &param)
{
    Q_ASSERT(!param.isCSingle());
    s << param.name;
    s << param.className;
    s << param.paramOut;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZSingleDefine &param)
{
    s >> param.name;
    s >> param.className;
    s >> param.paramOut;
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

JZParam::JZParam(const QString &param_name, int type)
{
    this->name = param_name;
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

QString JZFunction::name() const
{
    return define.name;
}

QString JZFunction::className() const
{
    return define.className;
}

QString JZFunction::fullName() const
{
    return define.fullName();
}

bool JZFunction::isCFunction() const
{
    return define.isCFunction;
}

bool JZFunction::isMemberFunction() const
{
    return define.isMemberFunction();
}

bool JZFunction::isFlowFunction() const
{
    return define.isFlowFunction;
}

bool JZFunction::isVirtualFunction() const
{
    return define.isVirtualFunction;
}

QDataStream &operator<<(QDataStream &s, const JZFunction &param)
{
    s << param.define;
    s << param.path;
    s << param.addr;
    s << param.addrEnd;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZFunction &param)
{
    s >> param.define;
    s >> param.path;
    s >> param.addr;
    s >> param.addrEnd;
    return s;
}
