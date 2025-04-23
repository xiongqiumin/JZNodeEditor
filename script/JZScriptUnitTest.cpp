#include "JZNodeCompiler.h"
#include "JZScriptUnitTest.h"
#include "JZNodeFactory.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"

//JZNodeUnitTest
JZNodeUnitTest::JZNodeUnitTest()
{
    m_type = Node_unitTest;
    m_name = "unitTest";
}

JZNodeUnitTest::~JZNodeUnitTest()
{
}

void JZNodeUnitTest::fromNode(JZNode* node)
{
    m_pinList.clear();

    int n = node->pinCount();
    for (int i = 0; i < n; i++)
    {
        auto pin = node->pinByIndex(i);
        m_pinList.push_back(*pin);
    }
}

bool JZNodeUnitTest::compiler(JZNodeCompiler* c, QString& error)
{   
    c->addNodeEnter(m_id);

    QList<int> param_out_list = paramOutList();
    QList<JZNodeIRParam> in, out;
    in << irLiteral(m_id);
    for (int i = 0; i < param_out_list.size(); i++)
        out << irId(c->paramId(m_id, param_out_list[i]));
    
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

void JZScriptUnitTestVistor::visitorSelf(JZNode *node)
{
    if(node->type() == Node_param)
    {
        JZScriptItemDepend::ParamDepend d;
        d.id = node->id();
        depend->paramList.push_back(d);
    }
    else if(node->type() == Node_function)
    {
        JZScriptItemDepend::FunctionDepend d;
        d.id = node->id();
        depend->functionList.push_back(d);
    }
}

//JZScriptUnitTest
JZScriptUnitTest *instance()
{
    JZScriptUnitTest inst;
    return &inst;
}

JZScriptUnitTest::JZScriptUnitTest()
{
    m_script = nullptr;
}

JZScriptUnitTest::~JZScriptUnitTest()
{
}


void JZScriptUnitTest::setProject(JZProject* project)
{
    auto func_inst = project->environment()->functionManager();

    JZFunctionDefine hook;
    hook.name = "JZUnitTestHook";
    hook.isCFunction = true;
    hook.isFlowFunction = false;
    hook.paramIn.push_back(JZParamDefine("nodeId", "int"));
    hook.paramIn.push_back(JZParamDefine("outParamId", "args"));

    auto hook_func = BuiltInFunctionPtr(new JZUnitTestHook());
    func_inst->registBuiltInFunction(hook, hook_func);

    JZFunctionDefine widget_hook;
    widget_hook.name = "JZUnitWidgetHook";
    widget_hook.isCFunction = true;
    widget_hook.isFlowFunction = false;
    widget_hook.paramIn.push_back(JZParamDefine("w", "args"));
    widget_hook.paramIn.push_back(JZParamDefine("w", "args"));

    auto widget_hook_func = BuiltInFunctionPtr(new JZUnitTestHook());
    func_inst->registBuiltInFunction(hook, hook_func);
}

bool JZScriptUnitTest::hasHook(int id)
{
    return m_hookValues.contains(id);
}

QVariant JZScriptUnitTest::hookValue(int id)
{
    return m_hookValues.value(id);
}

JZScriptItemDependPtr JZScriptUnitTest::init(JZScriptItem *script)
{
    m_script = script;

    JZScriptUnitTestVistor visitor;
    visitor.visitorScript(m_script); 
}

void JZScriptUnitTest::build()
{
    JZTempItemGuard guard(m_project, m_script, true);

    QString func_name = m_script->function().fullName();

    JZNodeBuilder builder;
    builder.setProject(m_project);

    JZNodeProgram program;
    if (!builder.build(&program))
        return;

    JZNodeEngine engine;
    engine.setProgram(&program);
    engine.init();
    
    QVariantList in, out;
    
    const JZFunction* jz_func = program.function(func_name);
    
    auto cls = m_script->getClassItem();
    QVariant obj_holder = engine.createVariable(cls->classType());
    QVariant obj_ptr = JZNodeType::convertToPointer(obj_holder);

    in << obj_ptr;
    for (int i = 0; i < jz_func->define.paramIn.size(); i++)
    {
        in << inputList[i];
    }
    engine.call(jz_func,in,out);
}

void JZScriptUnitTest::applyDepends(JZScriptItemDependPtr depend)
{
    JZScriptUnitTestVistor visitor;

    JZNodeUnitTest *test_node = new JZNodeUnitTest();
    
    m_script->insertNode();

    visitor.replace();
}

//JZUnitTestHook
class JZUnitTestHook: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        auto hook_inst = JZScriptUnitTest::instance();
        int node_id = engine->getReg(Reg_CallIn).toInt();
        Q_ASSERT(hook_inst->hasHook(node_id));

        int count = engine->regInCount();
        for (int i = 1; i < count; i++)
        {
            int out_id = engine->getReg(Reg_CallIn + i).toInt();
            engine->setParam(irId(out_id), hook_inst->hookValue(out_id));
        }
    }
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

//JZScriptUnitTestInit
void JZScriptUnitTestInit()
{
    JZNodeFactory::instance()->registNode(Node_unitTest,createJZNode<JZNodeUnitTest>);
}