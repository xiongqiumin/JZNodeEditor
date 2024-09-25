#include "JZScriptBuildInFunction.h"
#include "JZNodeBind.h"
#include "JZNodeEngine.h"


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
        engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPtr(obj,true)));
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

class JZPrint: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QStringList list;
        int count = engine->regInCount();
        for (int i = 0; i < count; i++)
        {
            list << JZNodeType::debugString(engine->getReg(Reg_CallIn + i));
        }
        engine->print(list.join(" "));
    }
};

void InitBuildInFunction()
{
    auto env = jzbind::bindEnvironment();
    auto func_inst = env->functionManager();
    JZFunctionDefine print;
    print.name = "print";
    print.isCFunction = true;
    print.isFlowFunction = true;    
    print.paramIn.push_back(env->paramDefine("args", Type_args));
    auto print_func = BuiltInFunctionPtr(new JZPrint());
    func_inst->registBuiltInFunction(print, print_func);

    JZFunctionDefine create;
    create.name = "createObject";
    create.isCFunction = true;
    create.isFlowFunction = false;    
    create.paramIn.push_back(env->paramDefine("type", Type_string));
    create.paramOut.push_back(env->paramDefine("arg", Type_arg));
    auto create_func = BuiltInFunctionPtr(new JZCreate());
    func_inst->registBuiltInFunction(create, create_func);
 
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
}