#include "JZNodeCompiler.h"
#include "JZScriptUnitTest.h"
#include "JZNodeFactory.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"
#include "JZNodeFunctionManager.h"

//JZUnitTestHook
class JZUnitTestHook : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {        
        int node_id = engine->getReg(Reg_CallIn).toInt();
        Q_ASSERT(hook->hasHook(node_id));

        int count = engine->regInCount();
        for (int i = 1; i < count; i++)
        {
            int out_id = engine->getReg(Reg_CallIn + i).toInt();
            engine->setReg(Reg_CallOut, hook->hookValue(out_id));
        }
    }

    JZScriptUnitTest *hook;
};

//JZUnitWidgetHook
class JZUnitWidgetHook : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine* engine) override
    {
        throw std::runtime_error("not support widget function now");
    }
};

//JZScriptItemDepend    
JZScriptItemDepend::JZScriptItemDepend()
{
}

void JZScriptItemDepend::setParam(QString name, const QVariant &value)
{
    for (int i = 0; i < paramList.size(); i++)
    {
        if (paramList[i].param->variable() == name)
            paramList[i].value = value;
    }
}

void JZScriptItemDepend::setFunction(QString func, const QVariant &value)
{
    for (int i = 0; i < functionList.size(); i++)
    {
        if (functionList[i].func->function() == func)
            functionList[i].value = value;
    }
}

//JZNodeUnitTest
JZNodeUnitTest::JZNodeUnitTest()
{
    m_type = Node_unitTest;
    m_name = "unitTest";
    hook = nullptr;
}

JZNodeUnitTest::~JZNodeUnitTest()
{
}

void JZNodeUnitTest::copyFrom(const JZNode* node)
{
    m_pinList.clear();
    
    m_id = node->id();
    int n = node->pinCount();
    for (int i = 0; i < n; i++)
    {
        auto pin = node->pinByIndex(i);
        m_pinList.push_back(*pin);
    }
}

bool JZNodeUnitTest::compiler(JZNodeCompiler* c, QString& error)
{   
    if (isFlowNode())
    {
        if (!c->addFlowInput(m_id, error))
            return false;
    }
    else
    {
        if (!c->addDataInput(m_id, error))
            return false;
    }

    QList<int> param_out_list = paramOutList();
    QList<JZNodeIRParam> in, out;
    in << irLiteral(m_id);
    for (int i = 0; i < param_out_list.size(); i++)
    {
        int out_id = c->paramId(m_id, param_out_list[i]);
        out << irId(out_id);

        int out_type = JZNodeType::variantType(hook->hookValue(out_id));
        c->setPinType(m_id, param_out_list[i], out_type);
    }    
    c->addCall("JZUnitTestHook",in, out);
    if (isFlowNode())
        c->addFlowOutput(m_id);

    return true;
}

//JZScriptUnitTestVistor
JZScriptUnitTestVistor::JZScriptUnitTestVistor()
{
    depend = JZScriptItemDependPtr(new JZScriptItemDepend());
}

void JZScriptUnitTestVistor::visitorSelf(const JZNode *node)
{
    if(node->type() == Node_param)
    {
        JZScriptItemDepend::ParamDepend d;
        d.param = dynamic_cast<const JZNodeParam*>(node);
        depend->paramList.push_back(d);
    }
    else if(node->type() == Node_function)
    {
        JZScriptItemDepend::FunctionDepend d;
        d.func = dynamic_cast<const JZNodeFunction*>(node);
        depend->functionList.push_back(d);
    }
}

//JZScriptUnitTest
JZScriptUnitTest::JZScriptUnitTest()
{
    m_script = new JZScriptItem(ProjectItem_scriptFunction);
    m_project = nullptr;
}

JZScriptUnitTest::~JZScriptUnitTest()
{
    delete m_script;
}

void JZScriptUnitTest::setProject(JZProject* project)
{
    auto func_inst = project->environment()->functionManager();
    m_project = project;    
}

bool JZScriptUnitTest::hasHook(int id)
{
    return m_hookValues.contains(id);
}

QVariant JZScriptUnitTest::hookValue(int id)
{
    return m_hookValues.value(id);
}

JZScriptItem *JZScriptUnitTest::script()
{
    return m_script;
}

JZScriptItemDependPtr JZScriptUnitTest::genDepend(const JZScriptItem *script)
{    
    Q_ASSERT(m_project == script->project());

    JZScriptUnitTestVistor visitor;
    visitor.depend->script = script;
    visitor.visitorScript(script);

    return visitor.depend;
}

JZScriptItem *JZScriptUnitTest::createUnitScript(JZScriptItemDependPtr depend)
{
    Q_ASSERT(m_project == depend->script->project());

    QByteArray buffer = depend->script->toBuffer();
    m_script->fromBuffer(buffer);    
    m_script->setName("UnitTest_" + depend->script->name());

    QString func_name = m_script->function().fullName();    
    
    m_hookValues.clear();
    auto replace_node = [this](const JZNode *node,const QVariant &value)
    {        
        int hook_id = JZNodeCompiler::paramId(node->id(), node->paramOut(0));
        m_hookValues[hook_id] = value;

        JZNodeUnitTest *new_node = new JZNodeUnitTest();
        new_node->hook = this;
        new_node->copyFrom(node);
        m_script->removeNodeOnly(node->id());
        m_script->insertNode(new_node);               
    };

    //set hook value
    for (int i = 0; i < depend->paramList.size(); i++)
    {
        auto &p = depend->paramList[i];        
        if (p.value.isValid())
            replace_node(p.param, p.value);
    }    
    for (int i = 0; i < depend->functionList.size(); i++)
    {
        auto &p = depend->functionList[i];
        if (p.value.isValid())
            replace_node(p.func, p.value);
    }

    return m_script;
}

//JZScriptUnitTestBuildinInit
void JZScriptUnitTestBuildinInit(JZScriptEnvironment *env)
{
    JZFunctionDefine hook;
    hook.name = "JZUnitTestHook";
    hook.isCFunction = true;
    hook.isFlowFunction = false;
    hook.paramIn.push_back(JZParamDefine("nodeId", "int"));
    hook.paramIn.push_back(JZParamDefine("outId", "args"));
    hook.paramOut.push_back(JZParamDefine("outValue", "arg"));

    auto test_hook = new JZUnitTestHook();
    test_hook->hook = this;
    auto hook_func = BuiltInFunctionPtr(test_hook);
    func_inst->registBuiltInFunction(hook, hook_func);

    JZFunctionDefine widget_hook;
    widget_hook.name = "JZUnitWidgetHook";
    widget_hook.isCFunction = true;
    widget_hook.isFlowFunction = false;
    widget_hook.paramIn.push_back(JZParamDefine("input", "args"));
    widget_hook.paramIn.push_back(JZParamDefine("output", "args"));

    auto widget_hook_func = BuiltInFunctionPtr(new JZUnitTestHook());
    func_inst->registBuiltInFunction(widget_hook, hook_func);
}

//JZScriptUnitTestInit
void JZScriptUnitTestNodeInit()
{
    JZNodeFactory::instance()->registNode(Node_unitTest,createJZNode<JZNodeUnitTest>);
}