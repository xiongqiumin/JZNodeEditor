#include "JZScriptBuildInFunction.h"
#include "JZNodeBind.h"
#include "JZNodeEngine.h"
#include "3rd/JZCommon/jzCommon/JZLogManager.h"
#include "JZNodeRuntime.h"

bool JZForCheck(int first, int last, int step, int op, QString &error)
{
    if (op == OP_eq)
    {
        if (first == last)
            return true;
        if (first < last && step > 0 && (last - first) % step == 0)
            return true;
        if (first > last && step < 0 && (first - last) % (-step) == 0)
            return true;
    }
    else if (op == OP_ne)
    {
        if ((first != last) || (step != 0))
            return true;
    }
    else if (op == OP_lt)
    {
        if ((first < last && step > 0) || (first >= last))
            return true;
    }
    else if (op == OP_le)
    {
        if ((first <= last && step > 0) || (first > last))
            return true;
    }
    else if (op == OP_gt)
    {
        if ((first > last && step < 0) || (first <= last))
            return true;
    }
    else if (op == OP_ge)
    {
        if ((first >= last && step < 0) || (first < last))
            return true;
    }

    error = QString::asprintf("Dead loop, please check, first=%d, last=%d, step=%d, op='%s'",
        first, last, step, qUtf8Printable(JZNodeType::opName(op)));
    return false;
}

void JZForRuntimeCheck(int first, int last, int step, int op)
{
    QString error;
    if (JZForCheck(first, last, step, op, error))
        return;

    throw std::runtime_error(qUtf8Printable(error));
}

void QObjectConnect(QObject *sender, JZFunctionPointer signal, QObject *recv, JZFunctionPointer slot)
{
    auto jz_sender = qobjectToJZObject(sender);
    auto jz_recv = qobjectToJZObject(recv);
    Q_ASSERT(jz_sender && jz_recv);
    JZObjectConnect(jz_sender, signal, jz_recv, slot);
}

void QObjectDisconnect(QObject *sender, JZFunctionPointer signal, QObject *recv, JZFunctionPointer slot)
{
    auto jz_sender = qobjectToJZObject(sender);
    auto jz_recv = qobjectToJZObject(recv);
    Q_ASSERT(jz_sender && jz_recv);
    JZObjectDisconnect(jz_sender, signal, jz_recv, slot);
}

class JZCreate: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QString type = engine->getReg(Reg_CallIn).toString();
        JZNodeObject *obj = engine->environment()->objectManager()->create(type);
        engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPointer(obj,true)));
    }
};

class JZClone: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {        
        engine->setReg(Reg_CallOut,engine->environment()->clone(engine->getReg(Reg_CallIn)));
    }
};

class JZFormatFunc: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QString text = format(engine, 0);
        engine->setReg(Reg_CallOut, text);
    }

    QString format(JZNodeEngine* engine, int start)
    {
        int count = engine->regInCount();

        QString format_str = engine->getReg(Reg_CallIn + start).toString();
        QVariantList var_list;
        for (int i = start + 1; i < count; i++)
            var_list << engine->getReg(Reg_CallIn + i);

        QString error;
        JZFormat format;
        format.init(format_str, error);

        QString reuslt = format.formatString(var_list);
        return reuslt;
    }
};

class JZFormatBinFunc: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QStringList list;
        int count = engine->regInCount();

        QString format_str = engine->getReg(Reg_CallIn).toString(); 
        QVariantList var_list;
        for (int i = 1; i < count; i++)
            var_list << engine->getReg(Reg_CallIn + i);
        
        QString error;
        JZFormatBinary format;
        format.init(format_str, error);

        QByteArray text = format.formatBinary(var_list);
        engine->setReg(Reg_CallOut, text);
    }
};


class JZPrint: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QString text = func->format(engine,0);
        JZScriptLog(text);
    }

    JZFormatFunc* func;
};

class JZLog: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        int module = engine->getReg(Reg_CallIn).toInt();
        int level = engine->getReg(Reg_CallIn + 1).toInt();

        QString text = func->format(engine, 2);
        JZLogManager::instance()->log(module, level, text);
    }

    JZFormatFunc* func;
};

void InitBuildInFunction()
{
    
    auto env = jzbind::bindEnvironment();
    auto func_inst = env->functionManager();

    //format
    JZFunctionDefine format;
    format.name = "format";
    format.isCFunction = true;
    format.paramIn.push_back(env->paramDefine("format", Type_string));
    format.paramIn.push_back(env->paramDefine("args", Type_args));
    auto format_func = BuiltInFunctionPtr(new JZFormatFunc());
    func_inst->registBuiltInFunction(format, format_func);

    //format
    JZFunctionDefine format_bin;
    format_bin.name = "formatBinary";
    format_bin.isCFunction = true;
    format_bin.paramIn.push_back(env->paramDefine("format", Type_string));
    format_bin.paramIn.push_back(env->paramDefine("args", Type_args));
    auto format_bin_func = BuiltInFunctionPtr(new JZFormatBinFunc());
    func_inst->registBuiltInFunction(format_bin, format_bin_func);

    //print
    JZFunctionDefine print;
    print.name = "print";
    print.isCFunction = true;
    print.isFlowFunction = true;
    format_bin.paramIn.push_back(env->paramDefine("format", Type_string));
    print.paramIn.push_back(env->paramDefine("args", Type_args));
    auto print_func = BuiltInFunctionPtr(new JZPrint());
    func_inst->registBuiltInFunction(print, print_func);

    //print
    JZFunctionDefine log;
    log.name = "log";
    log.isCFunction = true;
    log.isFlowFunction = true;
    log.paramIn.push_back(env->paramDefine("module", Type_int));
    log.paramIn.push_back(env->paramDefine("level",  Type_int));
    log.paramIn.push_back(env->paramDefine("format", Type_string));    
    log.paramIn.push_back(env->paramDefine("args", Type_args));
    auto log_func = BuiltInFunctionPtr(new JZLog());
    func_inst->registBuiltInFunction(log, log_func);

    //createObject
    JZFunctionDefine create;
    create.name = "createObject";
    create.isCFunction = true;
    create.isFlowFunction = false;    
    create.paramIn.push_back(env->paramDefine("type", Type_string));
    create.paramOut.push_back(env->paramDefine("arg", Type_arg));
    auto create_func = BuiltInFunctionPtr(new JZCreate());
    func_inst->registBuiltInFunction(create, create_func);
 
    //clone
    JZFunctionDefine clone;
    clone.name = "clone";
    clone.isCFunction = true;
    clone.isFlowFunction = false;    
    clone.paramIn.push_back(env->paramDefine("in", Type_arg));
    clone.paramOut.push_back(env->paramDefine("out", Type_arg));
    auto clone_func = BuiltInFunctionPtr(new JZClone());
    func_inst->registBuiltInFunction(clone, clone_func);

    func_inst->registCFunction("connect", true, jzbind::createFuncion(QObjectConnect));
    func_inst->registCFunction("disconnect", true, jzbind::createFuncion(QObjectDisconnect));
    func_inst->registCFunction("forRuntimeCheck", true, jzbind::createFuncion(JZForRuntimeCheck));
    func_inst->registCFunction("sleep", true, jzbind::createFuncion(JZSleep));  
}