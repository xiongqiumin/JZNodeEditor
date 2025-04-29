#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimerEvent>
#include "JZNodeCompiler.h"
#include "JZScriptUnitTest.h"
#include "JZNodeFactory.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"
#include "JZNodeFunctionManager.h"

static JZScriptUnitTest* g_hook = nullptr;

//JZUnitTestHook
class JZUnitTestHook : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {        
        int hook_id = engine->getReg(Reg_CallIn).toInt();
        Q_ASSERT(g_hook->hasHook(hook_id));

        int count = engine->regInCount();
        for (int i = 0; i < count; i++)
        {
            int out_id = engine->getReg(Reg_CallIn + i).toInt();
            engine->setReg(Reg_CallOut, g_hook->hookValue(out_id));
        }
    }
};

//JZScriptItemDepend    
JZScriptItemDepend::JZScriptItemDepend()
{
    script = nullptr;
}

void JZScriptItemDepend::clear()
{
    paramList.clear();
    functionList.clear();
    script = nullptr;
}

void JZScriptItemDepend::setParam(int id, const QVariant &value)
{
    for (int i = 0; i < paramList.size(); i++)
    {
        if (paramList[i].node_id == id)
            paramList[i].value = value;
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
    for (int i = 0; i < param_out_list.size(); i++)
    {
        int out_id = c->paramId(m_id, param_out_list[i]);
        out << irId(out_id);

        in << irLiteral(out_id);

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
    m_depend = nullptr;
}

void JZScriptUnitTestVistor::updateDepend(JZScriptItemDepend *depend)
{
    m_depend = depend;
    visitorScript(m_depend->script);
}

//JZScriptNomarlVistor
JZScriptNomarlVistor::JZScriptNomarlVistor()
{
}

void JZScriptNomarlVistor::visitorSelf(const JZNode *node)
{
    if(node->type() == Node_param)
    {
        JZScriptItemDepend::ParamDepend d;
        d.node_id = node->id();
        m_depend->paramList.push_back(d);
    }
    else if(node->type() == Node_function)
    {
        JZScriptItemDepend::FunctionDepend d;
        d.node_id = node->id();
        m_depend->functionList.push_back(d);
    }
}

//JZScriptUnitTest
JZScriptUnitTest::JZScriptUnitTest()
{
    m_script = new JZScriptItem(JZScriptItem::Function);
    m_project = nullptr;
    m_timeId = -1;
}

JZScriptUnitTest::~JZScriptUnitTest()
{
    delete m_script;
}

void JZScriptUnitTest::setProject(JZProject* project)
{    
    m_project = project;
}

void JZScriptUnitTest::timerEvent(QTimerEvent* event)
{
    if (isFinish())
    {
        killTimer(event->timerId());
        m_engine.deinit();
        g_hook = nullptr;
    }
}

void JZScriptUnitTest::initEnv()
{
    JZScriptEnvironment* env = m_engine.environment();

    auto class_item = m_depend.script->getClassItem();
    if(class_item && env->isInherits(class_item->className(), "QWidget"))
    {
        JZNodeObjectDefine class_define = *env->meta(class_item->className());
        class_define.superName = "QObject";
        class_define.isUiWidget = false;
        env->objectManager()->replace(class_define);
    }
}

bool JZScriptUnitTest::hasHook(int id)
{
    return m_hookValues.contains(id);
}

QVariant JZScriptUnitTest::hookValue(int id)
{
    return m_hookValues.value(id);
}

JZNodeEngine* JZScriptUnitTest::engine()
{
    return &m_engine;
}

JZScriptItemDepend *JZScriptUnitTest::genDepend(JZScriptItem *script)
{    
    Q_ASSERT(m_project == script->project());

    m_depend.script = script;

    JZScriptNomarlVistor visitor;
    visitor.updateDepend(&m_depend);

    JZScriptUnitTestManager::instance()->updateDepend(&m_depend);

    return &m_depend;
}

bool JZScriptUnitTest::init()
{
    auto class_item = m_depend.script->getClassItem();
    JZProjectTempGuard guard(m_project, m_script, JZProjectTempGuard::TakeItem);
    if (class_item)
        guard.setClass(class_item->className());

    //replace
    QByteArray buffer = m_depend.script->toBuffer();
    m_script->fromBuffer(buffer);
    m_script->setName("UnitTest_" + m_depend.script->name());
    m_script->loadFinish();

    QString func_name = m_script->function().fullName();
    m_hookValues.clear();
    auto replace_node = [this](int id, const QVariant& value)
        {
            const JZNode* node = m_depend.script->getNode(id);

            int hook_id = JZNodeCompiler::paramId(id, node->paramOut(0));
            m_hookValues[hook_id] = value;

            JZNodeUnitTest* new_node = new JZNodeUnitTest();
            new_node->hook = this;
            new_node->copyFrom(node);
            m_script->removeNodeOnly(node->id());
            m_script->insertNode(new_node);
        };

    //set hook value
    for (int i = 0; i < m_depend.paramList.size(); i++)
    {
        auto& p = m_depend.paramList[i];
        if (p.value.isValid())
            replace_node(p.node_id, p.value);
    }
    for (int i = 0; i < m_depend.functionList.size(); i++)
    {
        auto& p = m_depend.functionList[i];
        if (p.value.isValid())
            replace_node(p.node_id, p.value);
    }

    //build
    JZNodeBuilder builder;
    builder.setProject(m_project);
    builder.setScriptExt({ m_script });

    if (!builder.build(&m_program))
    {
        m_error = "build failed";
        return false;
    }
    m_depend.function = m_script->function();

    m_engine.setProgram(&m_program);
    m_engine.init();
    g_hook = this;
    initEnv();

    QVariantList in, out;
    m_engine.call("__init__", in, out);

    if (class_item)
    {
        JZNodeObject* object = m_engine.environment()->objectManager()->create(class_item->classType());
        m_object = JZNodeObjectPointer(object, true);

        try
        {
            for (int i = 0; i < m_depend.initFuncList.size(); i++)
                m_depend.initFuncList[i](object);
        }
        catch (const std::exception& e)
        {
            m_error = e.what();
            return false;
        }
    }

    return true;
}

void JZScriptUnitTest::deinit()
{
    m_object = JZNodeObjectPointer();
}

void JZScriptUnitTest::start()
{
    QVariantList unit_in = m_depend.input;
    if (m_depend.function.isMemberFunction())
        unit_in.insert(0, QVariant::fromValue(m_object));

    m_engine.call(m_depend.function.fullName(), unit_in, m_depend.output);
    startTimer(50);
}

void JZScriptUnitTest::stop()
{
    killTimer(m_timeId);
    m_engine.deinit();
    g_hook = nullptr;
}

bool JZScriptUnitTest::isFinish()
{
    if (m_engine.status() == Status_running)
        return false;

    if (m_depend.isFinish)
        return m_depend.isFinish();

    return true;
}

bool JZScriptUnitTest::waitFinish(int timeout)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < timeout)
    {
        if (isFinish())
            return true;

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(20);
    }

    stop();
    return false;
}

bool JZScriptUnitTest::run(int timeout)
{
    if (!init())
        return false;

    start();
    bool ret = waitFinish(timeout);
    deinit();
    return ret;
}


//JZScriptUnitTestManager
JZScriptUnitTestManager *JZScriptUnitTestManager::instance()
{
    static JZScriptUnitTestManager inst;
    return &inst;
}

JZScriptUnitTestManager::JZScriptUnitTestManager()
{
}

JZScriptUnitTestManager::~JZScriptUnitTestManager()
{
    qDeleteAll(m_replaceList);
}

void JZScriptUnitTestManager::initEnv(JZScriptEnvironment* env)
{
    auto func_inst = env->functionManager();

    JZFunctionDefine hook;
    hook.name = "JZUnitTestHook";
    hook.isCFunction = true;
    hook.isFlowFunction = false;
    hook.paramIn.push_back(JZParamDefine("nodeId", "int"));
    hook.paramIn.push_back(JZParamDefine("outId", "args"));
    hook.paramOut.push_back(JZParamDefine("outValue", "arg"));

    auto test_hook = new JZUnitTestHook();
    auto hook_func = BuiltInFunctionPtr(test_hook);
    func_inst->registBuiltInFunction(hook, hook_func);
    env->nodeFactory()->registNode(Node_unitTest, createJZNode<JZNodeUnitTest>);
}

void JZScriptUnitTestManager::regist(JZScriptUnitTestVistor *replace)
{
    m_replaceList.push_back(replace);
}

void JZScriptUnitTestManager::updateDepend(JZScriptItemDepend *depend)
{
    for(int i = 0; i < m_replaceList.size(); i++)
        m_replaceList[i]->updateDepend(depend);
}